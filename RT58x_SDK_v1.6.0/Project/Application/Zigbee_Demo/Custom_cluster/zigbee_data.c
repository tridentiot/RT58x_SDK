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
/*! Define a default global trust center link key */
uint8_t ZB_STANDARD_TC_KEY[] = {0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 };
uint8_t install_code_key[18] = {0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23, 0xA5, 0xC6, 0x39, 0xB2, 0x69, 0x16, 0xD5, 0x05, 0xC3, 0xB5};
//=============================================================================
//                Global Variables
//=============================================================================
/* Custom cluster attributes */
zb_uint8_t custom_dst_endpoint = 0xFF;
zb_uint16_t custom_dst_short_addr = 0xFFFF;
zb_uint8_t g_attr_u8 = ZB_ZCL_CUSTOM_CLUSTER_ATTR_U8_DEFAULT_VALUE;
zb_int16_t g_attr_s16 = ZB_ZCL_CUSTOM_CLUSTER_ATTR_S16_DEFAULT_VALUE;
zb_uint24_t g_attr_24bit = ZB_ZCL_CUSTOM_CLUSTER_ATTR_24BIT_DEFAULT_VALUE;
zb_uint32_t g_attr_32bitmap = ZB_ZCL_CUSTOM_CLUSTER_ATTR_32BITMAP_DEFAULT_VALUE;
zb_ieee_addr_t g_attr_ieee = ZB_ZCL_CUSTOM_CLUSTER_ATTR_IEEE_DEFAULT_VALUE;
zb_char_t g_attr_char_string[ZB_ZCL_CUSTOM_CLUSTER_ATTR_CHAR_STRING_MAX_SIZE] =
    ZB_ZCL_CUSTOM_CLUSTER_ATTR_CHAR_STRING_DEFAULT_VALUE;
zb_time_t g_attr_utc_time = ZB_ZCL_CUSTOM_CLUSTER_ATTR_UTC_TIME_DEFAULT_VALUE;
zb_uint8_t g_attr_byte_array[ZB_ZCL_CUSTOM_CLUSTER_ATTR_BYTE_ARRAY_MAX_SIZE] =
    ZB_ZCL_CUSTOM_CLUSTER_ATTR_BYTE_ARRAY_DEFAULT_VALUE;
zb_bool_t g_attr_bool = ZB_ZCL_CUSTOM_CLUSTER_ATTR_BOOL_DEFAULT_VALUE;
zb_uint8_t g_attr_128_bit_key[16] = ZB_ZCL_CUSTOM_CLUSTER_ATTR_128_BIT_KEY_DEFAULT_VALUE;
//=============================================================================
//                Attribute definitions
//=============================================================================

ZB_ZCL_DECLARE_CUSTOM_ATTR_CLUSTER_ATTRIB_LIST(custom_attr_list,
        &g_attr_u8,
        &g_attr_s16,
        &g_attr_24bit,
        &g_attr_32bitmap,
        g_attr_ieee,
        g_attr_char_string,
        &g_attr_utc_time,
        g_attr_byte_array,
        &g_attr_bool,
        g_attr_128_bit_key);

zb_zcl_cluster_desc_t g_zigbee_cluster_list[] =
{
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_CUSTOM,
        ZB_ZCL_ARRAY_SIZE(custom_attr_list, zb_zcl_attr_t),
        (custom_attr_list),
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    ),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_CUSTOM,
        ZB_ZCL_ARRAY_SIZE(custom_attr_list, zb_zcl_attr_t),
        (custom_attr_list),
        ZB_ZCL_CLUSTER_CLIENT_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID
    )
};
//=============================================================================
//                Simple desc definitions
//=============================================================================
//ZB_DECLARE_SIMPLE_DESC(1, 1);

ZB_AF_SIMPLE_DESC_TYPE(1, 1) simple_desc_cc =
{
    2,                                              /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                            /* Application profile identifier */
    HA_TEST_DEVICE_ID,              /* Application device identifier */
    1,                                              /* Application device version */
    0,                                              /* Reserved */
    1,                                              /* Application input cluster count */
    1,                                              /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_CUSTOM,
        ZB_ZCL_CLUSTER_ID_CUSTOM
    }
};

ZB_AF_DECLARE_ENDPOINT_DESC(cc_ep,
                            2,
                            ZB_AF_HA_PROFILE_ID,
                            0,
                            NULL,
                            ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
                            g_zigbee_cluster_list,
                            (zb_af_simple_desc_1_1_t *)&simple_desc_cc,
                            0,
                            NULL,
                            1,
                            NULL);

zb_af_endpoint_desc_t *cc_ep_list[] =
{
    &cc_ep,
};

zb_af_device_ctx_t simple_desc_cc_ctx =
{
    1,
    cc_ep_list
};
