#include "stdint.h"
#include "cm3_mcu.h"
#include "project_config.h"

#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
#include "FreeRTOS.h"
#include "task.h"

#else

#define portBYTE_ALIGNMENT          8
#define portMAX_DELAY ( uint32_t ) 0xffffffffUL
#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#include <assert.h>

#define configASSERT(n) assert(n)

#endif /*(MODULE_ENABLE(SUPPORT_MULTITASKING))*/

#include "util_log.h"

#include "mem_mgmt.h"


#define MEM_MGMT_DEBUG_INFO_TASK_SIZE         20
#define MEM_MGMT_DEBUG_INFO_SIZE              100

#define __mem_mgmt_pool          __attribute__ ((used, section("mem_mgmt_pool")))

typedef struct MEM_MGMT_DEBUG_LOG_ENTRY
{
    char *func;
    uint32_t file_line  : 16;   // max 0xFFFF = 65535
    uint32_t malloc_len : 16;   // max 0xFFFF = 65535

    uint32_t malloc_offset  : 19;   // max 256k = 0x40000(19 bits)
    uint32_t valid          : 1;
    uint32_t                : 4;
    uint32_t task_index     : 8;
} malloc_log_entry_t;

typedef struct MEM_MGMT_DEBUG_LOG
{
    char *task_name_ptr[MEM_MGMT_DEBUG_INFO_TASK_SIZE];
    uint32_t malloc_base_addr;
    malloc_log_entry_t entry[MEM_MGMT_DEBUG_INFO_SIZE];
} malloc_log_t;

static malloc_log_t g_malloc_log;


/* Block sizes must not get too small. */
#define MEM_MGMT_HEAP_MINIMUM_BLOCK_SIZE        ( ( size_t ) ( mem_HeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define MEM_MGMT_HEAP_BITS_PER_BYTE             ( ( size_t ) 8 )

static uint8_t mem_Heap[ MEM_MGMT_HEAP_SIZE ] __mem_mgmt_pool;
/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pNextFreeBlock;  /*<< The next free block in the list. */
    size_t BlockSize;                     /*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void mem_prvInsertBlockIntoFreeList(BlockLink_t *pBlockToInsert);

/*
 * Called automatically to setup the required heap structures the first time
 * mem_Malloc() is called.
 */
static void mem_prvHeapInit(void)  ;

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t mem_HeapStructSize = ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t mem_Start, *mem_pEnd = NULL;

/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
static size_t mem_FreeBytesRemaining = 0U;
static size_t mem_MinimumEverFreeBytesRemaining = 0U;
static size_t mem_NumberOfSuccessfulAllocations = 0;
static size_t mem_NumberOfSuccessfulFrees = 0;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
 * member of an BlockLink_t structure is set then the block belongs to the
 * application.  When the bit is free the block is still part of the free heap
 * space. */
static size_t mem_BlockAllocatedBit = 0;

/*-----------------------------------------------------------*/

static void *_mem_malloc( size_t WantedSize )
{
    BlockLink_t *pBlock, *pPreviousBlock, *pNewBlockLink;
    void *pReturn = NULL;

#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    vTaskSuspendAll();
#else
#endif
    {
        /* If this is the first call to malloc then the heap will require
         * initialisation to setup the list of free blocks. */
        if ( mem_pEnd == NULL )
        {
            mem_prvHeapInit();
        }

        /* Check the requested block size is not so large that the top bit is
         * set.  The top bit of the block size member of the BlockLink_t structure
         * is used to determine who owns the block - the application or the
         * kernel, so it must be free. */
        if ( ( WantedSize & mem_BlockAllocatedBit ) == 0 )
        {
            /* The wanted size must be increased so it can contain a BlockLink_t
             * structure in addition to the requested amount of bytes. */
            if ( ( WantedSize > 0 ) &&
                    ( ( WantedSize + mem_HeapStructSize ) >  WantedSize ) ) /* Overflow check */
            {
                WantedSize += mem_HeapStructSize;

                /* Ensure that blocks are always aligned. */
                if ( ( WantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
                {
                    /* Byte alignment required. Check for overflow. */
                    if ( ( WantedSize + ( portBYTE_ALIGNMENT - ( WantedSize & portBYTE_ALIGNMENT_MASK ) ) )
                            > WantedSize )
                    {
                        WantedSize += ( portBYTE_ALIGNMENT - ( WantedSize & portBYTE_ALIGNMENT_MASK ) );
                        configASSERT( ( WantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
                    }
                    else
                    {
                        WantedSize = 0;
                    }
                }
            }
            else
            {
                WantedSize = 0;
            }

            if ( ( WantedSize > 0 ) && ( WantedSize <= mem_FreeBytesRemaining ) )
            {
                /* Traverse the list from the start (lowest address) block until
                 * one of adequate size is found. */
                pPreviousBlock = &mem_Start;
                pBlock = mem_Start.pNextFreeBlock;

                while ( ( pBlock->BlockSize < WantedSize ) && ( pBlock->pNextFreeBlock != NULL ) )
                {
                    pPreviousBlock = pBlock;
                    pBlock = pBlock->pNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                 * was not found. */
                if ( pBlock != mem_pEnd )
                {
                    /* Return the memory space pointed to - jumping over the
                     * BlockLink_t structure at its start. */
                    pReturn = ( void * ) ( ( ( uint8_t * ) pPreviousBlock->pNextFreeBlock ) + mem_HeapStructSize );

                    /* This block is being returned for use so must be taken out
                     * of the list of free blocks. */
                    pPreviousBlock->pNextFreeBlock = pBlock->pNextFreeBlock;

                    /* If the block is larger than required it can be split into
                     * two. */
                    if ( ( pBlock->BlockSize - WantedSize ) > MEM_MGMT_HEAP_MINIMUM_BLOCK_SIZE )
                    {
                        /* This block is to be split into two.  Create a new
                         * block following the number of bytes requested. The void
                         * cast is used to prevent byte alignment warnings from the
                         * compiler. */
                        pNewBlockLink = ( void * ) ( ( ( uint8_t * ) pBlock ) + WantedSize );
                        configASSERT( ( ( ( size_t ) pNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );
                        /* Calculate the sizes of two blocks split from the
                         * single block. */
                        pNewBlockLink->BlockSize = pBlock->BlockSize - WantedSize;
                        pBlock->BlockSize = WantedSize;

                        /* Insert the new block into the list of free blocks. */
                        mem_prvInsertBlockIntoFreeList( pNewBlockLink );
                    }

                    mem_FreeBytesRemaining -= pBlock->BlockSize;

                    if ( mem_FreeBytesRemaining < mem_MinimumEverFreeBytesRemaining )
                    {
                        mem_MinimumEverFreeBytesRemaining = mem_FreeBytesRemaining;
                    }


                    /* The block is being returned - it is allocated and owned
                     * by the application and has no "next" block. */
                    pBlock->BlockSize |= mem_BlockAllocatedBit;
                    pBlock->pNextFreeBlock = NULL;
                    mem_NumberOfSuccessfulAllocations++;
                }
            }
        }
    }
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    ( void ) xTaskResumeAll();
#else
#endif

    configASSERT( ( ( ( size_t ) pReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );
    return pReturn;
}
/*-----------------------------------------------------------*/

static void _mem_free(void *pv, const char *pc_func_ptr, uint32_t u32_line)
{
    uint8_t *puc = (uint8_t *) pv;
    BlockLink_t *pLink;

    if ( pv != NULL )
    {
        if ( ((uint32_t)pv < (uint32_t)mem_Heap) || ((uint32_t)pv > (uint32_t)&mem_Heap[ MEM_MGMT_HEAP_SIZE ]) )
        {
            err("mem_free 0x%p error %s-%d(%s)\n", pv, pc_func_ptr, u32_line);
        }
        configASSERT( ((uint32_t)pv >= (uint32_t)mem_Heap) && ((uint32_t)pv <= (uint32_t)&mem_Heap[ MEM_MGMT_HEAP_SIZE ]) );
        /* The memory being freed will have an BlockLink_t structure immediately
         * before it. */
        puc -= mem_HeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pLink = ( void * ) puc;

        /* Check the block is actually allocated. */
        configASSERT( ( pLink->BlockSize & mem_BlockAllocatedBit ) != 0 );
        configASSERT( pLink->pNextFreeBlock == NULL );

        if ( ( pLink->BlockSize & mem_BlockAllocatedBit ) != 0 )
        {
            if ( pLink->pNextFreeBlock == NULL )
            {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                pLink->BlockSize &= ~mem_BlockAllocatedBit;
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
                vTaskSuspendAll();
#else
#endif
                {
                    /* Add this block to the list of free blocks. */
                    mem_FreeBytesRemaining += pLink->BlockSize;
                    mem_prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pLink ) );
                    mem_NumberOfSuccessfulFrees++;
                }
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
                ( void ) xTaskResumeAll();
#else
#endif
            }
        }
    }
}
/*-----------------------------------------------------------*/

uint32_t mem_GetHeapStartAddress( void )
{
    return (uint32_t)mem_Heap;
}
/*-----------------------------------------------------------*/

uint32_t mem_GetHeapEndAddress( void )
{
    return (uint32_t)(mem_Heap + sizeof(mem_Heap));
}
/*-----------------------------------------------------------*/
size_t mem_GetFreeHeapSize( void )
{
    return mem_FreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t mem_GetMinimumEverFreeHeapSize( void )
{
    return mem_MinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

static void mem_prvHeapInit( void ) /* PRIVILEGED_FUNCTION */
{
    BlockLink_t *pFirstFreeBlock;
    uint8_t *pAlignedHeap;
    size_t Address;
    size_t TotalHeapSize = MEM_MGMT_HEAP_SIZE;

    /* Ensure the heap starts on a correctly aligned boundary. */
    Address = ( size_t ) mem_Heap;

    if ( ( Address & portBYTE_ALIGNMENT_MASK ) != 0 )
    {
        Address += ( portBYTE_ALIGNMENT - 1 );
        Address &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
        TotalHeapSize -= Address - ( size_t ) mem_Heap;
    }

    pAlignedHeap = ( uint8_t * ) Address;

    /* xStart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    mem_Start.pNextFreeBlock = ( void * ) pAlignedHeap;
    mem_Start.BlockSize = ( size_t ) 0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    Address = ( ( size_t ) pAlignedHeap ) + TotalHeapSize;
    Address -= mem_HeapStructSize;
    Address &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
    mem_pEnd = ( void * ) Address;
    mem_pEnd->BlockSize = 0;
    mem_pEnd->pNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd. */
    pFirstFreeBlock = ( void * ) pAlignedHeap;
    pFirstFreeBlock->BlockSize = Address - ( size_t ) pFirstFreeBlock;
    pFirstFreeBlock->pNextFreeBlock = mem_pEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    mem_MinimumEverFreeBytesRemaining = pFirstFreeBlock->BlockSize;
    mem_FreeBytesRemaining = pFirstFreeBlock->BlockSize;

    /* Work out the position of the top bit in a size_t variable. */
    mem_BlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * MEM_MGMT_HEAP_BITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void mem_prvInsertBlockIntoFreeList( BlockLink_t *pBlockToInsert ) /* PRIVILEGED_FUNCTION */
{
    BlockLink_t *pIterator;
    uint8_t *pc;

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for ( pIterator = &mem_Start; pIterator->pNextFreeBlock < pBlockToInsert; pIterator = pIterator->pNextFreeBlock )
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    pc = ( uint8_t * ) pIterator;

    if ( ( pc + pIterator->BlockSize ) == ( uint8_t * ) pBlockToInsert )
    {
        pIterator->BlockSize += pBlockToInsert->BlockSize;
        pBlockToInsert = pIterator;
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    pc = ( uint8_t * ) pBlockToInsert;

    if ( ( pc + pBlockToInsert->BlockSize ) == ( uint8_t * ) pIterator->pNextFreeBlock )
    {
        if ( pIterator->pNextFreeBlock != mem_pEnd )
        {
            /* Form one big block from the two blocks. */
            pBlockToInsert->BlockSize += pIterator->pNextFreeBlock->BlockSize;
            pBlockToInsert->pNextFreeBlock = pIterator->pNextFreeBlock->pNextFreeBlock;
        }
        else
        {
            pBlockToInsert->pNextFreeBlock = mem_pEnd;
        }
    }
    else
    {
        pBlockToInsert->pNextFreeBlock = pIterator->pNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
     * before and the block after, then it's pxNextFreeBlock pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if ( pIterator != pBlockToInsert )
    {
        pIterator->pNextFreeBlock = pBlockToInsert;
    }
}
/*-----------------------------------------------------------*/
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
void mem_vPortGetHeapStats( HeapStats_t *pxHeapStats )
{
    BlockLink_t *pxBlock;
    size_t xBlocks = 0, xMaxSize = 0, xMinSize = portMAX_DELAY; /* portMAX_DELAY used as a portable way of getting the maximum value. */

#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    vTaskSuspendAll();
#else
#endif
    {
        pxBlock = mem_Start.pNextFreeBlock;

        /* pxBlock will be NULL if the heap has not been initialised.  The heap
         * is initialised automatically when the first allocation is made. */
        if ( pxBlock != NULL )
        {
            do
            {
                /* Increment the number of blocks and record the largest block seen
                 * so far. */
                xBlocks++;

                if ( pxBlock->BlockSize > xMaxSize )
                {
                    xMaxSize = pxBlock->BlockSize;
                }

                if ( pxBlock->BlockSize < xMinSize )
                {
                    xMinSize = pxBlock->BlockSize;
                }

                /* Move to the next block in the chain until the last block is
                 * reached. */
                pxBlock = pxBlock->pNextFreeBlock;
            } while ( pxBlock != mem_pEnd );
        }
    }
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    ( void ) xTaskResumeAll();
#else
#endif

    pxHeapStats->xSizeOfLargestFreeBlockInBytes = xMaxSize;
    pxHeapStats->xSizeOfSmallestFreeBlockInBytes = xMinSize;
    pxHeapStats->xNumberOfFreeBlocks = xBlocks;

#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    taskENTER_CRITICAL();
#else
#endif
    {
        pxHeapStats->xAvailableHeapSpaceInBytes = mem_FreeBytesRemaining;
        pxHeapStats->xNumberOfSuccessfulAllocations = mem_NumberOfSuccessfulAllocations;
        pxHeapStats->xNumberOfSuccessfulFrees = mem_NumberOfSuccessfulFrees;
        pxHeapStats->xMinimumEverFreeBytesRemaining = mem_MinimumEverFreeBytesRemaining;
    }
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    taskEXIT_CRITICAL();
#else
#endif
}
#endif
// mgmt
//==================================
//     Memory functions
//==================================
/** Task name pointer transfor to table index
 * @param task_name task name pointer
 * @return table index
 */
static uint8_t search_current_task_name_index(void)
{
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    char *current_task_name = pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
#else
    char *current_task_name = "deadbeef";
#endif
    uint32_t u32_idx;

    for (u32_idx = 0; u32_idx < MEM_MGMT_DEBUG_INFO_TASK_SIZE; u32_idx++)
    {
        if (g_malloc_log.task_name_ptr[u32_idx] == NULL)
        {
            g_malloc_log.task_name_ptr[u32_idx] = current_task_name;
            break;
        }
        else if (g_malloc_log.task_name_ptr[u32_idx] == current_task_name)
        {
            break;
        }
    }

    if (u32_idx < MEM_MGMT_DEBUG_INFO_TASK_SIZE)
    {
        return u32_idx;
    }
    else
    {
        return 0xFF;
    }
}

static void malloc_info_insert(void *p_mptr, uint32_t u32_msize, char *pc_func_ptr, uint32_t u32_line)
{
    uint32_t u32_idx;

    if (g_malloc_log.malloc_base_addr == 0)
    {
        g_malloc_log.malloc_base_addr = (uint32_t)p_mptr & 0xFFF00000;
    }

    for (u32_idx = 0; u32_idx < MEM_MGMT_DEBUG_INFO_SIZE; u32_idx++)
    {
        if (g_malloc_log.entry[u32_idx].valid == 0)
        {
            g_malloc_log.entry[u32_idx].func = pc_func_ptr;
            g_malloc_log.entry[u32_idx].file_line = u32_line & 0xFFFF;
            g_malloc_log.entry[u32_idx].malloc_len = u32_msize & 0xFFFF;
            g_malloc_log.entry[u32_idx].malloc_offset = (uint32_t)p_mptr & 0x7FFFF; // max 512k
            g_malloc_log.entry[u32_idx].task_index = search_current_task_name_index();
            g_malloc_log.entry[u32_idx].valid = 1;
            break;
        }
    }
}

static void malloc_info_delete(void *p_mptr)
{
    uint32_t u32_idx, malloc_offset;

    malloc_offset = (uint32_t)p_mptr & 0xFFFFF;

    for (u32_idx = 0; u32_idx < MEM_MGMT_DEBUG_INFO_SIZE; u32_idx++)
    {
        if (g_malloc_log.entry[u32_idx].valid)
        {
            if (g_malloc_log.entry[u32_idx].malloc_offset == malloc_offset)
            {
                memset(&g_malloc_log.entry[u32_idx], 0x0, sizeof(g_malloc_log.entry[u32_idx]));
                break;
            }
        }
    }
}

void mem_mgmt_show_info(void)
{
    uint32_t u32_idx;
    info("+------------+-------+---------------------------\n");
    info("|  Pointer   | Size  | Func-Line(Task)\n");
    info("+------------+-------+---------------------------\n");
    for (u32_idx = 0; u32_idx < MEM_MGMT_DEBUG_INFO_SIZE; u32_idx++)
    {
        if (g_malloc_log.entry[u32_idx].valid)
        {
            info("| 0x%08X | %05u | %s-%d(%s)\n",
                 g_malloc_log.malloc_base_addr | g_malloc_log.entry[u32_idx].malloc_offset,
                 g_malloc_log.entry[u32_idx].malloc_len,
                 g_malloc_log.entry[u32_idx].func,
                 g_malloc_log.entry[u32_idx].file_line,
                 g_malloc_log.task_name_ptr[g_malloc_log.entry[u32_idx].task_index]);
        }
    }
    info("+------------+-------+---------------------------\n");
    info("   Free Size : 0x%06X(%d), Min 0x%06X(%d)\n", mem_GetFreeHeapSize(), mem_GetFreeHeapSize(),
         mem_GetMinimumEverFreeHeapSize(), mem_GetMinimumEverFreeHeapSize());
}
/** Memory allocation
 * @param u32_size sleep time in milliseconds
 */
void *mem_malloc_fn(uint32_t u32_size, const char *pc_func_ptr, uint32_t u32_line)
{
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    vTaskSuspendAll();
#else
    enter_critical_section();
#endif
    void *ptr;
    /*-----------------------------------*/
    /* A.Input Parameter Range Check     */
    /*-----------------------------------*/
    if (u32_size == 0)
    {
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
        xTaskResumeAll();
#else
        leave_critical_section();
#endif
        return NULL;
    }

    if (u32_size & portBYTE_ALIGNMENT_MASK)
    {
        u32_size += portBYTE_ALIGNMENT - (u32_size & portBYTE_ALIGNMENT_MASK);
    }

    /*-----------------------------------*/
    /* B. Main Functionality             */
    /*-----------------------------------*/
    ptr = _mem_malloc(u32_size);

    if (ptr)
    {
        malloc_info_insert(ptr, u32_size, (char *)pc_func_ptr, u32_line);
    }
    /*-----------------------------------*/
    /* C. Result & Return                */
    /*-----------------------------------*/
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    xTaskResumeAll();
#else
    leave_critical_section();
#endif
    return ptr;
}

/** Memory free
 * @param p_pointer
 */
void mem_free_fn(void *p_pointer, const char *pc_func_ptr, uint32_t u32_line)
{
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    vTaskSuspendAll();
#else
    enter_critical_section();
#endif
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    char *current_task_name = pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
#else
    char *current_task_name = "deadbeef";
#endif
    /*-----------------------------------*/
    /* A.Input Parameter Range Check     */
    /*-----------------------------------*/
    if (p_pointer == NULL)
    {
        info("%s-%d(%s) free null pointer\n",
             pc_func_ptr, u32_line, current_task_name);
    }
    configASSERT(p_pointer != NULL);

    /*-----------------------------------*/
    /* B. Main Functionality             */
    /*-----------------------------------*/
    _mem_free(p_pointer, pc_func_ptr, u32_line);

    malloc_info_delete(p_pointer);

    /*-----------------------------------*/
    /* C. Result & Return                */
    /*-----------------------------------*/
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    xTaskResumeAll();
#else
    leave_critical_section();
#endif
}



/** Memory copy
 * @param p_pointer_dest, p_pointer_src, lens
 */
void mem_copy_fn(void *p_pointer_dest, const void *p_pointer_src, uint32_t lens, const char *pc_func_ptr, uint32_t u32_line)
{
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    vTaskSuspendAll();
#else
    enter_critical_section();
#endif
    char *d = p_pointer_dest;
    const char *s = p_pointer_src;
    while (lens--)
    {
        *d++ = *s++;
    }
#if (MODULE_ENABLE(SUPPORT_MULTITASKING))
    xTaskResumeAll();
#else
    leave_critical_section();
#endif
}
