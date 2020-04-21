#include "pci.h"
#include "ide.h"

#include <drivers/ioports.h>

#define PCI_PORT_CONF_ADDR 0xCF8


static inline void _io_wait(void)
{
    asm volatile ("jmp 1f\n\t"
        "1:jmp 2f\n\t"
        "2:");
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t addr;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t t = 0;

    addr = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
        (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(PCI_PORT_CONF_ADDR, addr);
    _io_wait();
    uint32_t in = inl(0xCFC);
    t = (uint16_t)((in >> ((offset & 2) * 8)) & 0xffff);
    return t;
}

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t addr;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t t = 0;

    addr = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
        (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(PCI_PORT_CONF_ADDR, addr);
    _io_wait();
    uint32_t in = inl(0xCFC);
    t = ((in >> ((offset & 2) * 8)));
    return t;
}

void pci_init()
{
    for (int bus = 0; bus < 256; bus++)
    {
        for (int slot = 0; slot < 32; slot++)
        {
            uint16_t vendor = pci_config_read_word(bus, slot, 0, 0);
            if (vendor == 0xffff)
            {
                continue;
            }

            //detect ide device
            if ((pci_config_read_word(bus, slot, 0, 0xA) & 0xFF) == 0x01 &&
                (pci_config_read_word(bus, slot, 0, 0xB) & 0xFF) == 0x01)
            {
                ide_init(bus, slot);
                continue;
            }
        }
    }
}