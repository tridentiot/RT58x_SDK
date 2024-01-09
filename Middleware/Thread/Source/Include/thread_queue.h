/**
 * @file thread_queue.h
 * @author Jiemin Cao(jiemin.cao@rafaelmicro.com)
 * @brief
 * @version 0.1
 * @date 2023-08-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __THREAD_QUEUE_H__
#define __THREAD_QUEUE_H__

#include "util_queue.h"

#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
//                Include (Better to prevent)
//=============================================================================

//=============================================================================
//                Public Definitions of const value
//=============================================================================

//=============================================================================
//                Public ENUM
//=============================================================================

//=============================================================================
//                Public Struct
//=============================================================================
// thread queue struct
typedef struct
{
    queue_t thread_queue;
    uint32_t size; //queue sizes
    uint32_t count; // current number of queues
    uint32_t dataSize; // ponter sizes
} thread_queue_t;
//=============================================================================
//                Public Function Declaration
//=============================================================================
void thread_queue_init(thread_queue_t *queue, int size, uint32_t dataSize);
int thread_enqueue(thread_queue_t *queue, void *data) ;
int thread_dequeue(thread_queue_t *queue, void *data);
#ifdef __cplusplus
};
#endif
#endif /* __OTA_HANDLER_H__ */
