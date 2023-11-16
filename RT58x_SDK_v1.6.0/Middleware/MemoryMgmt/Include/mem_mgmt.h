// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
#ifndef __MEM_MGMT_H__
#define __MEM_MGMT_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Include (Better to prevent)
//=============================================================================
#include <stdio.h>
#include <stdint.h>
#include "project_config.h"

//=============================================================================
//                Public Definitions of const value
//=============================================================================
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
#define MEM_MGMT_HEAP_SIZE           ((size_t)(SYS_HEAP_SIZE_BY_K << 10))
#else
#define MEM_MGMT_HEAP_SIZE           ((size_t)(10 << 10))
#endif
//=============================================================================
//                Public ENUM
//=============================================================================

//=============================================================================
//                Public Struct
//=============================================================================
#if !(MODULE_ENABLE(SUPPORT_MULTITASKING))
/* Used to pass information about the heap out of vPortGetHeapStats(). */
typedef struct xHeapStats
{
    size_t xAvailableHeapSpaceInBytes;      /* The total heap size currently available - this is the sum of all the free blocks, not the largest block that can be allocated. */
    size_t xSizeOfLargestFreeBlockInBytes;  /* The maximum size, in bytes, of all the free blocks within the heap at the time vPortGetHeapStats() is called. */
    size_t xSizeOfSmallestFreeBlockInBytes; /* The minimum size, in bytes, of all the free blocks within the heap at the time vPortGetHeapStats() is called. */
    size_t xNumberOfFreeBlocks;             /* The number of free memory blocks within the heap at the time vPortGetHeapStats() is called. */
    size_t xMinimumEverFreeBytesRemaining;  /* The minimum amount of total free memory (sum of all free blocks) there has been in the heap since the system booted. */
    size_t xNumberOfSuccessfulAllocations;  /* The number of calls to pvPortMalloc() that have returned a valid memory block. */
    size_t xNumberOfSuccessfulFrees;        /* The number of calls to vPortFree() that has successfully freed a block of memory. */
} HeapStats_t;
#endif
//=============================================================================
//                Public Function Declaration
//=============================================================================

uint32_t mem_GetHeapStartAddress(void);
uint32_t mem_GetHeapEndAddress(void);
size_t mem_GetFreeHeapSize(void);
size_t mem_GetMinimumEverFreeHeapSize(void);

//==================================
//     Memory functions
//==================================
void mem_mgmt_show_info(void);

/** Memory allocation
 * @param u32_size memory size in bytes
 */
void *mem_malloc_fn(uint32_t u32_size, const char *pc_func_ptr, uint32_t u32_line);
#define mem_malloc(len) mem_malloc_fn(len, __FUNCTION__, __LINE__)

/** Memory free
 * @param p_pointer
 */
void mem_free_fn(void *p_pointer, const char *pc_func_ptr, uint32_t u32_line);
#define mem_free(ptr) mem_free_fn(ptr, __FUNCTION__, __LINE__)

/** Memory copy
 * @param p_pointer_dest, p_pointer_src, lens
 */
void mem_copy_fn(void *p_pointer_dest, const void *p_pointer_src, uint32_t lens, const char *pc_func_ptr, uint32_t u32_line);
#define mem_memcpy(dest,src,len) mem_copy_fn(dest,src,len, __FUNCTION__, __LINE__)

#ifdef __cplusplus
};
#endif
#endif /* __MEM_MGMT_H__ */
