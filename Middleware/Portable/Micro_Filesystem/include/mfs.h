// ---------------------------------------------------------------------------
// All rights reserved.
//
// ---------------------------------------------------------------------------
#ifndef __MODULE_MFS_H__
#define __MODULE_MFS_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Include (Better to prevent)
//=============================================================================
#include "flashctl.h"
//=============================================================================
//                Public Definitions of const value
//=============================================================================
#define EOF                         (-1)
#define EMPTY                       (-69)
#define MAX_FILES                    4
//=============================================================================
//                Public ENUM
//=============================================================================
typedef enum FILE_SYSTEM_ERR_T
{
    FS_OK = 0,
    FS_ERR,
    FS_NO_SPACE,
    FS_FILE_EXIST,
    FS_NOT_FOUND,
    FS_ID_INVALID,
    FS_OUT_OF_RANGE,
    FS_WRITE_FAILED,
} fs_err_t;

typedef enum FILE_SYSTEM_SEEK_NMODE
{
    FS_SEEK_HEAD = 0,
    FS_SEEK_TAIL,
} e_fs_seek;

typedef struct FILE_STRUCTURE_T
{
    uint8_t     filename[8];
    uint32_t    size             : 15;
    uint32_t    opened           : 1;
    uint32_t                     : 16;
} fs_st_t;
typedef struct FILE_LIST_T
{
    uint32_t    count;
    fs_st_t     fs_st[MAX_FILES];
} fs_list_t;

//=============================================================================
//                Public Struct
//=============================================================================

//=============================================================================
//                Public Function Declaration
//=============================================================================
fs_err_t file_system_init(void);
fs_err_t fs_close(int fileID);
fs_err_t fs_seek(int fileID, uint32_t offset, e_fs_seek e_type);
void filesystem_reset(void);
int fs_read(int fileID, uint8_t *buf, int length);
int fs_open(const char *FileName);
int fs_list(void);
size_t fs_write(int fileID, uint8_t *buf, int length);
size_t fs_get_file_size(const char *FileName);

#endif /* __MODULE_MFS_H__ */
