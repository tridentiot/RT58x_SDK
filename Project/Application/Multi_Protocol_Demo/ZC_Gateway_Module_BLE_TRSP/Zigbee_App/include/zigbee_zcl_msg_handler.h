// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __ZIGBEE_ZCL_MSG_HANDLER_H__
#define __ZIGBEE_ZCL_MSG_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zigbee_stack_api.h"

//=============================================================================
//                Public ENUM
//=============================================================================
typedef enum
{
    GW_CMD_APP_SRV_DEV_GET_VER_INFO = 0,
    GW_CMD_APP_SRV_DEV_GET_MANUFACTURE_NAME,
    GW_CMD_APP_SRV_DEV_GET_MODEL_ID,
    GW_CMD_APP_SRV_DEV_GET_DATE_CODE,
    GW_CMD_APP_SRV_DEV_GET_SOFTWARE_ID,
} e_dev_info;

typedef enum
{
    GW_CMD_APP_SRV_DEV_READ_ATTRIBUTES = 0,
} e_dev_attribute;

typedef enum
{
    GW_CMD_APP_SRV_IDENTIFY = 0,
    GW_CMD_APP_SRV_IDENTIFY_QUERY,
} e_dev_identify;

typedef enum
{
    GW_CMD_APP_SRV_GROUP_ADD = 0,
    GW_CMD_APP_SRV_GROUP_VIEW,
    GW_CMD_APP_SRV_GROUP_GET_MEMBERSHIP,
    GW_CMD_APP_SRV_GROUP_REMOVE,
    GW_CMD_APP_SRV_GROUP_REMOVE_ALL,
    GW_CMD_APP_SRV_GROUP_ADD_IF_IDENTIFYING,
} e_group_mgmt;

typedef enum
{
    GW_CMD_APP_SRV_SCENE_ADD = 0,
    GW_CMD_APP_SRV_SCENE_VIEW,
    GW_CMD_APP_SRV_SCENE_REMOVE,
    GW_CMD_APP_SRV_SCENE_REMOVE_ALL,
    GW_CMD_APP_SRV_SCENE_STORE,
    GW_CMD_APP_SRV_SCENE_RECALL,
    GW_CMD_APP_SRV_SCENE_GET_EMBERSHIP,
} e_scene_mgmt;

typedef enum
{
    GW_CMD_APP_SRV_ONOFF_ON = 0,
    GW_CMD_APP_SRV_ONOFF_OFF,
    GW_CMD_APP_SRV_ONOFF_TOGGLE,
    GW_CMD_APP_SRV_ONOFF_ON_WITH_RECALL_GLOBAL_SCENE,
    GW_CMD_APP_SRV_ONOFF_OFF_WITH_TIMED_OFF,
} e_on_off_ctrl;

typedef enum
{
    GW_CMD_APP_SRV_LEVEL_MOVE_TO_LEVEL = 0,
    GW_CMD_APP_SRV_LEVEL_MOVE,
    GW_CMD_APP_SRV_LEVEL_STEP,
    GW_CMD_APP_SRV_LEVEL_STOP,
    GW_CMD_APP_SRV_LEVEL_MOVE_TO_LEVEL_WITH_ONOFF,
    GW_CMD_APP_SRV_LEVEL_MOVE_WITH_ONOFF,
    GW_CMD_APP_SRV_LEVEL_STEP_WITH_ONOFF,
} e_level_ctrl;

typedef enum
{
    GW_CMD_APP_SRV_COLOR_MOVE_TO_HUE = 0,
    GW_CMD_APP_SRV_COLOR_MOVE_HUE,
    GW_CMD_APP_SRV_COLOR_STEP_HUE,
    GW_CMD_APP_SRV_COLOR_MOVE_TO_SATURATION,
    GW_CMD_APP_SRV_COLOR_MOVE_SATURATION,
    GW_CMD_APP_SRV_COLOR_STEP_SATURATION,
    GW_CMD_APP_SRV_COLOR_MOVE_TO_HUE_AND_SATURATION,
    GW_CMD_APP_SRV_COLOR_MOVE_TO_COLOR,
    GW_CMD_APP_SRV_COLOR_MOVE_COLOR,
    GW_CMD_APP_SRV_COLOR_STEP_COLOR,
    GW_CMD_APP_SRV_COLOR_MOVE_TO_COLOR_TEMPERATURE  = 0x0000000a,
    GW_CMD_APP_SRV_COLOR_MOVE_COLOR_TEMPERATURE     = 0x0000004b,
    GW_CMD_APP_SRV_COLOR_STEP_COLOR_TEMPERATURE     = 0x0000004c,
} e_color_ctrl;

//=============================================================================
//                Public Function Declaration
//=============================================================================

void zigbee_zcl_msg_handler(sys_tlv_t *pt_tlv);

#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_ZCL_MSG_HANDLER_H__ */
