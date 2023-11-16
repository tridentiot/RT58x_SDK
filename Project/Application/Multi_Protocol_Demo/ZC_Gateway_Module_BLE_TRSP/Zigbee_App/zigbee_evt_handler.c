/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_evt_handler.c
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

#include "zigbee_lib_api.h"
#include "zigbee_gateway.h"

app_db_t gt_app_db;

//=============================================================================
//                Private Function Declaration
//=============================================================================
static void _zdo_evt_device_announce(sys_tlv_t *pt_tlv);
static void _zdo_evt_start(sys_tlv_t *pt_tlv);
static void _zdo_evt_simple_desc_idc(sys_tlv_t *pt_tlv);
static void _zdo_evt_act_eo_idc(sys_tlv_t *pt_tlv);
static void _mac_ed_scan_idc(sys_tlv_t *pt_tlv);
static void _zdo_evt_device_leave_idc(sys_tlv_t *pt_tlv);
static void _permit_join_idc(sys_tlv_t *pt_tlv);
static void(*zdo_evt_idc_func_list[])(sys_tlv_t *) =
{
    [ZIGBEE_EVT_TYPE_START_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _zdo_evt_start,
    [ZIGBEE_EVT_TYPE_DEVICE_ANNCE_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _zdo_evt_device_announce,
    [ZIGBEE_EVT_TYPE_LEAVE_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _zdo_evt_device_leave_idc,
    [ZIGBEE_EVT_TYPE_DEVICE_ASSOCIATED_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_PANID_CONFLICT_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_ZDO_ACT_EP_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _zdo_evt_act_eo_idc,
    [ZIGBEE_EVT_TYPE_ZDO_SIMPLE_DESC_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _zdo_evt_simple_desc_idc,
    [ZIGBEE_EVT_TYPE_MAC_ED_SCAN_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _mac_ed_scan_idc,
    [ZIGBEE_EVT_TYPE_ZDO_PERMIT_JOIN_IDC - ZIGBEE_EVT_TYPE_ZDO_START_IDC] = _permit_join_idc,

};
static void _permit_join_idc(sys_tlv_t *pt_tlv)
{
    zigbee_zdo_device_permit_join_idc_t *pt_idc;
    do
    {
        if (!pt_tlv)
        {
            break;
        }
        pt_idc = (zigbee_zdo_device_permit_join_idc_t *)pt_tlv->value;
        info_color(LOG_YELLOW, "permit_join duration = %ds\n", pt_idc->permit_duration);
    } while (0);
}
static void _zdo_evt_act_eo_idc(sys_tlv_t *pt_tlv)
{
    int i;
    zigbee_zdo_act_ep_idc_t *pt_idc;
    do
    {
        if (!pt_tlv)
        {
            break;
        }

        pt_idc = (zigbee_zdo_act_ep_idc_t *)pt_tlv->value;

        info_color(LOG_CYAN, "Active Ep : Addr %02X, Endpoint ", pt_idc->nwkAddr);

        for (i = 0; i < pt_idc->epCounts; i++)
        {
            info_color(LOG_CYAN, "%02X", pt_idc->ep_list[i]);
        }
        info("\n");

    } while (0);
}
static void _zdo_evt_simple_desc_idc(sys_tlv_t *pt_tlv)
{
    zigbee_zdo_simple_desc_idc_t *pt_idc;
    do
    {
        if (!pt_tlv)
        {
            break;
        }

        pt_idc = (zigbee_zdo_simple_desc_idc_t *)pt_tlv->value;

        info_color(LOG_CYAN, "Simple desc : Addr %02X, Endpoint %02X, Profile %04X, DeviceID %04X\n",
                   pt_idc->nwkAddr, pt_idc->endpoint, pt_idc->profileID, pt_idc->deviceID);

        device_table_update_cmd_forward(pt_idc->nwkAddr, pt_idc->endpoint, pt_idc->deviceID);

    } while (0);
}

static void _zdo_evt_device_announce(sys_tlv_t *pt_tlv)
{
    zigbee_zdo_device_annce_idc_t *pt_dev_annce;
    do
    {
        if (!pt_tlv)
        {
            break;
        }
        pt_dev_annce = (zigbee_zdo_device_annce_idc_t *)pt_tlv->value;
        info_color(LOG_GREEN, "Device Announce :\n");
        info_color(LOG_GREEN, "\tIEEE %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
                   pt_dev_annce->ieeeAddr[7], pt_dev_annce->ieeeAddr[6],
                   pt_dev_annce->ieeeAddr[5], pt_dev_annce->ieeeAddr[4],
                   pt_dev_annce->ieeeAddr[3], pt_dev_annce->ieeeAddr[2],
                   pt_dev_annce->ieeeAddr[1], pt_dev_annce->ieeeAddr[0]);
        info_color(LOG_GREEN, "\tShort address 0x%04x, Cap %02X\n", pt_dev_annce->shortAddr, pt_dev_annce->capability);
        zigbee_gateway_cmd_send((GW_CMD_DEVICE_ANN_IND), 0x0000, 0, 0, (uint8_t *)pt_tlv->value, 11);

        device_table_create_cmd_forward(pt_dev_annce->ieeeAddr, pt_dev_annce->shortAddr);

    } while (0);
}

static void _zdo_evt_start(sys_tlv_t *pt_tlv)
{
    zigbee_nwk_start_idc_t *pt_start_idc = (zigbee_nwk_start_idc_t *)pt_tlv->value;
    if (pt_start_idc->status != 0)
    {
        info_color(LOG_RED, "Start fail\n");
        zigbee_app_evt_change(APP_INIT_EVT, false);
    }
    else
    {
        info_color(LOG_GREEN, "Device start success\n");
        info_color(LOG_GREEN, "PAN: %04X, ShortAddr: %04X, on channel: %02d, MAC: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
                   pt_start_idc->panID, pt_start_idc->nwkAddr, pt_start_idc->channel,
                   pt_start_idc->ieee_addr[7], pt_start_idc->ieee_addr[6],
                   pt_start_idc->ieee_addr[5], pt_start_idc->ieee_addr[4],
                   pt_start_idc->ieee_addr[3], pt_start_idc->ieee_addr[2],
                   pt_start_idc->ieee_addr[1], pt_start_idc->ieee_addr[0]);
        zigbee_app_evt_change(APP_ZB_START_EVT, false);
        zigbee_send_permit_join(0, 0);
        if ((g_pan_id != pt_start_idc->panID) ||
                (g_channel != pt_start_idc->channel))
        {
            zc_info_remove_cmd_forward();
            zc_info_create_cmd_forward(pt_start_idc->channel, pt_start_idc->panID, false);
        }

    }
}
static void _mac_ed_scan_idc(sys_tlv_t *pt_tlv)
{
    zigbee_mac_ed_scan_idc_t *pt_idc = (zigbee_mac_ed_scan_idc_t *)pt_tlv->value;
    int i;
    for (i = 0; i < sizeof(pt_idc->EDValue); i++)
    {
        info("channel %d value: %d\n", i + 11, pt_idc->EDValue[i]);
    }
}

static void _zdo_evt_device_leave_idc(sys_tlv_t *pt_tlv)
{

    zigbee_zdo_device_leave_idc_t *pt_dev_leave;
    do
    {
        if (!pt_tlv)
        {
            break;
        }
        pt_dev_leave = (zigbee_zdo_device_leave_idc_t *)pt_tlv->value;
        info_color(LOG_RED, "Device Leave :\n");
        info_color(LOG_RED, "\tIEEE %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
                   pt_dev_leave->ieeeAddr[7], pt_dev_leave->ieeeAddr[6],
                   pt_dev_leave->ieeeAddr[5], pt_dev_leave->ieeeAddr[4],
                   pt_dev_leave->ieeeAddr[3], pt_dev_leave->ieeeAddr[2],
                   pt_dev_leave->ieeeAddr[1], pt_dev_leave->ieeeAddr[0]);
        info_color(LOG_RED, "\tShort address 0x%04x, rejoin %02X\n", pt_dev_leave->shortAddr, pt_dev_leave->rejoin);

        group_table_delete_by_nwk_addr(pt_dev_leave->shortAddr);
        scene_table_delete_by_nwk_addr(pt_dev_leave->shortAddr);
        bind_table_delete_by_ieee_addr(pt_dev_leave->ieeeAddr);
        device_table_delete_cmd_forward(pt_dev_leave->ieeeAddr);
    } while (0);
}

void zigbee_evt_handler(sys_tlv_t *pt_tlv)
{
    if ((pt_tlv->type >= ZIGBEE_EVT_TYPE_ZDO_START_IDC) &&
            (pt_tlv->type <= ZIGBEE_EVT_TYPE_ZDO_FINISH_IDC))
    {
        if (zdo_evt_idc_func_list[pt_tlv->type - ZIGBEE_EVT_TYPE_ZDO_START_IDC])
        {
            zdo_evt_idc_func_list[pt_tlv->type - ZIGBEE_EVT_TYPE_ZDO_START_IDC](pt_tlv);
        }
    }
}



