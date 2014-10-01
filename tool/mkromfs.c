#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>

#define hash_init 5381

uint32_t hash_djb2(const uint8_t * str, uint32_t hash) {
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}

void usage(const char * binname) {
    printf("Usage: %s [-d <dir>] [outfile]\n", binname);
    exit(-1);
}

void processdir(DIR * dirp, const char * curpath, FILE * outfile, const char * prefix) {

    char fullpath[1024];
    char buf[16 * 1024];
    struct dirent * ent;
    DIR * rec_dirp;
    uint32_t cur_hash = hash_djb2((const uint8_t *) curpath, hash_init);
    uint32_t size, w, hash;
    uint8_t b;
    FILE * infile;


    while ((ent = readdir(dirp))) {
        strcpy(fullpath, prefix);
        strcat(fullpath, "/");
        strcat(fullpath, curpath);
        strcat(fullpath, ent->d_name);


#ifdef _WIN32
        if (GetFileAttributes(fullpath) & FILE_ATTRIBUTE_DIRECTORY) {
#else
        if (ent->d_type == DT_DIR) {
#endif
            if (strcmp(ent->d_name, ".") == 0)
                continue;
            if (strcmp(ent->d_name, "..") == 0)
                continue;
            strcat(fullpath, "/");
            rec_dirp = opendir(fullpath);
            processdir(rec_dirp, fullpath + strlen(prefix) + 1, outfile, prefix);
            closedir(rec_dirp);

        } else {
            hash = hash_djb2((const uint8_t *) ent->d_name, cur_hash);
            infile = fopen(fullpath, "rb");
            if (!infile) {
                perror("opening input file");
                exit(-1);
            }
            b = (hash >>  0) & 0xff;
            fwrite(&b, 1, 1, outfile);
            b = (hash >>  8) & 0xff;
            fwrite(&b, 1, 1, outfile);
            b = (hash >> 16) & 0xff;
            fwrite(&b, 1, 1, outfile);
            b = (hash >> 24) & 0xff;
            fwrite(&b, 1, 1, outfile);
            fseek(infile, 0, SEEK_END);
            size = ftell(infile);
            fseek(infile, 0, SEEK_SET);
            b = (size >>  0) & 0xff;
            fwrite(&b, 1, 1, outfile);
            b = (size >>  8) & 0xff;
            fwrite(&b, 1, 1, outfile);
            b = (size >> 16) & 0xff;
            fwrite(&b, 1, 1, outfile);
            b = (size >> 24) & 0xff;
            fwrite(&b, 1, 1, outfile);
            while (size) {
                w = size > 16 * 1024 ? 16 * 1024 : size;
                fread(buf, 1, w, infile);
                fwrite(buf, 1, w, outfile);
                size -= w;
            }
            fclose(infile);
        }
    }
}

void createind (DIR * dirp, const char * curpath, const char * prefix,FILE * indfile) {
    char fullpath[1024];
    uint8_t b;

    struct dirent **namelist;
    int n;
    uint32_t hash,size,ind_size;
    struct dirent * ent;
    DIR * rec_dirp;
    fpos_t pos_size, pos_end;

    hash = hash_djb2((const uint8_t *) curpath, hash_init);
    //printf("mypath=%s\thash=%08X\n",curpath,hash);
    strcpy(fullpath, prefix);
    strcat(fullpath, "/");
    strcat(fullpath, curpath);
    n = scandir(fullpath, &namelist, NULL, alphasort);
    //printf("pwd=/romfs/%s\n",curpath);

    b = (hash >>  0) & 0xff;
    fwrite(&b, 1, 1, indfile);
    b = (hash >>  8) & 0xff;
    fwrite(&b, 1, 1, indfile);
    b = (hash >> 16) & 0xff;
    fwrite(&b, 1, 1, indfile);
    b = (hash >> 24) & 0xff;
    fwrite(&b, 1, 1, indfile);

    ind_size=0;
    fgetpos(indfile, &pos_size);
    fwrite(&ind_size, 1, 4, indfile);
    ind_size = ind_size + strlen(curpath) +1;
    b = ( strlen(curpath) & 0xff ) | 0x80;
    fwrite(&b, 1, 1, indfile);
    fwrite(curpath, 1, strlen(curpath), indfile);

    if (n < 0)
        perror("scan dir fail!\n");
    else {
        while (n--) {
            if (strcmp(namelist[n]->d_name, ".") == 0)
                continue;
            if (strcmp(namelist[n]->d_name, "..") == 0)
                continue;
            size = strlen(namelist[n]->d_name);
            if (namelist[n]->d_type == DT_DIR) {
                //printf("(%d)%s/\n", size , namelist[n]->d_name);
                b = ( size & 0xff ) | 0x80;
                fwrite(&b, 1, 1, indfile);
            } else {
                //printf("(%d)%s\n", size , namelist[n]->d_name);
                b = size & 0xff;
                fwrite(&b, 1, 1, indfile);
            }
            fwrite(namelist[n]->d_name, 1, size, indfile);
            ind_size = (ind_size + 1 + size);
        }
        fgetpos(indfile, &pos_end);
        fsetpos(indfile, &pos_size);
        fwrite(&ind_size, 1, 4, indfile);
        fsetpos(indfile, &pos_end);
        free(namelist);
    }


    while ((ent = readdir(dirp))) {
        strcpy(fullpath, prefix);
        strcat(fullpath, "/");
        strcat(fullpath, curpath);
        strcat(fullpath, ent->d_name);
#ifdef _WIN32
        if (GetFileAttributes(fullpath) & FILE_ATTRIBUTE_DIRECTORY) {
#else
        if (ent->d_type == DT_DIR) {
#endif
            if (strcmp(ent->d_name, ".") == 0)
                continue;
            if (strcmp(ent->d_name, "..") == 0)
                continue;
            strcat(fullpath, "/");
            rec_dirp = opendir(fullpath);
            createind(rec_dirp, fullpath + strlen(prefix) + 1, prefix,indfile);
            closedir(rec_dirp);
        } else {
        }
    }

}

int main(int argc, char ** argv) {
    char * binname = *argv++;
    char * o;
    char * outname = NULL;
    char * dirname = ".";
    uint64_t z = 0;
    FILE * outfile;
    FILE * indfile;
    DIR * dirp;

    while ((o = *argv++)) {
        if (*o == '-') {
            o++;
            switch (*o) {
            case 'd':
                dirname = *argv++;
                break;
            default:
                usage(binname);
                break;
            }
        } else {
            if (outname)
                usage(binname);
            outname = o;
        }
    }

    if (!outname)
        outfile = stdout;
    else
        outfile = fopen(outname, "wb");

    if (!outfile) {
        perror("opening output file");
        exit(-1);
    }

    dirp = opendir(dirname);
    if (!dirp) {
        perror("opening directory");
        exit(-1);
    }
    char indpath[strlen(outname)+5];
    strcpy(indpath, outname);
    strcat(indpath, ".idx");
    indfile=fopen(indpath, "wb");

    if (!indfile) {
        perror("create INDEX fail");
        exit(-1);
    }
    createind(dirp,"",dirname,indfile);
    fclose(indfile);
    closedir(dirp);

    dirp = opendir(dirname);
    processdir(dirp, "", outfile, dirname);
    fwrite(&z, 1, 8, outfile);
    if (outname)
        fclose(outfile);
    closedir(dirp);

    return 0;
}
