#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <unistd.h>
#include "fio.h"
#include "filesystem.h"
#include "romfs.h"
#include "osdebug.h"
#include "hash-djb2.h"
#include "clib.h"

extern const unsigned char _sromind;


static uint8_t get_byte(const uint8_t * d) {
    return ((uint8_t) d[0]) ;
}

int pwd (uint32_t hash, char *buf) {
    uint8_t name_len=0;
    const uint8_t * file;
    file =romfs_get_file_by_hash(&_sromind, hash, NULL);
    name_len = get_byte(file) & 0x7f;
    if (name_len) {
        strncpy(buf,(const char *)file+1,name_len-1);
        buf[name_len]='\0';
        return 1;
    } else {
        buf[0]='\0';
        return 0;
    }
}


int ls (uint32_t hash) {
    uint32_t dir_len, offset=0;
    uint8_t  name_len=0;
    const uint8_t * file;
    file =romfs_get_file_by_hash(&_sromind, hash, &dir_len);

    name_len = get_byte(file) & 0x7f;
    offset = offset + name_len + 1;


    while ( dir_len>offset ) {
        name_len = get_byte(file+offset);
        if (name_len & 0x80) {
            name_len &= 0x7f ;
            fio_write(1, file+offset+1, name_len);
            fio_printf(1, "/");
        } else {
            fio_write(1, file+offset+1, name_len);
        }

        fio_printf(1, "\r\n");
        offset = offset + name_len + 1;

    }


    return 0;
}

