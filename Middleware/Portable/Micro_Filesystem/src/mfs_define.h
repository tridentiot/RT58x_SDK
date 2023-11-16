// ---------------------------------------------------------------------------
// All rights reserved.
//
// ---------------------------------------------------------------------------
#ifndef __MFS_DEFINE_H__
#define __MFS_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Include (Betterz\ to prevent)
//=============================================================================

//=============================================================================
//                Public Definitions of const value
//=============================================================================
#define MAX_DATA_PAGES               30
#define DATA_PAGE_SIZE               128 // bytes

#pragma pack(push, 1)
typedef struct FILE_SYSTEM_HEAD_T
{
    uint8_t magic[3];
    uint8_t files;
} fs_head_t;

typedef struct FileDescriptor
{
    uint8_t     filename[8];
    uint32_t    size             : 15;
    uint32_t    opened           : 1;
    uint32_t    index            : 5;
    uint32_t    ptr              : 5;
    uint32_t    offset           : 6;
} FileDescriptor;

typedef struct DirectoryDescriptor
{
    FileDescriptor table[16];
} DirectoryDescriptor;

typedef struct DATA_PAGE_T
{
    uint16_t    next    :   5;
    uint16_t    free    :   1;
    uint16_t    ptr     :   8;
    uint16_t            :   2;
} dp_t;

typedef struct FileAllocationTable
{
    dp_t table[MAX_DATA_PAGES];
} FileAllocationTable;
#pragma pack(pop)

#if defined(CONFIG_CPU_ARM_CM4)
typedef struct cross_addr
{
    union
    {
        uintptr_t   addr_value;
        uint64_t    reserved;
    };
} cross_addr_t;

typedef struct partition_entry
{
    uint32_t        uid;
    cross_addr_t    partition_start;
    uint32_t        partition_len;
    cross_addr_t    raw_addr;
    uint32_t        raw_len;

} partition_entry_t;

typedef struct global_data
{
    // auto boot or enter rom routine ('0xA5A5A500 | gpio_id' for authentication)
    uint32_t            trap_gpio_id;
    partition_entry_t   part_entries[CONFIG_PARTITION_NUM_PER_VOLUME];

} global_data_t;

typedef struct mem_map
{
    uintptr_t       m4_base_addr;
    uintptr_t       priv_base_addr;
    uint32_t        length;
} mem_map_t;

typedef enum partition_id
{
    PARTITION_ID_IMG_ATTR = 0,
    PARTITION_ID_M4,
    PARTITION_ID_GDATA,

    // optional
    PARTITION_ID_M0,
    PARTITION_ID_DSP,
    PARTITION_ID_CFG,
    PARTITION_ID_SYSLOG,
    PARTITION_ID_USR1,
    PARTITION_ID_USR2,
    PARTITION_ID_USR3,

    PARTITION_TOTAL_NUM,

} partition_id_t;
#endif
#define FILE_SYSTEM_HEAD_SIZE       sizeof(fs_head_t)
#define FILE_DESCRIB_OFFSET         FILE_SYSTEM_HEAD_SIZE
#define DATA_INDEX_TABLE_OFFSET     FILE_DESCRIB_OFFSET + sizeof(DirectoryDescriptor)
#define DATA_BLOCK_OFFSET           DATA_INDEX_TABLE_OFFSET + sizeof(FileAllocationTable)
//=============================================================================
//                Public ENUM
//=============================================================================

//=============================================================================
//                Public Struct
//=============================================================================

//=============================================================================
//                Public Function Declaration
//=============================================================================
const uint8_t g_mfs_magic[3] = {'R', 'A', 'F'};
#endif /* __MFS_DEFINE_H__ */
