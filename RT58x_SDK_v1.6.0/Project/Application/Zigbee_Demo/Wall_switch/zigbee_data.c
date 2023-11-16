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
#include "zigbee_app.h"

//=============================================================================
//                Private Global Variables
//=============================================================================
/* Basic cluster attributes */
static const uint8_t attr_zcl_version  = ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
static const uint8_t attr_app_version  = 0;
static const uint8_t attr_stack_version  = 0;
static const uint8_t attr_hw_version  = 0;
static const char attr_mf_name[]  = { 11, 'R', 'a', 'f', 'a', 'e', 'l', 'M', 'i', 'c', 'r', 'o'};
static const char attr_model_id[]  = { 10, 'S', 'w', 'i', 't', 'c', 'h', '0', '0', '0', '1'};
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

/* Identify cluster attributes data */
uint16_t g_attr_identify_time = 0;

/* Group cluster attributes data */
uint8_t g_attr_name_support = 0;

/* Scenes cluster attribute data */
uint8_t g_attr_scenes_scene_count = ZB_ZCL_SCENES_SCENE_COUNT_DEFAULT_VALUE;      /* Number of scenes currently in the device's scene table */
uint8_t g_attr_scenes_current_scene = ZB_ZCL_SCENES_CURRENT_SCENE_DEFAULT_VALUE;    /* Scene ID of the scene last invoked */
uint8_t g_attr_scenes_scene_valid = ZB_ZCL_SCENES_SCENE_VALID_DEFAULT_VALUE;      /* Indicates whether the state of the device corresponds to
                                          * that associated with the CurrentScene and CurrentGroup attributes*/
uint8_t g_attr_scenes_name_support = 0;   /* Indicates support for scene names */
uint16_t g_attr_scenes_current_group = ZB_ZCL_SCENES_CURRENT_GROUP_DEFAULT_VALUE;   /* Group ID of the scene last invoked */

/* On/Off cluster attributes data */
uint32_t g_attr_on_off_on_off = ZB_ZCL_ON_OFF_IS_OFF;

/* Level cluster attributes data */
uint8_t g_attr_level_current_level = ZB_ZCL_LEVEL_CONTROL_LEVEL_MIN_VALUE;
uint16_t g_attr_level_remaining_time = 0;

zb_zcl_cvc_alarm_variables_t lev_ctrl_ctx[1];
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

ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST(scenes_attr_list,
                                  &g_attr_scenes_scene_count,
                                  &g_attr_scenes_current_scene,
                                  &g_attr_scenes_current_group,
                                  &g_attr_scenes_scene_valid,
                                  &g_attr_scenes_name_support);

ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST(on_off_attr_list,
                                  &g_attr_on_off_on_off);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(level_control_attr_list,
        &g_attr_level_current_level,
        &g_attr_level_remaining_time);

zb_zcl_cluster_desc_t g_zigbee_cluster_list[] =
{
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),
        (basic_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t),
        (identify_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_ARRAY_SIZE(groups_attr_list, zb_zcl_attr_t),
        (groups_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_ARRAY_SIZE(scenes_attr_list, zb_zcl_attr_t),
        (scenes_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_ARRAY_SIZE(on_off_attr_list, zb_zcl_attr_t),
        (on_off_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
        ZB_ZCL_ARRAY_SIZE(level_control_attr_list, zb_zcl_attr_t),
        (level_control_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        0,
        NULL,
        ZB_ZCL_CLUSTER_CLIENT_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
        0,
        NULL,
        ZB_ZCL_CLUSTER_CLIENT_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        0,
        NULL,
        ZB_ZCL_CLUSTER_CLIENT_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),

};
//=============================================================================
//                Simple desc definitions
//=============================================================================
//ZB_DECLARE_SIMPLE_DESC(1, 3);
ZB_DECLARE_SIMPLE_DESC(6, 3);

ZB_AF_SIMPLE_DESC_TYPE(6, 3) simple_desc_switch_ep_2 =
{
    WALL_SWITCH_EP,                                  /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                           /* Application profile identifier */
    HA_DIMMER_PLUG_IN_UNIT_DEVICE_ID,              /* Application device identifier */
    1,                                              /* Application device version */
    0,                                  /* Reserved */
    6,                  /* Application input cluster count */
    3,                 /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
    }
};

ZB_AF_DECLARE_ENDPOINT_DESC(
    switch_ep_2,
    WALL_SWITCH_EP,
    ZB_AF_HA_PROFILE_ID,
    0,
    NULL,
    ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
    g_zigbee_cluster_list,
    (zb_af_simple_desc_1_1_t *)&simple_desc_switch_ep_2,
    8,
    rep_ctx,
    1,
    lev_ctrl_ctx);

zb_af_endpoint_desc_t *ep_list_switch[] =
{
    &switch_ep_2,
};

zb_af_device_ctx_t simple_desc_switch_ctx =
{
    1,
    ep_list_switch
};


uint32_t get_current_level(void)
{
    return g_attr_level_current_level;
}

void set_current_level(uint32_t level)
{
    g_attr_level_current_level = level;
}

uint32_t get_on_off_status(void)
{
    return g_attr_on_off_on_off;
}

void set_on_off_status(uint32_t status)
{
    g_attr_on_off_on_off = status;
}

uint32_t get_identify_time(void)
{
    return g_attr_identify_time;
}

void reset_attr(void)
{
    g_attr_identify_time = 0;
    g_attr_name_support = 0;
    g_attr_scenes_scene_count = ZB_ZCL_SCENES_SCENE_COUNT_DEFAULT_VALUE;
    g_attr_scenes_current_scene = ZB_ZCL_SCENES_CURRENT_SCENE_DEFAULT_VALUE;
    g_attr_scenes_scene_valid = ZB_ZCL_SCENES_SCENE_VALID_DEFAULT_VALUE;
    g_attr_scenes_name_support = 0;
    g_attr_scenes_current_group = ZB_ZCL_SCENES_CURRENT_GROUP_DEFAULT_VALUE;
    g_attr_on_off_on_off = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;
    g_attr_level_current_level = 0;
    g_attr_level_remaining_time = 0;
}

void set_scene_count(uint8_t scene_count)
{
    g_attr_scenes_scene_count = scene_count;
}
