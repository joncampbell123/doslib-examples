
#include <stdio.h>
#include <conio.h> /* this is where Open Watcom hides the outp() etc. functions */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <dos.h>

#include <hw/pci/pci.h>
#include <hw/cpu/cpu.h>

/* EXAMPLE: These are the PCI device and vendor IDs of the device you're interested in. */
/* For *MY* development purposes these are tied to the S3 PCI emulation in DOSBox-X, change
 * to match *YOUR* video card of interest. */
static uint16_t pci_want_vendor = 0x5333;       /* S3 */
static uint16_t pci_want_device = 0x8811;       /* whatever DOSBox is emulating */

/* EXAMPLE: Once you find the PCI device, store the bus and device number you found it at.
 *          We don't store the function number because it's highly unlikely your VGA card
 *          would have multiple PCI functions. */
static signed char pci_found_bus = -1;
static signed char pci_found_device = -1;

int main(int argc,char **argv) {
    /* what CPU am I running under? */
	cpu_probe();

    /* PCI was defined in 1992, but didn't appear until very late 486 era,
     * when Pentium was becoming the new standard. The 32-bit I/O used in
     * the PCI library requires a 386 or higher.
     *
     * If you remove this check, the PCI library itself will refuse to
     * probe if lower than 386. This just tells the user in the same case. */
	if (cpu_basic_level < CPU_386) {
		printf("PCI programming requires a 386 or higher\n");
		return 1;
	}

    /* probe for the PCI bus.
     * The device of interest is a PCI device. */
	if (pci_probe(-1) == PCI_CFG_NONE) {
		printf("PCI bus not found\n");
		return 1;
	}

    /* how many PCI busses are there on this system? */
    pci_probe_for_last_bus();

    /* enumerate the PCI device to find the card.
     * we lock onto the FIRST card if multiple cards exist.
     * feel free to add command line options that let the user
     * tell this program which card to use if you like. */
    {
        uint16_t vendor,device;
        uint8_t bus,dev;

        for (bus=0;bus <= pci_bios_last_bus;bus++) {
            for (dev=0;dev < 32;dev++) {
                /* unlike hw/pci, don't probe functions because we don't care about functions */
                /* for more information what this code is reading, see the PCI 2.x specification
                 * describing PCI configuration space. */

                /* vendor ID */
                vendor = pci_read_cfgw(bus,dev,/*func*/0,/*byte offset*/0x00);
                if (vendor == 0xFFFF) continue; /* if nothing there, move on */

                /* device ID */
                device = pci_read_cfgw(bus,dev,/*func*/0,/*byte offset*/0x02);
                if (device == 0xFFFF) continue; /* if nothing there, move on */

                if (vendor == pci_want_vendor && device == pci_want_device) {
                    /* found it! stop the scan! */
                    pci_found_bus = bus;
                    pci_found_device = dev;
                    break;
                }
            }
        }
    }

    if (pci_found_bus < 0 || pci_found_device < 0) {
        printf("Did not find device\n");
        return 1;
    }

    printf("PCI device of interest is on bus %u, device %u, function 0\n",pci_found_bus,pci_found_device);
	return 0;
}

