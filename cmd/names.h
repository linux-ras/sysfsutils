/* names.h */

#ifndef _NAMES_H_
#define _NAMES_H_

#define PCI_LOOKUP_VENDOR 1
#define PCI_LOOKUP_DEVICE 2
#define PCI_LOOKUP_CLASS 4
#define PCI_LOOKUP_SUBSYSTEM 8
#define PCI_LOOKUP_PROGIF 16
#define PCI_LOOKUP_NUMERIC 0x10000

#define PCI_VENDOR_ID	0x00
#define PCI_DEVICE_ID	0x02

struct pci_access {
        int numeric_ids;
        char *pci_id_file_name;
        char *nl_list;
        struct nl_entry **nl_hash;
};

#endif /* _NAMES_H_ */
