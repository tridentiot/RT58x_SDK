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
static uint8_t attr_zcl_version  = ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
static uint8_t attr_power_source = ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE;

uint8_t install_code_key[18] = {0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23, 0xA5, 0xC6, 0x39, 0xB2, 0x69, 0x16, 0xD5, 0x05, 0xC3, 0xB5};
//=============================================================================
//                Global Variables
//=============================================================================

//=============================================================================
//                Attribute definitions
//=============================================================================
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST(
    basic_attr_list,
    &attr_zcl_version,
    &attr_power_source);

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
        ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
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
ZB_DECLARE_SIMPLE_DESC(0, 1);

ZB_AF_SIMPLE_DESC_TYPE(1, 1) simple_desc_switch_ep_1 =
{
    BUTTON_1_EP,                                  /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                /* Application profile identifier */
    HA_COLOR_DIMMER_SWITCH_DEVICE_ID,                       /* Application device identifier */
    0,                      /* Application device version */
    0,                                  /* Reserved */
    1,                  /* Application input cluster count */
    1,                 /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_ON_OFF,
    }
};

ZB_AF_SIMPLE_DESC_TYPE(0, 1) simple_desc_switch_ep_2 =
{
    BUTTON_2_EP,                                  /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                           /* Application profile identifier */
    HA_DIMMER_SWITCH_DEVICE_ID,              /* Application device identifier */
    0,                                              /* Application device version */
    0,                                  /* Reserved */
    0,                  /* Application input cluster count */
    1,                 /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
    }
};

ZB_AF_SIMPLE_DESC_TYPE(0, 1) simple_desc_switch_ep_3 =
{
    BUTTON_3_EP,                                  /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                           /* Application profile identifier */
    HA_COLOR_DIMMER_SWITCH_DEVICE_ID,              /* Application device identifier */
    0,                                              /* Application device version */
    0,                                  /* Reserved */
    0,                  /* Application input cluster count */
    1,                 /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_COLOR_CONTROL
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
    NULL,
    0,
    NULL);
ZB_AF_DECLARE_ENDPOINT_DESC(
    switch_ep_2,
    BUTTON_2_EP,
    ZB_AF_HA_PROFILE_ID,
    0,
    NULL,
    ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
    g_zigbee_cluster_list,
    (zb_af_simple_desc_1_1_t *)&simple_desc_switch_ep_2,
    0,
    NULL,
    0,
    NULL);

ZB_AF_DECLARE_ENDPOINT_DESC(
    switch_ep_3,
    BUTTON_3_EP,
    ZB_AF_HA_PROFILE_ID,
    0,
    NULL,
    ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
    g_zigbee_cluster_list,
    (zb_af_simple_desc_1_1_t *)&simple_desc_switch_ep_3,
    0,
    NULL,
    0,
    NULL);


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

