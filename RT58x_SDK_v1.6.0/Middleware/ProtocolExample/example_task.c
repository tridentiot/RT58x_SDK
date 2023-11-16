/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file example_task.c
 *
 * @version 0.1
 * @date 2021/11/26
 * @license
 * @description
 */

//=============================================================================
//                Include
//=============================================================================
#include "sys_arch.h"
#include "example_task.h"
#include "mem_mgmt.h"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define EXAMPLE_TASK_SIZE       256
#define EXAMPLE_TASK_PRIORITY   TASK_PRIORITY_PROTOCOL_NORMAL
#define EXAMPLE_TASK_QUEUE_SIZE 8

#define NOTIFY_TASK_SIZE        128
#define NOTIFY_TASK_PRIORITY    TASK_PRIORITY_PROTOCOL_LOW
#define NOTIFY_TASK_QUEUE_SIZE  8

#define EXAMPLE_CFM_QUEUE_SIZE  2
//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================
/* Protocol Task */
static sys_queue_t example_queue;
static sys_task_t  example_task;
static sys_queue_t example_cfm_queue;

/* APP Notify Task */
static sys_queue_t app_notify_queue;
static sys_task_t  app_notify_task;
//=============================================================================
//                Functions
//=============================================================================
extern void example_app_notify(sys_tlv_t *pt_tlv);

static void example_task_handle(void *arg)
{
    task_queue_t t_queue = {0};
    task_queue_t t_cfm_queue;
    uint32_t u32_time;
    uint32_t *cfm_status;

    for (;;)
    {
        u32_time = sys_queue_recv(&example_queue, (void *)&t_queue, 0);

        if (u32_time != SYS_ARCH_TIMEOUT)
        {
            t_cfm_queue.pt_tlv = mem_malloc(sizeof(sys_tlv_t) + sizeof(uint32_t));
            t_cfm_queue.pt_tlv->type = 0xFFFF; // cfm type
            t_cfm_queue.pt_tlv->length = 4;    // cfm len
            cfm_status = (uint32_t *)t_cfm_queue.pt_tlv->value; // cfm value
            *cfm_status = 1;

            sys_queue_send(&example_cfm_queue, (void *)&t_cfm_queue);
            sys_queue_send(&app_notify_queue, (void *)&t_queue);
        }
    }
}

static void notify_task_handle(void *arg)
{
    task_queue_t t_queue = {0};
    uint32_t u32_time;
    sys_tlv_t *pt_tlv;
    for (;;)
    {
        u32_time = sys_queue_recv(&app_notify_queue, (void *)&t_queue, 0);

        if (u32_time != SYS_ARCH_TIMEOUT)
        {
            pt_tlv = t_queue.pt_tlv;
            example_app_notify(pt_tlv);
            mem_free(pt_tlv);
        }
    }
}
uint32_t example_wait_cfm(sys_tlv_t **pt_cfm_tlv, uint32_t u32_timeout)
{
    uint32_t u32_time;
    task_queue_t t_queue = {0};
    u32_time = sys_queue_recv(&example_cfm_queue, (void *)&t_queue, u32_timeout);
    if (u32_time != SYS_ARCH_TIMEOUT)
    {
        *pt_cfm_tlv = t_queue.pt_tlv;
    }
    return u32_time;
}
int example_task_init(void)
{
    int t_ret = 0;

    do
    {
        example_task = sys_task_new("example_t",
                                    example_task_handle,
                                    NULL,
                                    EXAMPLE_TASK_SIZE,
                                    EXAMPLE_TASK_PRIORITY);

        if (example_task == NULL)
        {
            t_ret = -1;
            break;
        }

        app_notify_task = sys_task_new("notify_t",
                                       notify_task_handle,
                                       NULL,
                                       NOTIFY_TASK_SIZE,
                                       NOTIFY_TASK_PRIORITY);

        if (app_notify_task == NULL)
        {
            t_ret = -1;
            break;
        }

        if (sys_queue_new(&example_queue,
                          EXAMPLE_TASK_QUEUE_SIZE,
                          sizeof(task_queue_t)) != ERR_OK)
        {
            t_ret = -1;
            break;
        }

        if (sys_queue_new(&app_notify_queue,
                          NOTIFY_TASK_QUEUE_SIZE,
                          sizeof(task_queue_t)) != ERR_OK)
        {
            t_ret = -1;
            break;
        }

        if (sys_queue_new(&example_cfm_queue,
                          EXAMPLE_CFM_QUEUE_SIZE,
                          sizeof(task_queue_t)) != ERR_OK)
        {
            t_ret = -1;
            break;
        }

        sys_timer_init(1);

    } while (0);

    return t_ret;
}

sys_err_t example_queue_sendto(sys_tlv_t *pt_tlv)
{
    sys_err_t t_return = SYS_ERR_OK;

    /*-----------------------------------*/
    /* A.Input Parameter Range Check     */
    /*-----------------------------------*/
    if (pt_tlv == NULL)
    {
        return SYS_ERR_SENDTO_POINTER_NULL;
    }

    /*-----------------------------------*/
    /* B. Main Functionality             */
    /*-----------------------------------*/
    do
    {
        task_queue_t t_queue;

        t_queue.u32_send_systick = sys_now();
        t_queue.pt_tlv = pt_tlv;

        if (sys_queue_send_with_timeout(&example_queue,
                                        (void *)&t_queue,
                                        5) == ERR_TIMEOUT)
        {
            t_return = SYS_ERR_SENDTO_FAIL;
            break;
        }
    } while (0);

    /*-----------------------------------*/
    /* C. Result & Return                */
    /*-----------------------------------*/
    return t_return;
}
