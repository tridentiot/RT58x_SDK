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

/* BSP APIs */
#include "bsp.h"
#include "bsp_console.h"
#include "bsp_led.h"
#include "bsp_button.h"

#include "mfs.h"

#include "uart_handler.h"

#include "multi_app.h"

#include "zigbee_gateway.h"
//=============================================================================
//                Global Variables
//=============================================================================
extern xQueueHandle g_app_msg_q;
SemaphoreHandle_t semaphore_zb;

//=============================================================================
//                Private Definitions of const value
//=============================================================================

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
//static uint32_t app_event_state;

static const char *db_file = "DB";

static zb_cmd_handler_parm_t zb_cmd_parm;

static uint8_t zb_cmd_buf[256] = { 0 };

//=============================================================================
//                Functions
//=============================================================================
void app_db_check(void)
{
    /* Open or Create file */
    int fp;

    fp = fs_open(db_file);

    fs_read(fp, (uint8_t *)&gt_app_db, sizeof(gt_app_db));

    fs_close(fp);
}

void app_db_update(void)
{
    /* Open or Create file */
    int fp;

    fp = fs_open(db_file);

    fs_seek(fp, 0, FS_SEEK_HEAD);
    fs_write(fp, (uint8_t *)&gt_app_db, sizeof(gt_app_db));

    fs_close(fp);
}

void zb_app_main(uint32_t event)
{
    uint8_t  extPANID[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t  resetflag;

    switch (event)
    {
    case APP_INIT_EVT:

        if (g_pan_id == 0xFFFF)
        {
            info_color(LOG_YELLOW, "PAN ID & Channel not found\n");
        }
        else
        {
            resetflag = ((g_channel & 0x80) == 0) ? 0 : 1;
            info_color(LOG_YELLOW, "zigbee_nwk_start_request( PAN: %04X, ch:%d, reset:%d %d %d)\n", g_pan_id, (g_channel & 0x7F), resetflag, ZIGBEE_CHANNEL_MASK((g_channel & 0x7F)), ZIGBEE_CHANNEL_MASK(g_channel & 0x7F));
            if (zigbee_nwk_start_request(ZIGBEE_DEVICE_ROLE_CORDINATOR, ZIGBEE_CHANNEL_MASK((g_channel & 0x7F)), 0x0, 20, g_pan_id, extPANID, resetflag) != 0)
            {
                info_color(LOG_YELLOW, "Start PAN failed\n");
            }
        }

        break;

    case APP_IDLE_EVT:

        break;
    case APP_ZB_START_EVT:
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

    case ZIGBEE_EVT_TYPE_RAF_CMD_CONFIRM:
        raf_cmd_cfm_handler(pt_zb_tlv);
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

void zigbee_gateway_cmd_recv(sys_tlv_t *pt_tlv)
{
    multi_app_queue_t t_app_q;

    if (xSemaphoreTake(semaphore_zb, 0) == pdPASS)
    {
        t_app_q.param_type = QUEUE_TYPE_APP_REQ;
        t_app_q.event = APP_QUEUE_GW_CMD_PROC_EVT;
        t_app_q.param.pt_data = (uint8_t *)pt_tlv;

        sys_queue_send_with_timeout(&g_app_msg_q, &t_app_q, 0);
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

void zb_cmd_recv(uint8_t *rx_buf, uint16_t len)
{
    /*  */
    static uint16_t total_len = 0;
    static uint16_t offset = 0;
    //uint8_t rx_buf[UART_HANDLER_RX_CACHE_SIZE] = { 0 };
    //int len;

    uint16_t msgbufflen = 0;
    uint32_t parser_status = 0;
    int i = 0;
    /*  */
    do
    {
        //len = bsp_uart_recv(rx_buf, sizeof(rx_buf));
        if (len)
        {
            if (total_len + len  > sizeof(zb_cmd_buf))
            {
                total_len = 0;
            }
            memcpy(zb_cmd_buf + total_len, rx_buf, len);
            total_len += len;
            for (i = 0; i < TRSPS_HANDLER_PARSER_CB_NUM; i++)
            {
                if (zb_cmd_parm.ZbCmdParserCB[i] == NULL)
                {
                    continue;
                }
                parser_status = zb_cmd_parm.ZbCmdParserCB[i](zb_cmd_buf, total_len, &msgbufflen, &offset);
                if (parser_status == ZB_DATA_VALID)
                {
                    if (zb_cmd_parm.ZbCmdRecvCB[i] == NULL)
                    {
                        break;
                    }
                    sys_tlv_t *pt_tlv = sys_malloc(sizeof(sys_tlv_t) + msgbufflen);

                    if (!pt_tlv)
                    {
                        break;
                    }
                    memcpy(pt_tlv->value, zb_cmd_buf + offset, msgbufflen);
                    pt_tlv->length = msgbufflen;
                    zb_cmd_parm.ZbCmdRecvCB[i](pt_tlv);
                    total_len -= msgbufflen;
                    break;
                }
                else if (parser_status == ZB_DATA_CS_ERROR)
                {
                    total_len = 0;
                    msgbufflen = 0;
                }
                else
                {

                    offset = 0;
                    msgbufflen = 0;
                }
            }
        }

    } while (0);
}

void zb_cmd_handler_init(zb_cmd_handler_parm_t *param)
{
    memcpy(&zb_cmd_parm, param, sizeof(zb_cmd_handler_parm_t));
}

void zb_event_get(uint8_t *p_data, uint8_t length)
{
    multi_app_queue_t t_app_q;

    if (xSemaphoreTake(semaphore_zb, 0) == pdPASS)
    {
        t_app_q.param_type = QUEUE_TYPE_APP_REQ;
        t_app_q.event = APP_QUEUE_ZB_MSG_EVT;
        t_app_q.data_len = length;
        t_app_q.param.pt_data = p_data;

        //if(sys_queue_send_with_timeout(&g_app_msg_q, &t_app_q, 0) == ERR_TIMEOUT)
        if (xQueueSendToBack(g_app_msg_q, &t_app_q, 5) != pdTRUE)
        {
            info_color(LOG_YELLOW, "zb_event_send fail %d\n", sys_queue_remaining_size(&g_app_msg_q));
        }
    }
    else
    {
        info_color(LOG_YELLOW, "zb event lost\n");
    }
}


