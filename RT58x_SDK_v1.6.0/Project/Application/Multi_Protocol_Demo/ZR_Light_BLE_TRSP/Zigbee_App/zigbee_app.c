/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_app.c
 *
 * @author Rex
 * @version 0.1
 * @date 2021/12/09
 * @license
 * @description
 */

//=============================================================================
//                Include
//=============================================================================
/* OS Wrapper APIs*/
#include "sys_arch.h"

/* Utility Library APIs */
#include "util_printf.h"
#include "util_log.h"

/* ZigBee Stack Library APIs */
#include "zigbee_stack_api.h"
#include "zigbee_app.h"

#include "zigbee_evt_handler.h"
#include "zigbee_zcl_msg_handler.h"
#include "zigbee_lib_api.h"
#include "task_pci.h"

/* BSP APIs */
#include "bsp.h"
#include "bsp_console.h"
#include "bsp_led.h"
#include "bsp_button.h"

/* MFS */
#include "mfs.h"

#include "multi_app.h"

//=============================================================================
//                Global Variables
//=============================================================================
extern xQueueHandle g_app_msg_q;
SemaphoreHandle_t semaphore_zb;
const char *scene_name = "DB";

//=============================================================================
//                Private Definitions of const value
//=============================================================================

//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================

const char *device_db_name = "DB";
//=============================================================================
//                Private Function Declaration
//=============================================================================

void device_db_check(void)
{
    /* Open or Create file */
    int fp;

    fp = fs_open(device_db_name);

    fs_read(fp, (uint8_t *)&device_table_db, sizeof(device_table_db));

    fs_close(fp);
}

void device_db_update(void)
{
    /* Open or Create file */
    int fp;

    fp = fs_open(device_db_name);

    fs_seek(fp, 0, FS_SEEK_HEAD);
    fs_write(fp, (uint8_t *)&device_table_db, sizeof(device_table_db));

    fs_close(fp);
}

//=============================================================================
//                Private Global Variables
//=============================================================================

//=============================================================================
//                Functions
//=============================================================================

void zb_app_main(uint32_t event)
{
    uint8_t button_0_state;
    uint8_t  extPANID[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //address all zero is able to join to any suitable PAN

    switch (event)
    {
    case APP_INIT_EVT:
        button_0_state = bsp_button_state_get(BSP_BUTTON_0);
        if (zigbee_nwk_start_request(ZIGBEE_DEVICE_ROLE_ROUTER, ZIGBEE_CHANNEL_ALL_MASK(), 0x0 /*ZIGBEE_CHANNEL_MASK(26)*/, 20, 0xFFFF, extPANID, !button_0_state) == 0)/*0: SYS_ERR_OK*/
        {
            device_db_check();
            if (!button_0_state)
            {
                memset(&device_table_db, 0, sizeof(device_table_db));
                device_db_update();
                info("clear scene DB\n");
            }
        }
        else
        {
            zigbee_app_evt_change(APP_INIT_EVT, false);

        }
        break;

    case APP_NOT_JOINED_EVT:
        zigbee_join_request();
        break;

    case APP_ZB_JOINED_EVT:
        gu32_join_state = 1;
        gu32_timer_100ms_cnt = 0;
        break;

    case APP_ZB_LEAVE_EVT:
        info("leave from BLE\n");
        zigbee_leave_req();
        break;

    case APP_ZB_MAC_ADDR_GET_EVT:
        zigbee_mac_address_get_req();
        break;

    case APP_ZB_IS_FACTORY_NEW_EVT:
        zigbee_is_factory_new_req();

        break;
    default:
        break;
    }
}

void zb_event_parse(sys_tlv_t *pt_zb_tlv)
{
    switch (pt_zb_tlv->type)
    {
    case ZIGBEE_EVT_TYPE_ZCL_DATA_IDC:
        zigbee_zcl_msg_handler(pt_zb_tlv);
        break;

    default:
        zigbee_evt_handler(pt_zb_tlv);
        break;
    }

    if (pt_zb_tlv)
    {
        vPortFree(pt_zb_tlv);
    }

}

void zb_app_evt_indication_cb(uint32_t data_len)
{
    int i32_err;
    uint8_t *pBuf;
    multi_app_queue_t t_app_q;

    if (xSemaphoreTake(semaphore_zb, 0) == pdPASS)
    {
        do
        {
            pBuf = pvPortMalloc(data_len);
            if (!pBuf)
            {
                xSemaphoreGive(semaphore_zb);
                break;
            }
            t_app_q.event = 0;
            i32_err = zigbee_event_msg_recvfrom(pBuf, &data_len);
            t_app_q.param_type = QUEUE_TYPE_ZIGBEE;
            t_app_q.param.pt_data = pBuf;


            if (i32_err == 0)
            {
                while (xQueueSendToBack(g_app_msg_q, &t_app_q, 20) != pdTRUE);
            }
            else
            {
                info_color(LOG_RED, "[%s] sys_err = %d !\n", __func__, i32_err);
                xSemaphoreGive(semaphore_zb);
                vPortFree(pBuf);
            }
        } while (0);
    }
}

uint8_t zigbee_app_evt_change(uint32_t evt, uint8_t from_isr)
{
    //app_event_state = evt;
    multi_app_queue_t t_app_q;

    if (xSemaphoreTake(semaphore_zb, 0) == pdPASS)
    {
        t_app_q.event = APP_QUEUE_ZIGBEE_EVT; // from Zigbee
        t_app_q.param_type = QUEUE_TYPE_APP_REQ;
        t_app_q.param.zb_app_req = evt;

        if (from_isr == false)
        {
            if (xQueueSendToBack(g_app_msg_q, &t_app_q, 20) != pdTRUE)
            {
                // send error
                xSemaphoreGive(semaphore_zb);
                return false;
            }
        }
        else
        {
            BaseType_t context_switch = pdFALSE;
            xQueueSendToBackFromISR(g_app_msg_q, &t_app_q, &context_switch);

            if (context_switch != pdTRUE)
            {
                // send error
                xSemaphoreGive(semaphore_zb);
                return false;
            }
        }
        return true;
    }
    return false;
}

