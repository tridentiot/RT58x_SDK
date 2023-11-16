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
#include "zigbee_api.h"
#include "zigbee_app.h"

//=============================================================================
//                Private Definitions of const value
//=============================================================================
static void _get_panid(raf_cmd_cfm_t *pt_cmd_cfm);
//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================
static void(*cli_cmd_cfm_func_list[])(raf_cmd_cfm_t *) =
{
    [GET_PANID] = _get_panid,
};
//=============================================================================
//                Private Global Variables
//=============================================================================

//=============================================================================
//                Functions
//=============================================================================

static void _get_panid(raf_cmd_cfm_t *pt_cmd_cfm)
{
    if (pt_cmd_cfm->Status == 0)
    {
        info_color(LOG_CYAN, "PANID : 0x%04X\n", *pt_cmd_cfm->cmd_value);
    }
    else
    {
        info_color(LOG_CYAN, "Get PANID Fail %d\n", pt_cmd_cfm->Status);
    }
}

void cli_raf_cmd_cfm_handler(sys_tlv_t *pt_tlv)
{
    raf_cmd_cfm_t *pt_cmd_cfm = (raf_cmd_cfm_t *)pt_tlv->value;
    if (cli_cmd_cfm_func_list[pt_cmd_cfm->cmd_type])
    {
        cli_cmd_cfm_func_list[pt_cmd_cfm->cmd_type](pt_cmd_cfm);
    }
}