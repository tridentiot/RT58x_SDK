/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_data.c
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
#include "sys_arch.h"
#include "zigbee_stack_api.h"


//=============================================================================
//                Private Global Variables
//=============================================================================
/* Basic cluster attributes */
static const uint8_t attr_zcl_version  = ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
static const uint8_t attr_app_version  = 0;
static const uint8_t attr_stack_version  = 0;
static const uint8_t attr_hw_version  = 0;
static const char attr_mf_name[]  = { 11, 'R', 'a', 'f', 'a', 'e', 'l', 'M', 'i', 'c', 'r', 'o'};
static const char attr_model_id[]  = { 9, 'T', 's', 't', 'a', 't', '0', '0', '0', '1'};
static const char attr_date_code[]  = { 8, '2', '0', '2', '2', '0', '1', '2', '1'};
static const char attr_sw_build_id[]  = { 8, '2', '0', '2', '2', '0', '1', '2', '1'};
static const uint8_t attr_power_source = ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE;

static uint8_t attr_location_id[] = { 16, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
static uint8_t attr_ph_env = 0;

static uint8_t attr_device_class = ZB_ZCL_BASIC_GENERIC_DEVICE_CLASS_DEFAULT_VALUE;
static uint8_t attr_device_type = ZB_ZCL_BASIC_GENERIC_DEVICE_TYPE_DEFAULT_VALUE;
static const char attr_product_code[] = ZB_ZCL_BASIC_PRODUCT_CODE_DEFAULT_VALUE;
static const char attr_product_url[] = ZB_ZCL_BASIC_PRODUCT_URL_DEFAULT_VALUE;

/*! Define a default global trust center link key */
uint8_t ZB_STANDARD_TC_KEY[] = {0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 };
uint8_t install_code_key[18] = {0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23, 0xA5, 0xC6, 0x39, 0xB2, 0x69, 0x16, 0xD5, 0x05, 0xC3, 0xB5};
//=============================================================================
//                Global Variables
//=============================================================================

/* Identify cluster attributes */
uint16_t g_attr_identify_time = 0;

/* Group cluster attributes data */
uint8_t g_attr_name_support = 0;

/* On/Off cluster attributes data */
uint32_t g_attr_on_off_on_off = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;
uint32_t g_attr_global_scene_ctrl = ZB_ZCL_ON_OFF_GLOBAL_SCENE_CONTROL_DEFAULT_VALUE;
uint16_t g_attr_on_off_on_time = ZB_ZCL_ON_OFF_ON_TIME_DEFAULT_VALUE;
uint16_t g_attr_on_off_off_wait_time = ZB_ZCL_ON_OFF_OFF_WAIT_TIME_DEFAULT_VALUE;
uint32_t g_attr_on_off_startup_onoff = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;

/* Metering cluster attributes data */
zb_uint48_t g_attr_curr_summ_delivered;
uint8_t g_attr_status = 0;
uint8_t g_attr_unit_of_measure = 0;
uint8_t g_attr_summation_formatting = 0;
uint8_t g_attr_curr_metering_device_type = 0;

/* Electrical measurement cluster attributes data */
uint16_t g_attr_measurement_type = 0x00000000;
uint16_t g_attr_dcpower = 0x8000;
uint16_t g_attr_rmsvoltage = 0xffff;
uint16_t g_attr_rmscurrent = 0xffff;
uint16_t g_attr_activepower = 0xffff;


zb_zcl_reporting_info_t rep_ctx[8];
//=============================================================================
//                Attribute definitions
//=============================================================================
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(basic_attr_list,
                                     &attr_zcl_version,
                                     &attr_app_version,
                                     &attr_stack_version,
                                     &attr_hw_version,
                                     &attr_mf_name,
                                     &attr_model_id,
                                     &attr_date_code,
                                     &attr_power_source,
                                     &attr_location_id,
                                     &attr_ph_env,
                                     &attr_sw_build_id,
                                     &attr_device_class,
                                     &attr_device_type,
                                     &attr_product_code,
                                     &attr_product_url);


ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list,
                                    &g_attr_identify_time);

ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST(groups_attr_list,
                                  &g_attr_name_support);

ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST_EXT(on_off_attr_list,
                                      &g_attr_on_off_on_off,
                                      &g_attr_global_scene_ctrl,
                                      &g_attr_on_off_on_time,
                                      &g_attr_on_off_off_wait_time,
                                      &g_attr_on_off_startup_onoff);

ZB_ZCL_DECLARE_METERING_ATTRIB_LIST(metering_attr_list,
                                    &g_attr_curr_summ_delivered,
                                    &g_attr_status,
                                    &g_attr_unit_of_measure,
                                    &g_attr_summation_formatting,
                                    &g_attr_curr_metering_device_type);

ZB_ZCL_DECLARE_ELECTRICAL_MEASUREMENT_ATTRIB_LIST(el_meas_attr_list,
        &g_attr_measurement_type,
        &g_attr_dcpower,
        &g_attr_rmsvoltage,
        &g_attr_rmscurrent,
        &g_attr_activepower);


zb_zcl_cluster_desc_t g_zigbee_cluster_list[] =
{
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_BASIC,
                        ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),
                        (basic_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_IDENTIFY,
                        ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t),
                        (identify_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_GROUPS,
                        ZB_ZCL_ARRAY_SIZE(groups_attr_list, zb_zcl_attr_t),
                        (groups_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_ARRAY_SIZE(on_off_attr_list, zb_zcl_attr_t),
                        (on_off_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_METERING,
                        ZB_ZCL_ARRAY_SIZE(metering_attr_list, zb_zcl_attr_t),
                        (metering_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT,
                        ZB_ZCL_ARRAY_SIZE(el_meas_attr_list, zb_zcl_attr_t),
                        (el_meas_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
};
//=============================================================================
//                Simple desc definitions
//=============================================================================
ZB_DECLARE_SIMPLE_DESC(6, 1);

ZB_AF_SIMPLE_DESC_TYPE(6, 1) simple_desc_meter =
{
    2,                                              /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                            /* Application profile identifier */
    HA_SMART_PLUG_DEVICE_ID,                /* Application device identifier */
    1,                                              /* Application device version */
    0,                                              /* Reserved */
    6,                                              /* Application input cluster count */
    1,                                              /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_CLUSTER_ID_METERING,
        ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
    }
};

ZB_AF_DECLARE_ENDPOINT_DESC(s_meter_ep,
                            2,
                            ZB_AF_HA_PROFILE_ID,
                            0,
                            NULL,
                            ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
                            g_zigbee_cluster_list,
                            (zb_af_simple_desc_1_1_t *)&simple_desc_meter,
                            8,
                            rep_ctx,
                            0,
                            NULL);

zb_af_endpoint_desc_t *s_meter_ep_list[] =
{
    &s_meter_ep,
};

zb_af_device_ctx_t simple_desc_meter_ctx =
{
    1,
    s_meter_ep_list
};
uint32_t get_identify_time(void)
{
    return g_attr_identify_time;
}

void reset_attr(void)
{
    /* Identify cluster attributes */
    g_attr_identify_time = 0;

    /* Group cluster attributes data */
    g_attr_name_support = 0;

    /* On/Off cluster attributes data */
    g_attr_on_off_on_off = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;
    g_attr_global_scene_ctrl = ZB_ZCL_ON_OFF_GLOBAL_SCENE_CONTROL_DEFAULT_VALUE;
    g_attr_on_off_on_time = ZB_ZCL_ON_OFF_ON_TIME_DEFAULT_VALUE;
    g_attr_on_off_off_wait_time = ZB_ZCL_ON_OFF_OFF_WAIT_TIME_DEFAULT_VALUE;
    g_attr_on_off_startup_onoff = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;

    /* Metering cluster attributes data */
    memset(&g_attr_curr_summ_delivered, 0, sizeof(zb_uint48_t));
    g_attr_status = 0;
    g_attr_unit_of_measure = 0;
    g_attr_summation_formatting = 0;
    g_attr_curr_metering_device_type = 0;

    /* Electrical measurement cluster attributes data */
    g_attr_measurement_type = 0x00000000;
    g_attr_dcpower = 0x8000;
    g_attr_rmsvoltage = 0xffff;
    g_attr_rmscurrent = 0xffff;
    g_attr_activepower = 0xffff;
}
