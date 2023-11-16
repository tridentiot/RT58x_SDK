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
#include "ble_app.h"
#include "ble_fota.h"


//=============================================================================
//                Private Function Declaration
//=============================================================================
uint16_t short_addr;

extern uint8_t long_addr[8];
static void _zdo_evt_start(sys_tlv_t *pt_tlv);
static void _zdo_evt_device_leave_idc(sys_tlv_t *pt_tlv);
static void _mac_address_get_idc(sys_tlv_t *pt_tlv);
static void _is_factory_new_idc(sys_tlv_t *pt_tlv);

static void(*zdo_evt_idc_func_list[])(sys_tlv_t *) =
{
    [ZIGBEE_EVT_TYPE_ZDO_START_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _zdo_evt_start,
    [ZIGBEE_EVT_TYPE_DEVICE_ANNCE_IDC - ZIGBEE_EVT_TYPE_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_LEAVE_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _zdo_evt_device_leave_idc,
    [ZIGBEE_EVT_TYPE_DEVICE_ASSOCIATED_IDC - ZIGBEE_EVT_TYPE_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_PANID_CONFLICT_IDC - ZIGBEE_EVT_TYPE_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_MAC_ADDRESS_GET_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _mac_address_get_idc,
    [ZIGBEE_EVT_TYPE_IS_FACTORY_NEW_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _is_factory_new_idc,
};

static void _zdo_evt_start(sys_tlv_t *pt_tlv)
{
    zigbee_nwk_start_idc_t *pt_start_idc = (zigbee_nwk_start_idc_t *)pt_tlv->value;
    if (pt_start_idc->status != 0)
    {
        if (ble_fota_state_get() == OTA_STATE_IDLE)
        {
            info_color(LOG_RED, "Device do join\n");
            zigbee_app_evt_change(APP_NOT_JOINED_EVT, false);
        }
        else
        {
            info_color(LOG_YELLOW, "Device suspend join\n");
        }
    }
    else
    {
        info_color(LOG_GREEN, "Device join success\n");
        info_color(LOG_GREEN, "PAN: %04X, ShortAddr: %04X, on channel: %02d, MAC: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
                   pt_start_idc->panID, pt_start_idc->nwkAddr, pt_start_idc->channel,
                   pt_start_idc->ieee_addr[7], pt_start_idc->ieee_addr[6],
                   pt_start_idc->ieee_addr[5], pt_start_idc->ieee_addr[4],
                   pt_start_idc->ieee_addr[3], pt_start_idc->ieee_addr[2],
                   pt_start_idc->ieee_addr[1], pt_start_idc->ieee_addr[0]);

        //memcpy(long_addr, pt_start_idc->ieee_addr, 8);
        short_addr = pt_start_idc->nwkAddr;
        zigbee_app_evt_change(APP_ZB_JOINED_EVT, false);
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

        if (memcmp(pt_dev_leave->ieeeAddr, long_addr, 8) == 0 && pt_dev_leave->rejoin == 0)
        {
            memset(&device_table_db, 0, sizeof(device_table_db));
            device_db_update();
            info("clear device DB\n");
            if (ble_fota_state_get() == OTA_STATE_IDLE)
            {
                Sys_Software_Reset();
            }
            else
            {
                info("BLE FOTA started\n");
            }
        }

    } while (0);
}

static void _mac_address_get_idc(sys_tlv_t *pt_tlv)
{
    extern void memcpy_inv(uint8_t *pDst, uint8_t *pSrc, uint16_t len);
    zigbee_mac_address_get_idc_t *pt_addr_idc = (zigbee_mac_address_get_idc_t *)pt_tlv->value;
    uint8_t new_device_name[16], idx, i, mac_addr[8];
    uint8_t hex_to_str[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                             };

    info_color(LOG_YELLOW, "mac addr %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", pt_addr_idc->mac_address[0], pt_addr_idc->mac_address[1],
               pt_addr_idc->mac_address[2], pt_addr_idc->mac_address[3],
               pt_addr_idc->mac_address[4], pt_addr_idc->mac_address[5],
               pt_addr_idc->mac_address[6], pt_addr_idc->mac_address[7]);

    memcpy_inv(mac_addr, pt_addr_idc->mac_address, sizeof(mac_addr));
    for (i = 0; i < 8 ; i++)
    {
        idx = ((mac_addr[i] & 0xF0) >> 4);
        new_device_name[(i << 1)] = hex_to_str[idx];
        idx = (mac_addr[i] & 0x0F);
        new_device_name[(i << 1) + 1] = hex_to_str[idx];
    }
    ble_device_rename(new_device_name);

}

static void _is_factory_new_idc(sys_tlv_t *pt_tlv)
{
    zigbee_is_factory_new_idc_t *pt_factory_new_idc = (zigbee_is_factory_new_idc_t *)pt_tlv->value;
    if (!pt_factory_new_idc->factory_new)
    {
        gu32_timer_100ms_cnt = 0;
    }
    // start adv
    app_request_set(APP_TRSP_P_HOST_ID, APP_REQUEST_ADV_START, false);
}

void zigbee_evt_handler(sys_tlv_t *pt_tlv)
{
    if ((pt_tlv->type >= ZIGBEE_EVT_TYPE_ZDO_START_IDC) &&
            (pt_tlv->type <= ZIGBEE_EVT_TYPE_ZDO_FINISH_IDC))
    {
        if (zdo_evt_idc_func_list[pt_tlv->type - ZIGBEE_EVT_TYPE_START_IDC])
        {
            zdo_evt_idc_func_list[pt_tlv->type - ZIGBEE_EVT_TYPE_START_IDC](pt_tlv);
        }
    }
}
