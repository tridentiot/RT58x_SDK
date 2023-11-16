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

/* Scenes cluster attribute data */

uint8_t g_attr_scenes_scene_count = ZB_ZCL_SCENES_SCENE_COUNT_DEFAULT_VALUE;      /* Number of scenes currently in the device's scene table */
uint8_t g_attr_scenes_current_scene = ZB_ZCL_SCENES_CURRENT_SCENE_DEFAULT_VALUE;    /* Scene ID of the scene last invoked */
uint8_t g_attr_scenes_scene_valid = ZB_ZCL_SCENES_SCENE_VALID_DEFAULT_VALUE;      /* Indicates whether the state of the device corresponds to
                                          * that associated with the CurrentScene and CurrentGroup attributes*/
uint8_t g_attr_scenes_name_support = 0;   /* Indicates support for scene names */
uint16_t g_attr_scenes_current_group = ZB_ZCL_SCENES_CURRENT_GROUP_DEFAULT_VALUE;   /* Group ID of the scene last invoked */


/* Thermostat cluster attributes */
int16_t g_attr_local_temperature = 0xff;
int16_t g_attr_abs_min_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
int16_t g_attr_abs_max_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
int16_t g_attr_abs_min_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
int16_t g_attr_abs_max_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
uint8_t g_attr_PI_cooling_demand;
uint8_t g_attr_PI_heating_demand;
uint8_t g_attr_HVAC_system_type_configuration = ZB_ZCL_THERMOSTAT_HVAC_SYSTEM_TYPE_CONFIGURATION_DEFAULT_VALUE;
int8_t g_attr_local_temperature_calibration = ZB_ZCL_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION_DEFAULT_VALUE;
int16_t g_attr_occupied_cooling_setpoint = ZB_ZCL_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE;
int16_t g_attr_occupied_heating_setpoint = ZB_ZCL_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_DEFAULT_VALUE;
int16_t g_attr_unoccupied_cooling_setpoint = ZB_ZCL_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE;
int16_t g_attr_unoccupied_heating_setpoint = ZB_ZCL_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT_DEFAULT_VALUE;
int16_t g_attr_min_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
int16_t g_attr_max_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
int16_t g_attr_min_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
int16_t g_attr_max_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
int8_t g_attr_min_setpoint_dead_band = ZB_ZCL_THERMOSTAT_MIN_SETPOINT_DEAD_BAND_DEFAULT_VALUE;
uint8_t g_attr_remote_sensing = ZB_ZCL_THERMOSTAT_REMOTE_SENSING_DEFAULT_VALUE;
uint8_t g_attr_control_seq_of_operation = ZB_ZCL_THERMOSTAT_CONTROL_SEQ_OF_OPERATION_DEFAULT_VALUE;
uint8_t g_attr_system_mode = ZB_ZCL_THERMOSTAT_SYSTEM_MODE_DEFAULT_VALUE;
uint8_t g_attr_start_of_week = ZB_ZCL_THERMOSTAT_START_OF_WEEK_DEFAULT_VALUE;


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

ZB_ZCL_DECLARE_THERMOSTAT_ATTRIB_LIST_EXT(thermostat_attr_list,
        &g_attr_local_temperature,
        &g_attr_abs_min_heat_setpoint_limit,
        &g_attr_abs_max_heat_setpoint_limit,
        &g_attr_abs_min_cool_setpoint_limit,
        &g_attr_abs_max_cool_setpoint_limit,
        &g_attr_PI_cooling_demand,
        &g_attr_PI_heating_demand,
        &g_attr_HVAC_system_type_configuration,
        &g_attr_local_temperature_calibration,
        &g_attr_occupied_cooling_setpoint,
        &g_attr_occupied_heating_setpoint,
        &g_attr_unoccupied_cooling_setpoint,
        &g_attr_unoccupied_heating_setpoint,
        &g_attr_min_heat_setpoint_limit,
        &g_attr_max_heat_setpoint_limit,
        &g_attr_min_cool_setpoint_limit,
        &g_attr_max_cool_setpoint_limit,
        &g_attr_min_setpoint_dead_band,
        &g_attr_remote_sensing,
        &g_attr_control_seq_of_operation,
        &g_attr_system_mode,
        &g_attr_start_of_week);


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
    ZB_ZCL_CLUSTER_DESC(ZB_ZCL_CLUSTER_ID_THERMOSTAT,
                        ZB_ZCL_ARRAY_SIZE(thermostat_attr_list, zb_zcl_attr_t),
                        (thermostat_attr_list),
                        ZB_ZCL_CLUSTER_SERVER_ROLE,
                        ZB_ZCL_MANUF_CODE_INVALID
                       ),
};
//=============================================================================
//                Simple desc definitions
//=============================================================================
ZB_DECLARE_SIMPLE_DESC(7, 0);

ZB_AF_SIMPLE_DESC_TYPE(7, 0) simple_desc_tstat =
{
    2,                                              /* Endpoint */
    ZB_AF_HA_PROFILE_ID,                            /* Application profile identifier */
    HA_THERMOSTAT_DEVICE_ID,                                /* Application device identifier */
    1,                                              /* Application device version */
    0,                                              /* Reserved */
    5,                                              /* Application input cluster count */
    0,                                              /* Application output cluster count */
    /* Application input and output cluster list */
    {
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_CLUSTER_ID_THERMOSTAT,
    }
};

ZB_AF_DECLARE_ENDPOINT_DESC(tstat_ep,
                            2,
                            ZB_AF_HA_PROFILE_ID,
                            0,
                            NULL,
                            ZB_ZCL_ARRAY_SIZE(g_zigbee_cluster_list, zb_zcl_cluster_desc_t),
                            g_zigbee_cluster_list,
                            (zb_af_simple_desc_1_1_t *)&simple_desc_tstat,
                            8,
                            rep_ctx,
                            1,
                            lev_ctrl_ctx);

zb_af_endpoint_desc_t *tstat_ep_list[] =
{
    &tstat_ep,
};

zb_af_device_ctx_t simple_desc_tstat_ctx =
{
    1,
    tstat_ep_list
};
uint32_t get_identify_time(void)
{
    return g_attr_identify_time;
}
uint8_t get_system_mode (void)
{
    return g_attr_system_mode;
}
int16_t get_cooling_setpoint (void)
{
    return g_attr_occupied_cooling_setpoint;
}
int16_t get_heating_setpoint (void)
{
    return g_attr_occupied_heating_setpoint;
}
void set_system_mode(uint8_t mode)
{
    g_attr_system_mode = mode;
}

void set_cooling_setpoint(int16_t setpoint)
{
    g_attr_occupied_cooling_setpoint = setpoint;
}
void set_heating_setpoint(int16_t setpoint)
{
    g_attr_occupied_heating_setpoint = setpoint;
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
    g_attr_local_temperature = 0xff;
    g_attr_abs_min_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_abs_max_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_abs_min_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_abs_max_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_HVAC_system_type_configuration = ZB_ZCL_THERMOSTAT_HVAC_SYSTEM_TYPE_CONFIGURATION_DEFAULT_VALUE;
    g_attr_local_temperature_calibration = ZB_ZCL_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION_DEFAULT_VALUE;
    g_attr_occupied_cooling_setpoint = ZB_ZCL_THERMOSTAT_OCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE;
    g_attr_occupied_heating_setpoint = ZB_ZCL_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_DEFAULT_VALUE;
    g_attr_unoccupied_cooling_setpoint = ZB_ZCL_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT_DEFAULT_VALUE;
    g_attr_unoccupied_heating_setpoint = ZB_ZCL_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT_DEFAULT_VALUE;
    g_attr_min_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_max_heat_setpoint_limit = ZB_ZCL_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_min_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_max_cool_setpoint_limit = ZB_ZCL_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT_DEFAULT_VALUE;
    g_attr_min_setpoint_dead_band = ZB_ZCL_THERMOSTAT_MIN_SETPOINT_DEAD_BAND_DEFAULT_VALUE;
    g_attr_remote_sensing = ZB_ZCL_THERMOSTAT_REMOTE_SENSING_DEFAULT_VALUE;
    g_attr_control_seq_of_operation = ZB_ZCL_THERMOSTAT_CONTROL_SEQ_OF_OPERATION_DEFAULT_VALUE;
    g_attr_system_mode = ZB_ZCL_THERMOSTAT_SYSTEM_MODE_DEFAULT_VALUE;
    g_attr_start_of_week = ZB_ZCL_THERMOSTAT_START_OF_WEEK_DEFAULT_VALUE;
}

void set_scene_count(uint8_t scene_count)
{
    g_attr_scenes_scene_count = scene_count;
}
