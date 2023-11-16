/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file example_api.c
 *
 * @version 0.1
 * @date 2021/11/26
 * @license
 * @description
 */

//=============================================================================
//                Includes
//=============================================================================
#include "sys_arch.h"
#include "example_task.h"
#include "example_api.h"
#include "mem_mgmt.h"
#include "util_log.h"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define APP_ALIGN_4_BYTES(a)    (((uint32_t)(a) + 0x3) & ~0x3)

//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================
static sys_tlv_t *g_app_ptlv = NULL;
static sys_sem_t g_app_sem;
static pf_evt_indication *pf_app_indication;

//=============================================================================
//                Functions
//=============================================================================
void example_app_notify(sys_tlv_t *pt_tlv)
{
    if (pt_tlv != NULL)
    {
        uint32_t u32_total_len;
        u32_total_len = pt_tlv->length + sizeof(example_tlv_t);

        while (1)
        {
            uint32_t u32_timeout = 0;
            g_app_ptlv = pt_tlv;

            info("%s wait app recv TLV %p(%d)\n", sys_current_thread_name(), pt_tlv, u32_total_len);
            if (!pf_app_indication)
            {
                err("%s APP notification callback not installed\n", sys_current_thread_name());
                break;
            }
            pf_app_indication(u32_total_len);
            // wait APP receive tlv packet
            u32_timeout = sys_sem_wait(&g_app_sem, 3000);
            if (u32_timeout == SYS_ARCH_TIMEOUT)
            {
                info("%s wait app receive pkt timeout\n", sys_current_thread_name());
            }
            else
            {
                break;
            }
        }
    }

}

int example_stack_init(example_cfg_t *pt_cfg)
{
    int i32_ret = 0;

    do
    {
        /*-----------------------------------*/
        /* A.Input Parameter Range Check     */
        /*-----------------------------------*/
        if (pt_cfg == NULL)
        {
            i32_ret = -1;
            break;
        }

        /*-----------------------------------*/
        /* B. Main Functionality             */
        /*-----------------------------------*/
        pf_app_indication = pt_cfg->pf_evt_indication;
        sys_binary_sem_new(&g_app_sem);

        /* Initial protocol stack tasks herer */
        example_task_init();

    } while (0);

    /*-----------------------------------*/
    /* C. Result & Return                */
    /*-----------------------------------*/
    return i32_ret;
}
int example_event_msg_recvfrom(uint8_t *pu8_buf, uint32_t *pu32_buf_len)
{
    uint32_t u32_tlv_length;
    do
    {
        /*-----------------------------------*/
        /* A.Input Parameter Range Check     */
        /*-----------------------------------*/
        if (pu8_buf == NULL)
        {
            return -1;
        }
        if (g_app_ptlv == NULL)
        {
            return -2;
        }
        u32_tlv_length = g_app_ptlv->length + sizeof(example_tlv_t);
        if (*pu32_buf_len < u32_tlv_length)
        {
            return -3;
        }

        /*-----------------------------------*/
        /* B. Main Functionality             */
        /*-----------------------------------*/
        *pu32_buf_len = u32_tlv_length;
        memcpy(pu8_buf, g_app_ptlv, u32_tlv_length);
        g_app_ptlv = NULL;
        sys_sem_signal(&g_app_sem);

    } while (0);
    /*-----------------------------------*/
    /* C. Result & Return                */
    /*-----------------------------------*/
    return 0;
}
int example_event_msg_sendto(example_tlv_t *pt_tlv)
{
    sys_tlv_t *pt_new_tlv = NULL;
    sys_tlv_t *pt_cfm_tlv = NULL;
    uint32_t u32_new_size;
    int sys_rtn = SYS_ERR_OK;


    do
    {
        /*-----------------------------------*/
        /* A.Input Parameter Range Check     */
        /*-----------------------------------*/
        if (pt_tlv == NULL)
        {
            sys_rtn = SYS_ERR_SENDTO_POINTER_NULL;
            break;
        }

        /*-----------------------------------*/
        /* B. Main Functionality             */
        /*-----------------------------------*/
        u32_new_size = APP_ALIGN_4_BYTES(pt_tlv->length) + sizeof(sys_tlv_t);

        pt_new_tlv = (sys_tlv_t *)mem_malloc(u32_new_size);
        if (pt_new_tlv == NULL)
        {
            sys_rtn = SYS_ERR_DATA_MALLOC_FAIL;
            break;
        }
        memcpy(pt_new_tlv, pt_tlv, u32_new_size);

        sys_rtn = example_queue_sendto(pt_new_tlv);

        info("Sento status %d ... ", sys_rtn);
        if (sys_rtn == SYS_ERR_OK)
        {
            info("Wait cfm ");
            if (example_wait_cfm(&pt_cfm_tlv, 1) != SYS_ARCH_TIMEOUT)
            {
                sys_rtn = (int) * pt_cfm_tlv->value;
                info("\nRecv cfm status %d\n", sys_rtn);
                mem_free(pt_cfm_tlv);
            }
            else
            {
                info("Timeout!\n");
                sys_rtn = SYS_BUSY;
            }
        }


    } while (0);

    /*-----------------------------------*/
    /* C. Result & Return                */
    /*-----------------------------------*/
    return sys_rtn;
}
