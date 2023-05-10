#include "fat.h"
#include "ide.h"
#include "rprintf.h"
#include <stdint.h>
#include <string.h>

struct boot_sector *bs;
char bootSect[512];
char fat_table[512*8];
char rootSect[512];
char readBuf[512];
uint32_t root_sector;
uint32_t BOOT_SECT_START;

int fatInit() {
	BOOT_SECT_START = 0x800;
	ata_lba_read(BOOT_SECT_START, bootSect, 1);
	bs = bootSect;
	uint32_t fat_sector = BOOT_SECT_START + bs->num_reserved_sectors;
	esp_printf(putc, "fat table start: '%d'\n", fat_sector);
	esp_printf(putc,"boot signature: '%x'\n", bs->boot_signature);
	esp_printf(putc, "filesystem type: '%s'\n", strtok(bs->fs_type, " "));
	ata_lba_read(fat_sector, fat_table, 1);
	root_sector = BOOT_SECT_START + (bs->num_fat_tables * bs->num_sectors_per_fat) + bs->num_hidden_sectors + bs->num_reserved_sectors;
}

void fatOpen(char *path, struct file *open) {
	
	ata_lba_read(root_sector, rootSect, bs->num_sectors_per_cluster);
	struct root_directory_entry *prde = rootSect;
	char *filename = strtok(path, "/");
	filename = strtok(filename, ".");
	esp_printf(putc, "\n\n\n");
	esp_printf(putc, "filename: '%s'\n", filename);
	for (int i = 0; i < sizeof(prde); i++) {
		char *current = strtok(prde[i].file_name, " ");
		esp_printf(putc, "current: '%s'\n", current);
		if (appstrcmp(filename, current) == 0) {
			open->rde = prde[i];
			open->start_cluster = (uint32_t) prde[i].cluster;
			break;
		}	
	}
}

void fatRead(struct file open) {
	// uses file pointer as input
	uint32_t current_cluster = open.start_cluster;
	uint32_t data_cluster = 0x800 + (bs->num_fat_tables * bs->num_sectors_per_fat) + bs->num_hidden_sectors + bs->num_reserved_sectors + (bs->num_root_dir_entries * sizeof(struct root_directory_entry) / bs->bytes_per_sector) + ((current_cluster - 2) * bs->num_sectors_per_cluster);
	uint32_t current = open.start_cluster;
	while (fat_table[current] != 0x0) {
		ata_lba_read(data_cluster, readBuf, 1);
		for (int i = 0; i < 512; i++) {
			esp_printf(putc, "%c", readBuf[i]);
		}
		current_cluster = fat_table[current];
	}
	ata_lba_read(data_cluster, readBuf, 1);
	for (int i = 0; i < 512; i++) {
		esp_printf(putc, "%c", readBuf[i]);
	}
}

int appstrcmp(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] == str2[i]) {
        if (str1[i] == '\0')
            return 0;
        i++;
    }
    return str1[i] - str2[i];
}



char *strtok(char *str, const char *delim) {
    static char *lastToken = NULL;  //store state
    char *token;
    int i, j, match;

    if (str == NULL && lastToken == NULL) {
        // returjn null
        return NULL;
    } else if (str == NULL && lastToken != NULL) {
    
        str = lastToken;
    }

    
    i = 0;
    while (str[i] != '\0') {
        match = 0;
        for (j = 0; delim[j] != '\0'; j++) {
            if (str[i] == delim[j]) {
                match = 1;
                break;
            }
        }
        if (!match) {
            break;
        }
        i++;
    }

    
    if (str[i] == '\0') {
        lastToken = NULL;
        return NULL;
    }

    token = str + i;
    while (*token != '\0') {
        match = 0;
        for (j = 0; delim[j] != '\0'; j++) {
            if (*token == delim[j]) {
                match = 1;
                break;
            }
        }
        if (match) {
            *token = '\0';
            lastToken = token + 1;
            return str + i;
        }
        token++;
    }

    lastToken = NULL;
    return str + i;
}

