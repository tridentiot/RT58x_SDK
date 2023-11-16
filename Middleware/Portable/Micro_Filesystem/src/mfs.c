/**
 * Copyright (c) 2020 Rex Huang, All Rights Reserved.
 */
/** @file ping.c
 *
 * @author Rex Huang
 * @version 0.1
 * @date 2020/05/28
 * @license
 * @description
 */

//=============================================================================
//                Include
//=============================================================================
#include <stddef.h>

#include "util_log.h"
#include "util_printf.h"
#include "mfs.h"
#include "mfs_define.h"
#include "crc.h"

#include "sys_arch.h"

//=============================================================================
//                Private Definitions of const value
//=============================================================================
#define MFS_START_ADDRESS   0xF0000
#define MFS_STORAGE_SIZE    0x1000

#define RT58X_FLASH_BASE_ADDR   0x0

//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================

//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================
static int opened_files = 0;
static uint8_t Cache_buf[MFS_STORAGE_SIZE];
static DirectoryDescriptor root ;
static FileAllocationTable fat ;
static fs_head_t fs_head;
static uintptr_t startAddress = 0;
//=============================================================================
//                Public Global Variables
//=============================================================================

//=============================================================================
//                Private Definition of Compare/Operation/Inline function/
//=============================================================================
static int getFileID(const char *name, DirectoryDescriptor *rootDir);
static int getFreeNode(FileAllocationTable *fat);
static int getFreePage(FileDescriptor *file, FileAllocationTable *fat);
static int getNextPage(int currNode, FileAllocationTable *pfat);
static int getFileSize(FileDescriptor *file, FileAllocationTable *pfat);
static int readFile(FileDescriptor *pfile, FileAllocationTable *pfat, int length, uint8_t *pdbuf);
static void createFile(const char *name, FileDescriptor *file);
static void initialFileDesc(FileAllocationTable *pfat, DirectoryDescriptor *proot);

//=============================================================================
//                Functions
//=============================================================================
static void _memcpy_ram(uint8_t *dst, uint8_t *src, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
}

static void fs_flush(void)
{
    int i;
    while (flash_check_busy());
    flash_erase(FLASH_ERASE_SECTOR, MFS_START_ADDRESS);

    // page program
    for (i = 0; i < (0x1000 >> 8); i++)
    {
        while (flash_check_busy());
        //info("wr %x -> %x, %d\n", (uint32_t)&((uint8_t*)Cache_buf)[i * (0x1 << 8)], to_pos, valid_len);
        flash_write_page((uint32_t) & ((uint8_t *)Cache_buf)[i * (0x100)], MFS_START_ADDRESS + i * 0x100);
    }
}
static void fc_read(uintptr_t ram_addr, uintptr_t fc_addr, uint32_t size)
{
    _memcpy_ram((uint8_t *)ram_addr, &Cache_buf[fc_addr], size);
}

void filesystem_reset(void)
{
    while (flash_check_busy());
    //taskENTER_CRITICAL();
    flash_erase(FLASH_ERASE_SECTOR, RT58X_FLASH_BASE_ADDR + MFS_START_ADDRESS);
    //info("Flash control erase success\n");
    initialFileDesc(&fat, &root);

    _memcpy_ram(fs_head.magic, (uint8_t *)g_mfs_magic, 3);
    fs_head.files = 0;

    _memcpy_ram(&Cache_buf[0], (uint8_t *)&fs_head, sizeof(fs_head_t));
    _memcpy_ram(&Cache_buf[FILE_DESCRIB_OFFSET], (uint8_t *)&root, sizeof(DirectoryDescriptor));
    _memcpy_ram(&Cache_buf[DATA_INDEX_TABLE_OFFSET], (uint8_t *)&fat, sizeof(FileAllocationTable));

    fs_flush();
#if 0
    fc_write((uint32_t)&fs_head, MFS_START_ADDRESS, sizeof(fs_head_t));
    fc_write((uint32_t)&root, MFS_START_ADDRESS + FILE_DESCRIB_OFFSET, sizeof(DirectoryDescriptor));
    fc_write((uint32_t)&fat, MFS_START_ADDRESS + DATA_INDEX_TABLE_OFFSET, sizeof(FileAllocationTable));
#endif
}

fs_err_t file_system_init(void)
{
    fs_err_t    fs_err = FS_OK;
    uint8_t crc_flag = 0, i;
    do
    {
        flash_set_read_pagesize();

        startAddress = MFS_START_ADDRESS;

        info("File System position : %X, size : %d \n", startAddress, MFS_STORAGE_SIZE);
        for (i = 0; i < (0x1000 >> 8); i++)
        {
            while (flash_check_busy());
            //info("dw %x -> %x, %d\n", (uint32_t)&((uint8_t*)Cache_buf)[i * (0x1 << 8)], to_pos, size);
            flash_read_page((uint32_t) & ((uint8_t *)Cache_buf)[i * (0x1 << 8)], MFS_START_ADDRESS + (i * 0x100));
        }

        fc_read((uint32_t)&fs_head, 0, sizeof(fs_head));
        fc_read((uint32_t)&root, FILE_DESCRIB_OFFSET, sizeof(DirectoryDescriptor));
        fc_read((uint32_t)&fat,  DATA_INDEX_TABLE_OFFSET, sizeof(FileAllocationTable));

        for (i = 0; i < 3; i++)
        {
            if (fs_head.magic[i] != g_mfs_magic[i])
            {
                crc_flag = 1;
                break;
            }
        }
        if (crc_flag == 0)
        {
            info("Magic correct\n");
            fc_read((uint32_t)&root, FILE_DESCRIB_OFFSET, sizeof(DirectoryDescriptor));
            fc_read((uint32_t)&fat, DATA_INDEX_TABLE_OFFSET, sizeof(FileAllocationTable));
        }
        else
        {
            info("FileSystem header incorrect, reset\n");
            if (flash_erase(FLASH_ERASE_SECTOR, RT58X_FLASH_BASE_ADDR + MFS_START_ADDRESS) != 0)
            {
                //info("Flash control erase faild\n");
                fs_err = FS_ERR;

                break;
            }
            //info("Flash control erase success\n");
            initialFileDesc(&fat, &root);

            _memcpy_ram(fs_head.magic, (uint8_t *)g_mfs_magic, 3);
            fs_head.files = 0;

            _memcpy_ram(&Cache_buf[0], (uint8_t *)&fs_head, sizeof(fs_head_t));
            _memcpy_ram(&Cache_buf[FILE_DESCRIB_OFFSET], (uint8_t *)&root, sizeof(DirectoryDescriptor));
            _memcpy_ram(&Cache_buf[DATA_INDEX_TABLE_OFFSET], (uint8_t *)&fat, sizeof(FileAllocationTable));

            fs_flush();

        }
    } while (0);
    return fs_err;
}

int fs_open(const char *FileName)
{
    if (fs_head.files > MAX_FILES)
    {
        return -2;
    }
    int fileID = getFileID(FileName, &root);
    if (fileID != -1)
    {
        return fileID;
    }
    info("File <%s> not found, create.. ", FileName);

    fileID = fs_head.files++;
    createFile(FileName, &(root.table[fileID]));
    info("Success !!\n");
    root.table[fileID].index = getFreeNode(&fat);
    root.table[fileID].ptr = root.table[fileID].index;
    return fileID;
}
fs_err_t fs_close(int fileID)
{
    if (opened_files > MAX_FILES)
    {
        info("Error: File #%d does not exist.\n", fileID);
        return FS_ERR;
    }

    root.table[fileID].opened = 0;
    root.table[fileID].ptr = root.table[fileID].index;
    root.table[fileID].offset = 0;
    int i;
    for (i = 0; i < MAX_DATA_PAGES; i++)
    {
        fat.table[i].ptr = 0;
    }

    _memcpy_ram(&Cache_buf[0], (uint8_t *)&fs_head, sizeof(fs_head_t));
    _memcpy_ram(&Cache_buf[FILE_DESCRIB_OFFSET], (uint8_t *)&root, sizeof(DirectoryDescriptor));
    _memcpy_ram(&Cache_buf[DATA_INDEX_TABLE_OFFSET], (uint8_t *)&fat, sizeof(FileAllocationTable));

    fs_flush();
    return FS_OK;
}
int fs_list(void)
{
    int num_list = 0;
    fs_list_t list;
    do
    {
        int i = 0;
        info("Name\t\tOpend\t\tSize\n");
        for (i = 0; i < MAX_FILES; i++)
        {
            if (root.table[i].index != MAX_DATA_PAGES)
            {
                memcpy(list.fs_st[num_list].filename, root.table[i].filename, 8);
                list.fs_st[num_list].size = root.table[i].size;
                list.fs_st[num_list].opened = root.table[i].opened;
                num_list++;
                info("%s\t%s\t\t%d\n", list.fs_st[i].filename, list.fs_st[i].opened ? "Y" : "N", list.fs_st[i].size);
            }
            else
            {
                continue;
            }
        }
    } while (0);

    return num_list;
}

fs_err_t fs_seek(int fileID, uint32_t offset, e_fs_seek e_type)
{
    if (opened_files > MAX_FILES)
    {
        //info("Error: File #%d does not exist.\n", fileID);
        return FS_ID_INVALID;
    }

    if (offset > root.table[fileID].size)
    {
        return FS_OUT_OF_RANGE;
    }

    uint8_t count = 0, curr_idx = root.table[fileID].index;

    if (e_type == FS_SEEK_TAIL)
    {
        offset = root.table[fileID].size - offset;
    }

    if (offset < DATA_PAGE_SIZE)
    {
        root.table[fileID].offset = offset;
        root.table[fileID].ptr = root.table[fileID].index;
    }
    else
    {
        curr_idx = root.table[fileID].index;
        count = offset / DATA_PAGE_SIZE;

        while (count--)
        {
            curr_idx = getNextPage(curr_idx, &fat);
        }

        root.table[fileID].ptr = curr_idx;
        root.table[fileID].offset = offset % DATA_PAGE_SIZE;
    }
    return FS_OK;
}
size_t fs_write(int fileID, uint8_t *buf, int length)
{
    int addr = 2;
    int temp_len = 0;
    int count = 0;
    int i;
    int remain_len = 0;
    size_t w_size = 0;
    uint8_t w_data[DATA_PAGE_SIZE] = {0};

    do
    {
        if (opened_files > MAX_FILES)
        {
            break;
        }

        if (length < DATA_PAGE_SIZE)
        {
            temp_len = DATA_PAGE_SIZE;
            count = 1;
        }
        else
        {
            temp_len = length;
            count = temp_len / DATA_PAGE_SIZE + 1;
        }

        for (i = 0; i < count; i++)
        {
            memset(w_data, 0, sizeof(w_data));

            addr = root.table[fileID].ptr;

            fc_read((uintptr_t)w_data, DATA_BLOCK_OFFSET + (addr * DATA_PAGE_SIZE), DATA_PAGE_SIZE);

            remain_len = DATA_PAGE_SIZE - root.table[fileID].offset;
            if (length < remain_len)
            {
                remain_len = length;
            }
            _memcpy_ram(&w_data[root.table[fileID].offset], buf, remain_len);
            buf += remain_len;

            if (addr != MAX_DATA_PAGES && addr >= 0)
            {
                //fc_write((uintptr_t)w_data, MFS_START_ADDRESS + DATA_BLOCK_OFFSET + (addr * DATA_PAGE_SIZE), DATA_PAGE_SIZE);

                _memcpy_ram(&Cache_buf[DATA_BLOCK_OFFSET + (addr * DATA_PAGE_SIZE)], (uint8_t *)w_data, DATA_PAGE_SIZE);
                w_size += length < DATA_PAGE_SIZE ? length : DATA_PAGE_SIZE;
                if ((fat.table[addr].ptr + (length < DATA_PAGE_SIZE ? length : DATA_PAGE_SIZE)) >= DATA_PAGE_SIZE)
                {
                    fat.table[addr].ptr = DATA_PAGE_SIZE;
                }
                else
                {
                    fat.table[addr].ptr += length < DATA_PAGE_SIZE ? length : DATA_PAGE_SIZE;
                }



                if (w_size % DATA_PAGE_SIZE == 0)
                {
                    if (getNextPage(addr, &fat) == MAX_DATA_PAGES)
                    {
                        if (length)
                        {
                            root.table[fileID].ptr = getFreePage(&(root.table[fileID]), &fat);
                        }
                    }
                    else
                    {
                        root.table[fileID].ptr = getNextPage(addr, &fat);
                    }
                }
                root.table[fileID].offset = 0;

                if (length >= DATA_PAGE_SIZE)
                {
                    length -= remain_len;
                }
            }
            else
            {
                break;
            }
        }
        root.table[fileID].size = getFileSize(&(root.table[fileID]), &fat);
        _memcpy_ram(&Cache_buf[FILE_DESCRIB_OFFSET], (uint8_t *)&root, sizeof(DirectoryDescriptor));
        _memcpy_ram(&Cache_buf[DATA_INDEX_TABLE_OFFSET], (uint8_t *)&fat, sizeof(FileAllocationTable));

    } while (0);

    return w_size;
}
int fs_read(int fileID, uint8_t *buf, int length)
{
    if (opened_files > MAX_FILES && root.table[fileID].opened == 0 )
    {
        //info_color(LOG_RED, "No such file %d is opened\n", fileID);
        return 0;
    }
    if (root.table[fileID].size == 0)
    {
        return EOF;
    }

    return readFile(&(root.table[fileID]), &fat, length, buf);
}
size_t fs_get_file_size(const char *FileName)
{
    size_t filesize = 0;
    do
    {
        int fileID = getFileID(FileName, &root);
        if (fileID == -1)
        {
            break;
        }

        filesize = root.table[fileID].size;

    } while (0);

    return filesize;
}
static void createFile(const char *name, FileDescriptor *file)
{
    uint8_t name_len = 0, i = 0;
    while (name[i++])
    {
        name_len++;
    }
    memcpy(file->filename, name, name_len);
    file->opened = 1;
    file->ptr = 0;
    file->offset = 0;
    file->size   = 0;
}
static int getFileID(const char *name, DirectoryDescriptor *rootDir)
{
    int i = 0, fileID = -1;
    uint8_t name_len = 0;
    while (name[i++])
    {
        name_len++;
    }
    for (i = 0; i < MAX_FILES; i++)
    {
        if (memcmp(rootDir->table[i].filename, name, name_len) == 0)
        {
            fileID = i;
            break;
        }
    }

    if ( fileID != -1)
    {
        if (rootDir->table[fileID].opened == 1)
        {
            return -2;
        }
    }

    return fileID;
}

static int getFreeNode(FileAllocationTable *pfat)
{
    int i;
    for (i = 0; i < MAX_DATA_PAGES; i++)
    {
        if (pfat->table[i].free)
        {
            pfat->table[i].free = 0;
            return i;
        }
    }
    //info_color(LOG_RED, "Error: Cannot get free block.\n");
    return -1;
}

static int getFreePage(FileDescriptor *file, FileAllocationTable *pfat)
{
    int currNode = file->index;

    //info("pfat->table[%d].next = %d\n", currNode, pfat->table[currNode].next);

    while (pfat->table[currNode].next != MAX_DATA_PAGES)
    {
        currNode = pfat->table[currNode].next;
        file->ptr = currNode;
    }

    int nextNode = getFreeNode(pfat);
    pfat->table[currNode].next = nextNode;
    pfat->table[nextNode].next = MAX_DATA_PAGES;

    return nextNode < 0 ? -1 : nextNode;
}
static int getNextPage(int currNode, FileAllocationTable *pfat)
{
    currNode = pfat->table[currNode].next;

    return currNode;
}
static int getFileSize(FileDescriptor *file, FileAllocationTable *pfat)
{
    int currNode = file->index;
    int size = 0;
    while (pfat->table[currNode].next != MAX_DATA_PAGES)
    {
        size += pfat->table[currNode].ptr;
        currNode = pfat->table[currNode].next;
    }
    size += pfat->table[currNode].ptr;
    return size;
}
static int readFile(FileDescriptor *pfile, FileAllocationTable *pfat, int length, uint8_t *pdbuf)
{
    int curr, red = 0;
    int positionOnDisk = 0;
    int remain_len = 0;
    uint8_t *ptr;
    int read_len = 0;

    ptr = pdbuf;

    curr = pfile->ptr;

    for (; length != 0; curr = pfat->table[curr].next, red++)
    {
        positionOnDisk = curr;

        remain_len = DATA_PAGE_SIZE - pfile->offset;
        if (length < remain_len)
        {
            remain_len = length;
        }

        fc_read((uint32_t)ptr, DATA_BLOCK_OFFSET + (positionOnDisk * DATA_PAGE_SIZE) + pfile->offset, remain_len);
        pfile->offset = 0;

        length -= remain_len;
        ptr += remain_len;
        read_len += remain_len;
    }
    return read_len;
}

static void initialFileDesc(FileAllocationTable *pfat, DirectoryDescriptor *proot)
{
    int i;
    for (i = 0; i < MAX_DATA_PAGES; i++)
    {
        pfat->table[i].next = MAX_DATA_PAGES;
        pfat->table[i].free = 1;
        pfat->table[i].ptr = 0;
    }
    for (i = 0; i < MAX_FILES; i++)
    {
        memset(proot->table[i].filename, 0xff, sizeof(proot->table[i].filename));
        proot->table[i].size = 0;
        proot->table[i].index = MAX_DATA_PAGES;
    }
}
