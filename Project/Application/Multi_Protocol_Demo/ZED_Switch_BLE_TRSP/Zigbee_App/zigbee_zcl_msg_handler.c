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

//=============================================================================
//                Private Function Declaration
//=============================================================================

void zigbee_zcl_msg_handler(sys_tlv_t *pt_tlv)
{
    zigbee_zcl_data_idc_t *pt_zcl_msg = (zigbee_zcl_data_idc_t *)pt_tlv->value;
    do
    {
        if (!pt_zcl_msg)
        {
            break;
        }
        info("Recv ZCL message\n");
        info("Cluster %04x, cmd %d\n", pt_zcl_msg->clusterID, pt_zcl_msg->cmd);
        util_log_mem(UTIL_LOG_INFO, "  ", (uint8_t *)pt_zcl_msg->cmdFormat, pt_zcl_msg->cmdFormatLen, 0);
    } while (0);
}
