/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_zcl_msg_handler.c
 *
 * @author Rex
 * @version 0.1
 * @date 2021/12/23
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
#include "mfs.h"

/* ZigBee Stack Library APIs */
#include "zigbee_stack_api.h"
#include "zigbee_app.h"
#include "zigbee_lib_api.h"
#include "util_string.h"

#include "bsp_led.h"
static TimerHandle_t tmr_identify;
extern uint16_t short_addr;
//=============================================================================
//                Private Function Declaration
//=============================================================================

static void tmr_identify_cb(TimerHandle_t t_timer)
{
    uint8_t remaining_time;
    remaining_time = get_identify_time();

    bsp_led_toggle(BSP_LED_0);
    if (remaining_time == 0)
    {
        info("Identify complete\n");
        bsp_led_Off(BSP_LED_0);
    }
    else
    {
        xTimerStart(tmr_identify, 0);
    }
}
static void _zcl_common_command_process(uint16_t cmd, uint16_t datalen, uint8_t *pdata, uint32_t clusterID)
{
    do
    {
        if (cmd == ZB_ZCL_CMD_WRITE_ATTRIB ||
                cmd == ZB_ZCL_CMD_WRITE_ATTRIB_UNDIV ||
                cmd == ZB_ZCL_CMD_WRITE_ATTRIB_RESP ||
                cmd == ZB_ZCL_CMD_WRITE_ATTRIB_NO_RESP)
        {
            if (clusterID == ZB_ZCL_CLUSTER_ID_IDENTIFY && get_identify_time() != 0)
            {
                info("Identify process start, duration = %d\n", get_identify_time());

                if (!tmr_identify)
                {
                    tmr_identify = xTimerCreate("t_id", pdMS_TO_TICKS(200), pdFALSE, (void *)0, tmr_identify_cb);
                }
                if (!xTimerIsTimerActive(tmr_identify))
                {
                    xTimerStart(tmr_identify, 0);
                }
            }

        }
    } while (0);
}
static void _zcl_basic_process(uint16_t cmd, uint16_t datalen, uint8_t *pdata)
{
    if (cmd == ZB_ZCL_CMD_BASIC_RESET_ID)
    {
        taskENTER_CRITICAL();
        reset_attr();
        taskEXIT_CRITICAL();
        bsp_led_Off(BSP_LED_0);
    }
}
static void _zcl_indentify_process(uint16_t cmd, uint16_t datalen, uint8_t *pdata, uint32_t dir)
{
    if (cmd == ZB_ZCL_CMD_IDENTIFY_IDENTIFY_ID && dir == ZCL_FRAME_CLIENT_SERVER_DIR)
    {
        info("Identify process start, duration = %d\n", pdata[0] | (pdata[1] << 8));

        if (!tmr_identify)
        {
            tmr_identify = xTimerCreate("t_id", pdMS_TO_TICKS(200), pdFALSE, (void *)0, tmr_identify_cb);
        }
        if (!xTimerIsTimerActive(tmr_identify))
        {
            xTimerStart(tmr_identify, 0);
        }
    }
}

void zigbee_zcl_msg_handler(sys_tlv_t *pt_tlv)
{
    zigbee_zcl_data_idc_t *pt_zcl_msg = (zigbee_zcl_data_idc_t *)pt_tlv->value;
    do
    {
        if (!pt_zcl_msg)
        {
            break;
        }
        //info("Recv ZCL message from 0x%04X\n", pt_zcl_msg->srcAddr);
        //info("Cluster %04x, cmd %d\n", pt_zcl_msg->clusterID, pt_zcl_msg->cmd);
        //util_log_mem(UTIL_LOG_INFO, "  ", (uint8_t *)pt_zcl_msg->cmdFormat, pt_zcl_msg->cmdFormatLen, 0);
        if (pt_zcl_msg->is_common_command == 1)
        {
            _zcl_common_command_process(pt_zcl_msg->cmd, pt_zcl_msg->cmdFormatLen, pt_zcl_msg->cmdFormat, pt_zcl_msg->clusterID);
        }
        else if (pt_zcl_msg->is_common_command == 0)
        {
            switch (pt_zcl_msg->clusterID)
            {
            case ZB_ZCL_CLUSTER_ID_BASIC:
                _zcl_basic_process(pt_zcl_msg->cmd, pt_zcl_msg->cmdFormatLen, pt_zcl_msg->cmdFormat);
                break;
            case ZB_ZCL_CLUSTER_ID_IDENTIFY:
                _zcl_indentify_process(pt_zcl_msg->cmd, pt_zcl_msg->cmdFormatLen, pt_zcl_msg->cmdFormat, pt_zcl_msg->direction);
                break;
            default:
                break;
            }
        }

    } while (0);
}
