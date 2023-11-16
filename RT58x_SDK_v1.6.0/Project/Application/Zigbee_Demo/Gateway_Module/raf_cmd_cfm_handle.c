/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_app.c
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

#include "bsp_uart.h"

//=============================================================================
//                Private Definitions of const value
//=============================================================================
static void _raf_cmd_common_gen_gw_rsp(raf_cmd_cfm_t *pt_cmd_cfm);
//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================

//=============================================================================
//                Functions
//=============================================================================
static void _raf_cmd_common_gen_gw_rsp(raf_cmd_cfm_t *pt_cmd_cfm)
{
    do
    {
        if (pt_cmd_cfm == NULL)
        {
            break;
        }

        zigbee_gateway_cmd_send((pt_cmd_cfm->cmd_index | 0x8000), pt_cmd_cfm->cmd_dst_addr, 0, 0,
                                (uint8_t *)pt_cmd_cfm->cmd_value, pt_cmd_cfm->cmd_length);
    } while (0);
}

void raf_cmd_cfm_handler(sys_tlv_t *pt_tlv)
{
    raf_cmd_cfm_t *pt_cmd_cfm = (raf_cmd_cfm_t *)pt_tlv->value;

    if (pt_cmd_cfm)
    {
        _raf_cmd_common_gen_gw_rsp(pt_cmd_cfm);
    }

}
