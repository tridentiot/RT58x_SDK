/**
 * @file zigbee_gateway_cmd_app_service.c
 * @author Rex Huang (rex.huang@rafaelmicro.com)
 * @brief
 * @version 0.1
 * @date 2022-05-03
 *
 * @copyright Copyright (c) 2022
 *
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

#include "zigbee_evt_handler.h"
#include "zigbee_zcl_msg_handler.h"
#include "zigbee_lib_api.h"

/* BSP APIs */
#include "bsp.h"
#include "bsp_console.h"
#include "bsp_led.h"
#include "bsp_button.h"

#include "zigbee_gateway.h"

//=============================================================================
//                Private Definitions of const value
//=============================================================================
typedef struct __attribute__((packed))
{
    uint16_t attr_id;
    uint8_t status;
    uint8_t data_type;
    uint8_t data[];
}
read_attr_status_record_t;

typedef struct __attribute__((packed))
{
    uint16_t cluster_id;
    read_attr_status_record_t att_status;
}
read_cluster_attr_rsp_t;


typedef void (*gw_cmd_app_func)(uint32_t, uint8_t *);

#define GW_CMD_APP_CMD_OFFSET                       0x10000
#define GW_CMD_APP_SRV_DEV_BASE                     0x10000

#define GW_CMD_APP_SRV_ATTRIBUTES_BASE                     0x20000
#define GW_CMD_APP_SRV_IDENTIFY_BASE                0x40000

#define GW_CMD_APP_SRV_GROUP_MGMT_BASE              0x50000

#define GW_CMD_APP_SRV_SCENE_MGMT_BASE              0x60000

#define GW_CMD_APP_SRV_ONOFF_CTRL_BASE              0x70000

#define GW_CMD_APP_SRV_LEVEL_CTRL_BASE              0x90000

#define GW_CMD_APP_SRV_COLOR_CTRL_BASE              0x210000

//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================

//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================

//=============================================================================
//                Functions
//=============================================================================
static void _zcl_basic_read_rsp_cb(uint16_t cluster_id, uint16_t addr, uint8_t src_endp, uint8_t *pd, uint8_t pd_len)
{
    read_attr_status_record_t *pt_record;
    uint8_t offset = 0, i;
    uint8_t *p_rsp_pd = NULL, rsp_pd_len = 0;
    uint32_t cmd_id;

    do
    {
        if (cluster_id != ZB_ZCL_CLUSTER_ID_BASIC)
        {
            break;
        }
        if (pd == NULL)
        {
            break;
        }
        pt_record = (read_attr_status_record_t *)(pd);

        switch (pt_record->attr_id)
        {
        case ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID:
            p_rsp_pd = sys_malloc(4);
            if (p_rsp_pd == NULL)
            {
                break;
            }
            for (i = 0; i < 4; i++)
            {
                pt_record = (read_attr_status_record_t *)(pd + offset);
                offset += 5;
                p_rsp_pd[i] = pt_record->data[0];
            }
            rsp_pd_len = 4;
            cmd_id = GW_CMD_APP_SRV_DEV_GET_VER_INFO;
            break;

        case ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID:

            rsp_pd_len = pt_record->data[0] + 1;
            p_rsp_pd = sys_malloc(rsp_pd_len);
            if (p_rsp_pd == NULL)
            {
                break;
            }
            memcpy(p_rsp_pd, pt_record->data, rsp_pd_len);
            cmd_id = GW_CMD_APP_SRV_DEV_GET_MANUFACTURE_NAME;
            break;

        case ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID:
            rsp_pd_len = pt_record->data[0] + 1;
            p_rsp_pd = sys_malloc(rsp_pd_len);
            if (p_rsp_pd == NULL)
            {
                break;
            }
            memcpy(p_rsp_pd, pt_record->data, rsp_pd_len);
            cmd_id = GW_CMD_APP_SRV_DEV_GET_MODEL_ID;
            break;

        case ZB_ZCL_ATTR_BASIC_DATE_CODE_ID:
            rsp_pd_len = pt_record->data[0] + 1;
            p_rsp_pd = sys_malloc(rsp_pd_len);
            if (p_rsp_pd == NULL)
            {
                break;
            }
            memcpy(p_rsp_pd, pt_record->data, rsp_pd_len);
            cmd_id = GW_CMD_APP_SRV_DEV_GET_DATE_CODE;
            break;

        case ZB_ZCL_ATTR_BASIC_SW_BUILD_ID:
            rsp_pd_len = pt_record->data[0] + 1;
            p_rsp_pd = sys_malloc(rsp_pd_len);
            if (p_rsp_pd == NULL)
            {
                break;
            }
            memcpy(p_rsp_pd, pt_record->data, rsp_pd_len);
            cmd_id = GW_CMD_APP_SRV_DEV_GET_SOFTWARE_ID;
            break;

        default:
            break;
        }
    } while (0);

    if (p_rsp_pd)
    {
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(cmd_id | GW_CMD_APP_SRV_DEV_BASE),
                                addr, 0, src_endp, p_rsp_pd, rsp_pd_len);
        sys_free(p_rsp_pd);
    }
}

static void _zcl_read_attribute_cb(uint16_t cluster_id, uint16_t addr, uint8_t src_endp, uint8_t *pd, uint8_t pd_len)
{
    read_cluster_attr_rsp_t *pt_att_rsp;
    uint8_t rsp_pd_len = 0;
    do
    {

        if (pd == NULL)
        {
            break;
        }

        rsp_pd_len = pd_len + sizeof(cluster_id);
        pt_att_rsp = sys_malloc(rsp_pd_len);
        pt_att_rsp->cluster_id = cluster_id;
        memcpy(&pt_att_rsp->att_status, pd, pd_len);
    } while (0);

    if (pt_att_rsp)
    {
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_APP_SRV_ATTRIBUTES_BASE),
                                addr, 0, src_endp, (uint8_t *)pt_att_rsp, rsp_pd_len);
        sys_free(pt_att_rsp);
    }
    else
    {
        info_color(LOG_RED, "read attribute rsp malloc fail\n");
    }
}

static void _cmd_dev_info_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_DEV_GET_VER_INFO:
        attr_data_len = 8;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }

        p_attr_data[0] = ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID & 0xFF;
        p_attr_data[1] = (ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID >> 8) & 0xFF;

        p_attr_data[2] = ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID & 0xFF;
        p_attr_data[3] = (ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID >> 8) & 0xFF;

        p_attr_data[4] = ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID & 0xFF;
        p_attr_data[5] = (ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID >> 8) & 0xFF;

        p_attr_data[6] = ZB_ZCL_ATTR_BASIC_HW_VERSION_ID & 0xFF;
        p_attr_data[7] = (ZB_ZCL_ATTR_BASIC_HW_VERSION_ID >> 8) & 0xFF;
        break;

    case GW_CMD_APP_SRV_DEV_GET_MANUFACTURE_NAME:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        p_attr_data[0] = ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID & 0xFF;
        p_attr_data[1] = (ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID >> 8) & 0xFF;

        break;

    case GW_CMD_APP_SRV_DEV_GET_MODEL_ID:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        p_attr_data[0] = ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID & 0xFF;
        p_attr_data[1] = (ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID >> 8) & 0xFF;
        break;

    case GW_CMD_APP_SRV_DEV_GET_DATE_CODE:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        p_attr_data[0] = ZB_ZCL_ATTR_BASIC_DATE_CODE_ID & 0xFF;
        p_attr_data[1] = (ZB_ZCL_ATTR_BASIC_DATE_CODE_ID >> 8) & 0xFF;

        break;

    case GW_CMD_APP_SRV_DEV_GET_SOFTWARE_ID:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        p_attr_data[0] = ZB_ZCL_ATTR_BASIC_SW_BUILD_ID & 0xFF;
        p_attr_data[1] = (ZB_ZCL_ATTR_BASIC_SW_BUILD_ID >> 8) & 0xFF;
        break;

    default:
        break;
    }

    do
    {
        if (p_attr_data == NULL || attr_data_len == 0)
        {
            break;
        }

        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_BASIC,
                            ZB_ZCL_CMD_READ_ATTRIB,
                            FALSE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);

            zigbee_zcl_msg_read_rsp_cb_reg(_zcl_basic_read_rsp_cb);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_attributes_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;
    uint16_t cluster_id = 0x0000;

    switch (cmd_id)
    {

    case GW_CMD_APP_SRV_DEV_READ_ATTRIBUTES:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }

        cluster_id = (((((uint16_t)pt_pd->parameter[2]) << 8U) & 0xFF00U) | (((uint16_t)pt_pd->parameter[1]) & 0xFFU));

        p_attr_data[0] = pt_pd->parameter[3];
        p_attr_data[1] = pt_pd->parameter[4];

        break;



    default:
        break;
    }

    do
    {
        if (p_attr_data == NULL || attr_data_len == 0)
        {
            break;
        }

        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            cluster_id,
                            ZB_ZCL_CMD_READ_ATTRIB,
                            FALSE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)


#if 1
        if (pt_data_req)
        {
            memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);

            zigbee_zcl_msg_read_rsp_cb_reg(_zcl_read_attribute_cb);
        }
#endif
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_identify_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_IDENTIFY:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }

        p_attr_data[0] = pt_pd->parameter[2];
        p_attr_data[1] = pt_pd->parameter[3];

        break;

    case GW_CMD_APP_SRV_IDENTIFY_QUERY:
        attr_data_len = 0;
        break;

    default:
        break;
    }

    do
    {
        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_IDENTIFY,
                            cmd_id,
                            TRUE, pt_pd->parameter[1],
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            if (attr_data_len > 0 && p_attr_data)
            {
                memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            }
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_group_mgmt_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t disable_default_rsp = 1;
    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_GROUP_ADD:
    case GW_CMD_APP_SRV_GROUP_VIEW:
    case GW_CMD_APP_SRV_GROUP_REMOVE:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }

        p_attr_data[0] = pt_pd->parameter[1];
        p_attr_data[1] = pt_pd->parameter[2];
        break;

    case GW_CMD_APP_SRV_GROUP_GET_MEMBERSHIP:
        attr_data_len = pkt[4] - 8;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        memcpy(p_attr_data, &pt_pd->parameter[1], attr_data_len);
        break;
    case GW_CMD_APP_SRV_GROUP_REMOVE_ALL:
        attr_data_len = 0;
        disable_default_rsp = pt_pd->parameter[1];
        break;
    case GW_CMD_APP_SRV_GROUP_ADD_IF_IDENTIFYING:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }

        p_attr_data[0] = pt_pd->parameter[2];
        p_attr_data[1] = pt_pd->parameter[3];

        disable_default_rsp = pt_pd->parameter[1];
        break;
    default:
        break;
    }

    do
    {
        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_GROUPS,
                            cmd_id,
                            TRUE, disable_default_rsp,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            if (attr_data_len > 0 && p_attr_data)
            {
                memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            }
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_scene_mgmt_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t disable_default_rsp = 1;
    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_SCENE_ADD:
        attr_data_len = pkt[4] - 8;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        memcpy(p_attr_data, &pt_pd->parameter[1], attr_data_len);
        break;

    case GW_CMD_APP_SRV_SCENE_VIEW:
    case GW_CMD_APP_SRV_SCENE_REMOVE:
    case GW_CMD_APP_SRV_SCENE_STORE:
        attr_data_len = 3;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        memcpy(p_attr_data, &pt_pd->parameter[1], attr_data_len);
        break;
    case GW_CMD_APP_SRV_SCENE_REMOVE_ALL:
    case GW_CMD_APP_SRV_SCENE_GET_EMBERSHIP:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        memcpy(p_attr_data, &pt_pd->parameter[1], attr_data_len);
        break;
    case GW_CMD_APP_SRV_SCENE_RECALL:
        attr_data_len = 3;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        disable_default_rsp = pt_pd->parameter[1];
        break;
    default:
        break;
    }

    do
    {
        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_SCENES,
                            cmd_id,
                            TRUE, disable_default_rsp,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            if (attr_data_len > 0 && p_attr_data)
            {
                memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            }
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_level_ctrl_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t disable_default_rsp = 1;
    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_LEVEL_MOVE_TO_LEVEL:
    case GW_CMD_APP_SRV_LEVEL_MOVE_TO_LEVEL_WITH_ONOFF:
        attr_data_len = 3;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_LEVEL_MOVE:
    case GW_CMD_APP_SRV_LEVEL_MOVE_WITH_ONOFF:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;
    case GW_CMD_APP_SRV_LEVEL_STEP:
    case GW_CMD_APP_SRV_LEVEL_STEP_WITH_ONOFF:
        attr_data_len = 4;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;
    case GW_CMD_APP_SRV_LEVEL_STOP:
        attr_data_len = 0;
        disable_default_rsp = pt_pd->parameter[1];
        break;
    default:
        break;
    }

    do
    {
        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
                            cmd_id,
                            TRUE, disable_default_rsp,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            if (attr_data_len > 0 && p_attr_data)
            {
                memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            }
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_onoff_ctrl_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t disable_default_rsp = 1;
    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_ONOFF_ON:
    case GW_CMD_APP_SRV_ONOFF_OFF:
    case GW_CMD_APP_SRV_ONOFF_TOGGLE:
        attr_data_len = 0;
        disable_default_rsp = pt_pd->parameter[1];
        break;

    case GW_CMD_APP_SRV_ONOFF_ON_WITH_RECALL_GLOBAL_SCENE:
        attr_data_len = 0;
        disable_default_rsp = pt_pd->parameter[1];
        cmd_id = 0x41;
        break;

    case GW_CMD_APP_SRV_ONOFF_OFF_WITH_TIMED_OFF:
        attr_data_len = 5;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        cmd_id = 0x42;
        break;
    default:
        break;
    }

    do
    {
        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_ON_OFF,
                            cmd_id,
                            TRUE, disable_default_rsp,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            if (attr_data_len > 0 && p_attr_data)
            {
                memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            }
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

static void _cmd_dev_color_ctrl_handle(uint32_t cmd_id, uint8_t *pkt)
{
    gateway_cmd_pd *pt_pd = (gateway_cmd_pd *)&pkt[5];

    zigbee_zcl_data_req_t *pt_data_req;

    uint8_t disable_default_rsp = 1;
    uint8_t attr_data_len = 0;
    uint8_t *p_attr_data = NULL;

    switch (cmd_id)
    {
    case GW_CMD_APP_SRV_COLOR_MOVE_TO_HUE:
    case GW_CMD_APP_SRV_COLOR_MOVE_TO_HUE_AND_SATURATION:
        attr_data_len = 4;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_COLOR_MOVE_HUE:
    case GW_CMD_APP_SRV_COLOR_MOVE_SATURATION:
        attr_data_len = 2;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_COLOR_STEP_HUE:
    case GW_CMD_APP_SRV_COLOR_MOVE_TO_SATURATION:
    case GW_CMD_APP_SRV_COLOR_STEP_SATURATION:
        attr_data_len = 3;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_COLOR_STEP_COLOR:
    case GW_CMD_APP_SRV_COLOR_MOVE_TO_COLOR:
        attr_data_len = 6;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_COLOR_MOVE_COLOR:
    case GW_CMD_APP_SRV_COLOR_MOVE_TO_COLOR_TEMPERATURE:
        attr_data_len = 4;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_COLOR_MOVE_COLOR_TEMPERATURE:
        attr_data_len = 7;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    case GW_CMD_APP_SRV_COLOR_STEP_COLOR_TEMPERATURE:
        attr_data_len = 9;
        p_attr_data = sys_malloc(attr_data_len);

        if (p_attr_data == NULL)
        {
            break;
        }
        disable_default_rsp = pt_pd->parameter[1];
        memcpy(p_attr_data, &pt_pd->parameter[2], attr_data_len);
        break;

    default:
        break;
    }

    do
    {
        ZIGBEE_ZCL_DATA_REQ(pt_data_req, pt_pd->address, pt_pd->address_mode,
                            pt_pd->parameter[0], ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
                            cmd_id,
                            TRUE, disable_default_rsp,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, attr_data_len)

        if (pt_data_req)
        {
            if (attr_data_len > 0 && p_attr_data)
            {
                memcpy(pt_data_req->cmdFormat, p_attr_data, attr_data_len);
            }
            zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            sys_free(pt_data_req);
        }
        if (p_attr_data)
        {
            sys_free(p_attr_data);
        }
    } while (0);
}

void gw_cmd_app_service_handle(uint32_t cmd_id, uint8_t *pkt)
{
    if ( (cmd_id >= GW_CMD_APP_SRV_DEV_BASE) &&
            (cmd_id < GW_CMD_APP_SRV_DEV_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_info_handle(cmd_id - GW_CMD_APP_SRV_DEV_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_APP_SRV_ATTRIBUTES_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_ATTRIBUTES_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_attributes_handle(cmd_id - GW_CMD_APP_SRV_ATTRIBUTES_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_APP_SRV_IDENTIFY_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_IDENTIFY_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_identify_handle(cmd_id - GW_CMD_APP_SRV_IDENTIFY_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_APP_SRV_GROUP_MGMT_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_GROUP_MGMT_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_group_mgmt_handle(cmd_id - GW_CMD_APP_SRV_GROUP_MGMT_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_APP_SRV_SCENE_MGMT_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_SCENE_MGMT_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_scene_mgmt_handle(cmd_id - GW_CMD_APP_SRV_SCENE_MGMT_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_APP_SRV_ONOFF_CTRL_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_ONOFF_CTRL_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_onoff_ctrl_handle(cmd_id - GW_CMD_APP_SRV_ONOFF_CTRL_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_APP_SRV_LEVEL_CTRL_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_LEVEL_CTRL_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_level_ctrl_handle(cmd_id - GW_CMD_APP_SRV_LEVEL_CTRL_BASE, pkt);
    }

    else if ( (cmd_id >= GW_CMD_APP_SRV_COLOR_CTRL_BASE) &&
              (cmd_id < GW_CMD_APP_SRV_COLOR_CTRL_BASE + GW_CMD_APP_CMD_OFFSET))
    {
        _cmd_dev_color_ctrl_handle(cmd_id - GW_CMD_APP_SRV_COLOR_CTRL_BASE, pkt);
    }

}
