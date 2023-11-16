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



//=============================================================================
//                Private Function Declaration
//=============================================================================
uint16_t short_addr;
extern uint8_t long_addr[8];
static void _zdo_evt_start(sys_tlv_t *pt_tlv);
static void _zdo_evt_device_leave_idc(sys_tlv_t *pt_tlv);
static void _zdo_evt_rejoin_start(sys_tlv_t *pt_tlv);
static void _zdo_evt_rejoin_fail(sys_tlv_t *pt_tlv);
static void(*zdo_evt_idc_func_list[])(sys_tlv_t *) =
{
    [ZIGBEE_EVT_TYPE_ZDO_START_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _zdo_evt_start,
    [ZIGBEE_EVT_TYPE_DEVICE_ANNCE_IDC - ZIGBEE_EVT_TYPE_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_LEAVE_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _zdo_evt_device_leave_idc,
    [ZIGBEE_EVT_TYPE_DEVICE_ASSOCIATED_IDC - ZIGBEE_EVT_TYPE_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_PANID_CONFLICT_IDC - ZIGBEE_EVT_TYPE_START_IDC] = NULL,
    [ZIGBEE_EVT_TYPE_DEVICE_START_REJOIN - ZIGBEE_EVT_TYPE_START_IDC] = _zdo_evt_rejoin_start,
    [ZIGBEE_EVT_TYPE_REJOIN_FAILURE_IDC - ZIGBEE_EVT_TYPE_START_IDC] = _zdo_evt_rejoin_fail,

};

uint8_t Led_old_status = 0;
uint8_t Led1_old_status = 0;
uint8_t Led2_old_status = 0;

static void _zdo_evt_start(sys_tlv_t *pt_tlv)
{
    zigbee_nwk_start_idc_t *pt_start_idc = (zigbee_nwk_start_idc_t *)pt_tlv->value;
    if (pt_start_idc->status != 0)
    {
        info_color(LOG_RED, "Device do join\n");
        zigbee_app_evt_change(APP_NOT_JOINED_EVT);
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

        short_addr = pt_start_idc->nwkAddr;
        memcpy(long_addr, pt_start_idc->ieee_addr, 8);
        zigbee_app_evt_change(APP_ZB_JOINED_EVT);
        info("Resume Light status:%d,%d,%d\n", Led_old_status, Led1_old_status, Led2_old_status);

        if (Led_old_status)
        {
            gpio_pin_write(5, 1);
            Delay_ms(5);
            gpio_pin_write(5, 0);
            set_on_off_status(BUTTON_1_EP, 0x01);
            Led_old_status = 0x00;
        }
        if (Led1_old_status)
        {
            gpio_pin_write(28, 1);
            Delay_ms(5);
            gpio_pin_write(28, 0);
            set_on_off_status(BUTTON_2_EP, 0x01);
            Led1_old_status = 0x00;
        }
        if (Led2_old_status)
        {
            gpio_pin_write(30, 1);
            Delay_ms(5);
            gpio_pin_write(30, 0);
            set_on_off_status(BUTTON_3_EP, 0x01);
            Led2_old_status = 0x00;
        }
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
            memset(&scene_table_db, 0, sizeof(scene_table_db));
            scene_db_update();
            info("clear scene DB\n");
            Sys_Software_Reset();
        }

    } while (0);
}

static void _zdo_evt_rejoin_start(sys_tlv_t *pt_tlv)
{
    info("Light status:%d,%d,%d\n", get_on_off_status(BUTTON_1_EP), get_on_off_status(BUTTON_2_EP), get_on_off_status(BUTTON_3_EP));
    if (get_on_off_status(BUTTON_1_EP))
    {
        gpio_pin_write(6, 1); //high
        Delay_ms(5);
        gpio_pin_write(6, 0); //low
        set_on_off_status(BUTTON_1_EP, 0x00);
        Led_old_status = 0x01;
    }
    if (get_on_off_status(BUTTON_2_EP))
    {
        gpio_pin_write(29, 1); //high
        Delay_ms(5);
        gpio_pin_write(29, 0); //low
        set_on_off_status(BUTTON_2_EP, 0x00);
        Led1_old_status = 0x01;
    }
    if (get_on_off_status(BUTTON_3_EP))
    {
        gpio_pin_write(31, 1); //high
        Delay_ms(5);
        gpio_pin_write(31, 0); //low
        set_on_off_status(BUTTON_3_EP, 0x00);
        Led2_old_status = 0x01;
    }
    zigbee_app_evt_change(APP_ZB_REJOIN_START_EVT);
}

static void _zdo_evt_rejoin_fail(sys_tlv_t *pt_tlv)
{
    /* alway rejoin, user can set rejoin timeout here */
    zigbee_app_evt_change(APP_NOT_JOINED_EVT);
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
