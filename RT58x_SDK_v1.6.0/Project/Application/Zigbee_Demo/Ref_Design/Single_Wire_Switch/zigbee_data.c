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
#include "zigbee_lib_api.h"
#include "zigbee_app.h"

//=============================================================================
//                Private Global Variables
//=============================================================================
/* Basic cluster attributes */
static const uint8_t attr_zcl_basic_version  = (zb_uint8_t)ZB_ZCL_VERSION;
static const uint8_t attr_zcl_basic_app_version  = 0;
static const uint8_t attr_zcl_basic_stack_version  = 0x02;
static const uint8_t attr_zcl_basic_hw_version  = 0;

static const char attr_zcl_basic_mf_name[]  = { 15, '_', 'T', 'Z', '3', '0', '0', '0', '_', 't', 'k', 'o', 's', 't', '5', 'f'};
static const char attr_zcl_basic_model_id[]  = { 6, 'T', 'S', '0', '0', '0', '1'};
static const char attr_zcl_basic_date_code[]  = { 8, '2', '0', '2', '2', '0', '1', '2', '1'};
static const char attr_zcl_basic_sw_build_id[]  = { 9, '1', '2', '2', '0', '5', '2', '0', '1', '7'};
static const uint8_t attr_zcl_basic_power_source = 0x01;

static uint8_t attr_zcl_basic_location_id[] = { 16, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
static uint8_t attr_zcl_basic_ph_env = 0;
static const uint16_t attr_basic_cluster_revision  = 0x0001;



/*! Define a default global trust center link key */
uint8_t ZB_STANDARD_TC_KEY[] = {0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 };
uint8_t install_code_key[18] = {0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23, 0xA5, 0xC6, 0x39, 0xB2, 0x69, 0x16, 0xD5, 0x05, 0xC3, 0xB5};
//=============================================================================
//                Global Variables
//=============================================================================
/* Identify cluster attributes data */
uint16_t g_attr_identify_time = 0x0000;
int g_attr_identify_cluster_revision = 0x0001;


/* Group cluster attributes data */
uint8_t g_attr_name_support = 0;
uint16_t g_attr_group_cluster_revision = 0x0001;


/* Scenes cluster attribute data */
uint8_t g_attr_scenes_scene_count = 0x00;      /* Number of scenes currently in the device's scene table */
uint8_t g_attr_scenes_current_scene = 0x00;    /* Scene ID of the scene last invoked */
uint8_t g_attr_scenes_scene_valid = 0x00;      /* Indicates whether the state of the device corresponds to
                                          * that associated with the CurrentScene and CurrentGroup attributes*/
uint8_t g_attr_scenes_name_support = 0x00;     /* Indicates support for scene names */
uint16_t g_attr_scenes_current_group = 0x0000;   /* Group ID of the scene last invoked */
uint16_t g_attr_scenes_cluster_revision = 0x0001;


/* On/Off cluster attributes data */
uint8_t g_attr_on_off_on_off = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;
bool g_attr_global_scene_ctrl = FALSE;
uint16_t g_attr_on_time = 0x00;
uint16_t g_attr_off_wait_time = 0x00;
uint16_t g_attr_on_off_cluster_revision = 0x0001;

zb_zcl_cvc_alarm_variables_t lev_ctrl_ctx;
zb_zcl_reporting_info_t rep_ctx[8];
//=============================================================================
//                Attribute definitions
//=============================================================================
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT_WC(basic_attr_list,
                                        &attr_zcl_basic_version,
                                        &attr_zcl_basic_app_version,
                                        &attr_zcl_basic_stack_version,
                                        &attr_zcl_basic_hw_version,
                                        &attr_zcl_basic_mf_name,
                                        &attr_zcl_basic_model_id,
                                        &attr_zcl_basic_date_code,
                                        &attr_zcl_basic_power_source,
                                        &attr_zcl_basic_sw_build_id,
                                        &attr_zcl_basic_location_id,
                                        &attr_zcl_basic_ph_env,
                                        &attr_basic_cluster_revision);

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST_TJ(identify_attr_list,
                                       &g_attr_identify_time,
                                       &g_attr_identify_cluster_revision);


ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST_TJ(groups_attr_list,
                                     &g_attr_name_support,
                                     &g_attr_group_cluster_revision);


ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST_TJ(scenes_attr_list,
                                     &g_attr_scenes_scene_count,
                                     &g_attr_scenes_current_scene,
                                     &g_attr_scenes_current_group,
                                     &g_attr_scenes_scene_valid,
                                     &g_attr_scenes_name_support,
                                     &g_attr_scenes_cluster_revision);


ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST_EXT_TJ(on_off_attr_list,
        &g_attr_on_off_on_off,
        &g_attr_global_scene_ctrl,
        &g_attr_on_time,
        &g_attr_off_wait_time,
        &g_attr_on_off_cluster_revision);


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
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_SCENES,
                        ZB_ZCL_ARRAY_SIZE(scenes_attr_list, zb_zcl_attr_t),
                        (scenes_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_ARRAY_SIZE(on_off_attr_list, zb_zcl_attr_t),
                        (on_off_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       )
};
//=============================================================================
//                Simple desc definitions
//=============================================================================
ZB_DECLARE_SIMPLE_DESC(7, 1);

ZB_AF_SIMPLE_DESC_TYPE(7, 1) simple_desc_switch_ep_1 =
{
    BUTTON_1_EP,                                    /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                            /* Application profile identifier */
    HA_ON_OFF_LIGHT_DEVICE_ID,                      /* Application device identifier */
    0,                                              /* Application device version */
    0,                                              /* Reserved */
    7,                                              /* Application input cluster count */
    1,                                              /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
        ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
        ZB_ZCL_CLUSTER_ID_OTA_UPGRADE
    }
};
ZB_AF_DECLARE_ENDPOINT_DESC(
    switch_ep_1,
    BUTTON_1_EP,
    ZB_AF_HA_PROFILE_ID,
    0,
    NULL,
    ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
    g_zigbee_cluster_list,
    (zb_af_simple_desc_1_1_t *)&simple_desc_switch_ep_1,
    0,
    rep_ctx,
    1,
    &lev_ctrl_ctx);

/* Basic cluster attributes */
static const uint8_t attr_zcl_basic_version_1  = (zb_uint8_t)ZB_ZCL_VERSION;
static const uint8_t attr_zcl_basic_app_version_1  = 0;
static const uint8_t attr_zcl_basic_stack_version_1  = 0x02;
static const uint8_t attr_zcl_basic_hw_version_1  = 0;

static const char attr_zcl_basic_mf_name_1[]  = { 15, '_', 'T', 'Z', '3', '0', '0', '0', '_', 't', 'k', 'o', 's', 't', '5', 'f'};
static const char attr_zcl_basic_model_id_1[]  = { 6, 'T', 'S', '0', '0', '0', '1'};
static const char attr_zcl_basic_date_code_1[]  = { 8, '2', '0', '2', '2', '0', '1', '2', '1'};
static const char attr_zcl_basic_sw_build_id_1[]  = { 9, '1', '2', '2', '0', '5', '2', '0', '1', '7'};
static const uint8_t attr_zcl_basic_power_source_1 = 0x01;

static uint8_t attr_zcl_basic_location_id_1[] = { 16, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
static uint8_t attr_zcl_basic_ph_env_1 = 0;
static const uint16_t attr_basic_cluster_revision_1  = 0x0001;

/* Identify cluster attributes data */

uint16_t g_attr_identify_time_1 = 0x0000;
int g_attr_identify_cluster_revision_1 = 0x0001;

/* Group cluster attributes data */
uint8_t g_attr_name_support_1 = 0;
uint16_t g_attr_group_cluster_revision_1 = 0x0001;

/* Scenes cluster attribute data */
uint8_t g_attr_scenes_scene_count_1 = 0x00;      /* Number of scenes currently in the device's scene table */
uint8_t g_attr_scenes_current_scene_1 = 0x00;    /* Scene ID of the scene last invoked */
uint8_t g_attr_scenes_scene_valid_1 = 0x00;      /* Indicates whether the state of the device corresponds to
                                          * that associated with the CurrentScene and CurrentGroup attributes*/
uint8_t g_attr_scenes_name_support_1 = 0x00;     /* Indicates support for scene names */
uint16_t g_attr_scenes_current_group_1 = 0x0000;   /* Group ID of the scene last invoked */
uint16_t g_attr_scenes_cluster_revision_1 = 0x0001;


/* On/Off cluster attributes data */
uint8_t g_attr_on_off_on_off_1 = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;
bool g_attr_global_scene_ctrl_1 = FALSE;
uint16_t g_attr_on_time_1 = 0x00;
uint16_t g_attr_off_wait_time_1 = 0x00;
uint16_t g_attr_on_off_cluster_revision_1 = 0x0001;


zb_zcl_cvc_alarm_variables_t lev_ctrl_ctx1;
zb_zcl_reporting_info_t rep_ctx1[8];
//=============================================================================
//                Attribute definitions
//=============================================================================
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT_WC(basic_attr_list_1,
                                        &attr_zcl_basic_version_1,
                                        &attr_zcl_basic_app_version_1,
                                        &attr_zcl_basic_stack_version_1,
                                        &attr_zcl_basic_hw_version_1,
                                        &attr_zcl_basic_mf_name_1,
                                        &attr_zcl_basic_model_id_1,
                                        &attr_zcl_basic_date_code_1,
                                        &attr_zcl_basic_power_source_1,
                                        &attr_zcl_basic_sw_build_id_1,
                                        &attr_zcl_basic_location_id_1,
                                        &attr_zcl_basic_ph_env_1,
                                        &attr_basic_cluster_revision_1);

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST_TJ(identify_attr_list_1,
                                       &g_attr_identify_time_1,
                                       &g_attr_identify_cluster_revision_1);

ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST_TJ(groups_attr_list_1,
                                     &g_attr_name_support_1,
                                     &g_attr_group_cluster_revision_1);

ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST_TJ(scenes_attr_list_1,
                                     &g_attr_scenes_scene_count_1,
                                     &g_attr_scenes_current_scene_1,
                                     &g_attr_scenes_current_group_1,
                                     &g_attr_scenes_scene_valid_1,
                                     &g_attr_scenes_name_support_1,
                                     &g_attr_scenes_cluster_revision_1);

ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST_EXT_TJ(on_off_attr_list_1,
        &g_attr_on_off_on_off_1,
        &g_attr_global_scene_ctrl_1,
        &g_attr_on_time_1,
        &g_attr_off_wait_time_1,
        &g_attr_on_off_cluster_revision_1);

zb_zcl_cluster_desc_t g_zigbee_cluster_list_1[] =
{
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_BASIC,
                        ZB_ZCL_ARRAY_SIZE(basic_attr_list_1, zb_zcl_attr_t),
                        (basic_attr_list_1),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_IDENTIFY,
                        ZB_ZCL_ARRAY_SIZE(identify_attr_list_1, zb_zcl_attr_t),
                        (identify_attr_list_1),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_GROUPS,
                        ZB_ZCL_ARRAY_SIZE(groups_attr_list_1, zb_zcl_attr_t),
                        (groups_attr_list_1),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_SCENES,
                        ZB_ZCL_ARRAY_SIZE(scenes_attr_list_1, zb_zcl_attr_t),
                        (scenes_attr_list_1),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_ARRAY_SIZE(on_off_attr_list_1, zb_zcl_attr_t),
                        (on_off_attr_list_1),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       )
};
//=============================================================================
//                Simple desc definitions
//=============================================================================

ZB_DECLARE_SIMPLE_DESC(5, 0);
ZB_AF_SIMPLE_DESC_TYPE(5, 0) simple_desc_switch_ep_2 =
{
    BUTTON_2_EP,                                    /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                            /* Application profile identifier */
    HA_ON_OFF_LIGHT_DEVICE_ID,                      /* Application device identifier */
    0,                                              /* Application device version */
    0,                                              /* Reserved */
    7,                                              /* Application input cluster count */
    0,                                              /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
    }
};
ZB_AF_DECLARE_ENDPOINT_DESC(
    switch_ep_2,
    BUTTON_2_EP,
    ZB_AF_HA_PROFILE_ID,
    0,
    NULL,
    ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list_1, zb_zcl_cluster_desc_t),
    g_zigbee_cluster_list_1,
    (zb_af_simple_desc_1_1_t *)&simple_desc_switch_ep_2,
    0,
    rep_ctx1,
    1,
    &lev_ctrl_ctx1);


/* Basic cluster attributes */
static const uint8_t attr_zcl_basic_version_2  = (zb_uint8_t)ZB_ZCL_VERSION;
static const uint8_t attr_zcl_basic_app_version_2  = 0;
static const uint8_t attr_zcl_basic_stack_version_2  = 0x02;
static const uint8_t attr_zcl_basic_hw_version_2  = 0;

static const char attr_zcl_basic_mf_name_2[]  = { 15, '_', 'T', 'Z', '3', '0', '0', '0', '_', 't', 'k', 'o', 's', 't', '5', 'f'};
static const char attr_zcl_basic_model_id_2[]  = { 6, 'T', 'S', '0', '0', '0', '1'};
static const char attr_zcl_basic_date_code_2[]  = { 8, '2', '0', '2', '2', '0', '1', '2', '1'};
static const char attr_zcl_basic_sw_build_id_2[]  = { 9, '1', '2', '2', '0', '5', '2', '0', '1', '7'};
static const uint8_t attr_zcl_basic_power_source_2 = 0x01;

static uint8_t attr_zcl_basic_location_id_2[] = { 16, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
static uint8_t attr_zcl_basic_ph_env_2 = 0;
static const uint16_t attr_basic_cluster_revision_2  = 0x0001;

/* Identify cluster attributes data */

uint16_t g_attr_identify_time_2 = 0x0000;
int g_attr_identify_cluster_revision_2 = 0x0001;

/* Group cluster attributes data */
uint8_t g_attr_name_support_2 = 0;
uint16_t g_attr_group_cluster_revision_2 = 0x0001;

/* Scenes cluster attribute data */
uint8_t g_attr_scenes_scene_count_2 = 0x00;      /* Number of scenes currently in the device's scene table */
uint8_t g_attr_scenes_current_scene_2 = 0x00;    /* Scene ID of the scene last invoked */
uint8_t g_attr_scenes_scene_valid_2 = 0x00;      /* Indicates whether the state of the device corresponds to
                                          * that associated with the CurrentScene and CurrentGroup attributes*/
uint8_t g_attr_scenes_name_support_2 = 0x00;     /* Indicates support for scene names */
uint16_t g_attr_scenes_current_group_2 = 0x0000;   /* Group ID of the scene last invoked */
uint16_t g_attr_scenes_cluster_revision_2 = 0x0001;


/* On/Off cluster attributes data */
uint8_t g_attr_on_off_on_off_2 = ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE;
bool g_attr_global_scene_ctrl_2 = FALSE;
uint16_t g_attr_on_time_2 = 0x00;
uint16_t g_attr_off_wait_time_2 = 0x00;
uint16_t g_attr_on_off_cluster_revision_2 = 0x0001;


zb_zcl_cvc_alarm_variables_t lev_ctrl_ctx2;
zb_zcl_reporting_info_t rep_ctx2[8];
//=============================================================================
//                Attribute definitions
//=============================================================================
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT_WC(basic_attr_list_2,
                                        &attr_zcl_basic_version_2,
                                        &attr_zcl_basic_app_version_2,
                                        &attr_zcl_basic_stack_version_2,
                                        &attr_zcl_basic_hw_version_2,
                                        &attr_zcl_basic_mf_name_2,
                                        &attr_zcl_basic_model_id_2,
                                        &attr_zcl_basic_date_code_2,
                                        &attr_zcl_basic_power_source_2,
                                        &attr_zcl_basic_sw_build_id_2,
                                        &attr_zcl_basic_location_id_2,
                                        &attr_zcl_basic_ph_env_2,
                                        &attr_basic_cluster_revision_2);

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST_TJ(identify_attr_list_2,
                                       &g_attr_identify_time_2,
                                       &g_attr_identify_cluster_revision_2);

ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST_TJ(groups_attr_list_2,
                                     &g_attr_name_support_2,
                                     &g_attr_group_cluster_revision_2);

ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST_TJ(scenes_attr_list_2,
                                     &g_attr_scenes_scene_count_2,
                                     &g_attr_scenes_current_scene_2,
                                     &g_attr_scenes_current_group_2,
                                     &g_attr_scenes_scene_valid_2,
                                     &g_attr_scenes_name_support_2,
                                     &g_attr_scenes_cluster_revision_2);

ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST_EXT_TJ(on_off_attr_list_2,
        &g_attr_on_off_on_off_2,
        &g_attr_global_scene_ctrl_2,
        &g_attr_on_time_2,
        &g_attr_off_wait_time_2,
        &g_attr_on_off_cluster_revision_2);

zb_zcl_cluster_desc_t g_zigbee_cluster_list_2[] =
{
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_BASIC,
                        ZB_ZCL_ARRAY_SIZE(basic_attr_list_2, zb_zcl_attr_t),
                        (basic_attr_list_2),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_IDENTIFY,
                        ZB_ZCL_ARRAY_SIZE(identify_attr_list_2, zb_zcl_attr_t),
                        (identify_attr_list_2),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_GROUPS,
                        ZB_ZCL_ARRAY_SIZE(groups_attr_list_2, zb_zcl_attr_t),
                        (groups_attr_list_2),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_SCENES,
                        ZB_ZCL_ARRAY_SIZE(scenes_attr_list_2, zb_zcl_attr_t),
                        (scenes_attr_list_2),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_ARRAY_SIZE(on_off_attr_list_2, zb_zcl_attr_t),
                        (on_off_attr_list_2),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       )
};
//=============================================================================
//                Simple desc definitions
//=============================================================================

ZB_AF_SIMPLE_DESC_TYPE(5, 0) simple_desc_switch_ep_3 =
{
    BUTTON_3_EP,                                    /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                            /* Application profile identifier */
    HA_ON_OFF_LIGHT_DEVICE_ID,                      /* Application device identifier */
    0,                                              /* Application device version */
    0,                                              /* Reserved */
    7,                                              /* Application input cluster count */
    0,                                              /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
    }
};

ZB_AF_DECLARE_ENDPOINT_DESC(
    switch_ep_3,
    BUTTON_3_EP,
    ZB_AF_HA_PROFILE_ID,
    0,
    NULL,
    ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list_2, zb_zcl_cluster_desc_t),
    g_zigbee_cluster_list_2,
    (zb_af_simple_desc_1_1_t *)&simple_desc_switch_ep_3,
    0,
    rep_ctx2,
    1,
    &lev_ctrl_ctx2);


zb_af_endpoint_desc_t *ep_list_switch[] =
{
    &switch_ep_1,
    &switch_ep_2,
    &switch_ep_3,
};

zb_af_device_ctx_t simple_desc_switch_ctx =
{
    3,
    ep_list_switch
};

uint8_t get_on_off_status(uint8_t ep)
{
    uint8_t on_off_status = 0;
    switch (ep)
    {
    case BUTTON_1_EP:
        on_off_status = g_attr_on_off_on_off;
        break;
    case BUTTON_2_EP:
        on_off_status = g_attr_on_off_on_off_1;
        break;
    case BUTTON_3_EP:
        on_off_status = g_attr_on_off_on_off_2;
        break;
    default:
        break;
    }
    return on_off_status;
}

void set_on_off_status(uint8_t ep, uint8_t status)
{
    uint8_t len = 1;
    zigbee_zcl_attr_set_t *pt_attr_set_req;
    ZIGBEE_ZCL_ATTR_SET(pt_attr_set_req, ep, ZB_ZCL_CLUSTER_ID_ON_OFF, ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, 0, len);
    if (pt_attr_set_req)
    {
        pt_attr_set_req->value[0] = status;
        zigbee_zcl_attr_set(pt_attr_set_req, len);
        sys_free(pt_attr_set_req);
    }
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
}
void set_scene_count(uint8_t scene_count)
{
    g_attr_scenes_scene_count = scene_count;
}
