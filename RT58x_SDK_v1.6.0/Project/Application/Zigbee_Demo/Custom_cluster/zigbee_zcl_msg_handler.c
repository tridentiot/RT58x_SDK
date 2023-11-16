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

/* ZigBee Stack Library APIs */
#include "zigbee_stack_api.h"
#include "zigbee_app.h"

#include "bsp_led.h"

//=============================================================================
//                Private Function Declaration
//=============================================================================

void zigbee_zcl_msg_handler(sys_tlv_t *pt_tlv)
{
    zigbee_zcl_data_idc_t *pt_zcl_msg = (zigbee_zcl_data_idc_t *)pt_tlv->value;
    uint8_t i = 0;

    do
    {
        if (!pt_zcl_msg)
        {
            break;
        }
        //info("Recv ZCL message 0x%04X\n", pt_zcl_msg->dstAddr);
        //info("Cluster %04x, cmd %d\n", pt_zcl_msg->clusterID, pt_zcl_msg->cmd);
        //util_log_mem(UTIL_LOG_INFO, "  ", (uint8_t *)pt_zcl_msg->cmdFormat, pt_zcl_msg->cmdFormatLen, 0);
        if (pt_zcl_msg->is_common_command == 0)
        {
            switch (pt_zcl_msg->clusterID)
            {
            case ZB_ZCL_CLUSTER_ID_CUSTOM:
                if (pt_zcl_msg->cmd == UART_TRANSPARENT_CMD)
                {
                    info_color(LOG_YELLOW, "[%04X] ", pt_zcl_msg->srcAddr);
                    for (i = 0; i < pt_zcl_msg->cmdFormatLen; i++)
                    {
                        info_color(LOG_YELLOW, "%c", pt_zcl_msg->cmdFormat[i]);
                    }
                    info_color(LOG_YELLOW, "\n");
                }

                break;
            default:
                break;
            }
        }

    } while (0);
}
