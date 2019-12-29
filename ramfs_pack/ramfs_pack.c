
#include <stdio.h>
#include <string.h>
#include <dirent.h> 
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct header
{
    char path[128];
    uint32_t start;
    uint32_t len;
}header_t;

static header_t _headers[512];
static uint32_t _header_count = 0;

static uint32_t _file_len(const char* path)
{
    struct stat st;
    if (stat(path, &st) != -1)
        return st.st_size;
    printf("size failed %s\n", strerror(errno));
    return 0;
}

bool dir_visitor(const char* dest_path, const char* src_path)
{
    printf("Visiting %s\n", src_path);
    
    DIR* dir = opendir(src_path);
    if (dir == NULL)  // opendir returns NULL if couldn't open directory 
    {
        printf("could not open directory %s\n", src_path);
        return false;
    }
    struct dirent* de;
    while ((de = readdir(dir)) != NULL)
    {
        char src_buff[512];
        char dest_buff[512];
        strcpy(dest_buff, dest_path);
        if (strlen(dest_buff) > 0)
            strcat(dest_buff, "/");
        strcat(dest_buff, de->d_name);

        strcpy(src_buff, src_path);
        strcat(src_buff, "/");
        strcat(src_buff, de->d_name);

        if (de->d_type & DT_REG)
        {
            if (strlen(dest_buff) > 127)
            {
                printf("path %s longer than 127 chars\n", dest_buff);
                return false;
            }
            if (_header_count == 512)
            {
                printf("max file count of 512 exceeded\n");
                return false;
            }
            strcpy(_headers[_header_count].path, dest_buff);
            _headers[_header_count].len = _file_len(src_buff);
            _headers[_header_count].start = 0;
            _header_count++;
        }
        else if (de->d_type & DT_DIR && de->d_name[0] != '.')
        {
            dir_visitor(dest_buff, src_buff);
        }
    }
    closedir(dir);
    return true;
}

int ramfs_dump(char* path)
{
    uint32_t len = _file_len(path);
    FILE* f = fopen(path, "r");
    if (!f)
    {
        printf("failed to open %s\n", path);
        return -1;
    }
    uint8_t* buff = (uint8_t*)malloc(len);
    fread(buff, len, 1, f);
    fclose(f);
    uint32_t count = *(uint32_t*)buff;
    header_t* hdr = (header_t*)(buff + sizeof(uint32_t));
    for (uint32_t i = 0; i < count; i++)
    {
        printf("file %s 0x%08x to 0x%08x\n", hdr[i].path, hdr[i].start, hdr[i].start + hdr[i].len);
    }
    free(buff);
    return 0;
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        return ramfs_dump(argv[1]);
    }
    if (argc < 2)
	{
		printf("provide root dir and output arguments - eg ramfs_pack ../data data.bin\n");
		return -1;
	}
    
    if(!dir_visitor("", argv[1]))
        return -1;

    FILE* outf = fopen(argv[2], "w");
    if (!outf)
    {
        printf("could not open %s\n", argv[2]);
        return -1;
    }
    printf("packing %d files\n", _header_count);
    fwrite(&_header_count, sizeof(uint32_t), 1, outf);
    uint32_t offset = sizeof(uint32_t) + _header_count * sizeof(header_t);
    for (uint32_t i = 0; i < _header_count; i++)
    {
        _headers[i].start = offset;
        offset += _headers[i].len;
        printf("Header %s 0x%08x 0x%08x\n", _headers[i].path, _headers[i].start, _headers[i].len);
    }
    fwrite(_headers, sizeof(header_t), _header_count, outf);
    for (uint32_t i = 0; i < _header_count; i++)
    {
        char src_path[512];
        strcpy(src_path, argv[1]);
        strcat(src_path, "/");
        strcat(src_path, _headers[i].path);
        FILE* inf = fopen(src_path, "rb");
        if (!inf)
        {
            printf("failed to open file %s\n", src_path);
            return -1;
        }
        uint8_t* buff = malloc(_headers[i].len);
        fread(buff, 1, _headers[i].len, inf);
        fwrite(buff, 1, _headers[i].len, outf);
        fclose(inf);
        free(buff);
    }

    printf("wrote 0x%08x bytes\n", (uint32_t)ftell(outf));
    fclose(outf);
    return 0;
}