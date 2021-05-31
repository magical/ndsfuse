/*nds parsing stuff*/
#ifndef _NDSROM_H_
#include <stdint.h>
#include "tree.h"

typedef struct
{
	char gametitle[12];
	char gamecode[4];
	char makercode[2];
	uint8_t unitcode;
	uint8_t encryptionkeyselect;
	uint8_t devicecapacity;
	uint8_t reserved[9];
	uint8_t romversion;
	uint8_t autostartflag;
	uint32_t arm9rom;
	uint32_t arm9entry;
	uint32_t arm9ram;
	uint32_t arm9size;
	uint32_t arm7rom;
	uint32_t arm7entry;
	uint32_t arm7ram;
	uint32_t arm7size;
	uint32_t fntoffset;
	uint32_t fntsize;
	uint32_t fatoffset;
	uint32_t fatsize;
	uint32_t arm9overlayoffset;
	uint32_t arm9overlaysize;
	uint32_t arm7overlayoffset;
	uint32_t arm7overlaysize;
	uint32_t romctrlnormal;
	uint32_t romctrlkey1;
	uint32_t iconttitleoffset;
	uint16_t secureareachecksum;
	uint16_t securearealoadingtimout;
	uint32_t arm9unk;
	uint32_t arm7unk;
	uint8_t secureareadisable[8];
	uint32_t romsize;
	uint32_t headersize;
	uint8_t reserved2[0x38];
	uint8_t nintendologo[0x9C];
	uint16_t logochecksum;
	uint16_t headerchecksum;
	uint32_t debugrom;
	uint32_t debugsize;
	uint32_t debugram;
	uint32_t reserved3;
	uint8_t reserved4[0x90];
} nds_header;

typedef struct
{
	uint32_t subtableoff;
	uint16_t firstfileid;
	union
	{
		uint16_t totaldirs;
		uint16_t parentid;
	};
} nds_fnt_main_table_entry;

typedef struct
{
	uint32_t overlayid;
	uint32_t ramaddr;
	uint32_t ramsize;
	uint32_t bsssize;
	uint32_t staticinitstart;
	uint32_t staticinitend;
	uint32_t fileid;
	uint32_t reserved;
} nds_overlay_entry;

typedef struct
{
	tree_node *filetree;
	uint32_t *fat;
	uint32_t fatsize;
	int fd;

	time_t mtime;
} nds_file;

extern nds_file *nds_do_magic(const char *file);	//can leave unfreed objects on error
extern void free_nds_file(nds_file *file);

#endif
