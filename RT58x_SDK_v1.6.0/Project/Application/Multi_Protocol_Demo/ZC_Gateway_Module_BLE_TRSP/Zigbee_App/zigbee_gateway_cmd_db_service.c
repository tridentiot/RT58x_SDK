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

#include "zigbee_gateway.h"
#include "mem_mgmt.h"


//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Definitions of const value
//=============================================================================
typedef void (*gw_cmd_app_func)(uint32_t, uint8_t *);
//=============================================================================
//                Private Struct
//=============================================================================

typedef struct __attribute__((packed))
{
    uint32_t command_id;
    uint8_t parameter[];
}
gateway_db_cmd;

//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================



//=============================================================================
//                Functions
//=============================================================================


static void _cmd_device_table_handle(uint32_t cmd_id, uint8_t *pkt)
{

    info_color(LOG_RED, " _cmd_device_table_handle %d\n", cmd_id);
    switch (cmd_id)
    {
    case GW_CMD_DB_DEVICE_TABLE_GET:
    {
        device_table_get_by_idx_cmd_forward(0);
    }
    break;

    case GW_CMD_DB_DEVICE_TABLE_GET_BY_IDX:
    {
        device_table_get_by_idx_param_t *p_dev_table_get = (device_table_get_by_idx_param_t *)(pkt + GW_CMD_HEADER_LEN);
        device_table_get_rsp_t rsp_data;
        uint8_t next_idx, current_idx, rsp_len = 1, *p_rsp_data, temp_len = 0;
        //rsp data format
        //+-----------|--------------+-------------+--------------+-------------+--------------------+-----------------|----------------+--------------|
        //| status(1) | ieee addr(8) | nwk addr(2) | device id(2) | endpoint(1) | device name len(1) |device name(var) | ep name len(1) | ep name(var) |
        //+-----------|--------------+-------------+--------------+-------------+--------------------+-----------------|----------------+--------------|
        current_idx = device_table_get(&rsp_data, p_dev_table_get->start_idx);
        if (current_idx < DEVICE_TABLE_NUM)
        {
            if (rsp_data.device_info.nwk_addr != 0x0000)
            {
                rsp_len = (1/*status*/ + sizeof(device_info_param_t) + 1/*device_name_len*/ + rsp_data.device_name.name_len + 1/* ep_name_len*/ + rsp_data.ep_name.name_len);
                p_rsp_data = mem_malloc(rsp_len);

                temp_len = 1/*status*/ + sizeof(device_info_param_t) + 1/*device_name_len*/ + rsp_data.device_name.name_len;
                memcpy(p_rsp_data, (uint8_t *)&rsp_data, temp_len);

                p_rsp_data[temp_len] = rsp_data.ep_name.name_len;
                temp_len++;
                if (rsp_data.ep_name.name_len > 0)
                {
                    memcpy(p_rsp_data + temp_len, rsp_data.ep_name.name, rsp_data.ep_name.name_len);
                }

                zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_GET),
                                        0x0000, 0x00, 0x00, p_rsp_data, rsp_len);

                mem_free(p_rsp_data);
            }
            next_idx = current_idx + 1;
            device_table_get_by_idx_cmd_forward(next_idx);
        }
        else
        {
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_GET),
                                    0x0000, 0x00, 0x00, (uint8_t *)&rsp_data, rsp_len/*status*/);
        }

    }
    break;

    case GW_CMD_DB_DEVICE_RENAME:
    {
        device_table_rename_device_param_t *p_dev_table_rename = (device_table_rename_device_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[3], rsp_data_len = 1;

        //rsp data format
        //+-----------+-------------+
        //| status(1) | nwk addr(2) |
        //+-----------+-------------+

        if (device_table_device_rename(p_dev_table_rename->nwk_addr, p_dev_table_rename->len, p_dev_table_rename->name) != true)
        {
            rsp_data[0] = 0x8b;/*NOT_FOUND*/
            memcpy(&rsp_data[1], &p_dev_table_rename->nwk_addr, 2);
            rsp_data_len = 3;
        }
        else
        {
            rsp_data[0] = 0x00;
        }

        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_RENAME),
                                0x0000, 0x00, 0x00, rsp_data, rsp_data_len);

    }
    break;

    case GW_CMD_DB_END_POINT_RENAME:
    {
        device_table_rename_ep_param_t *p_dev_table_rename_ep = (device_table_rename_ep_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[4], rsp_data_len = 1;

        //rsp data format
        //+-----------+-------------+---------------+
        //| status(1) | nwk addr(2) | end point (1) |
        //+-----------+-------------+---------------+


        if (device_table_end_point_rename(p_dev_table_rename_ep->nwk_addr, p_dev_table_rename_ep->end_point,
                                          p_dev_table_rename_ep->len, p_dev_table_rename_ep->name) != true)
        {
            rsp_data[0] = 0x8b;/*NOT_FOUND*/
            memcpy(&rsp_data[1], &p_dev_table_rename_ep->nwk_addr, 2);
            rsp_data[3] = p_dev_table_rename_ep->end_point;
            rsp_data_len = 4;
        }
        else
        {
            rsp_data[0] = 0x00;
        }

        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_END_POINT_RENAME),
                                0x0000, 0x00, 0x00, rsp_data, rsp_data_len);

    }
    break;

    case GW_CMD_DB_DEVICE_TABLE_REMOVE_ALL:
    {
        uint8_t rsp_data = 0x00;

        device_table_remove_all(false, false);

        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_REMOVE_ALL),
                                0x0000, 0x00, 0x00, &rsp_data, sizeof(rsp_data));

    }
    break;

    case GW_CMD_DB_DEVICE_TABLE_REMOVE_SPECIFIC_DEVICE:
    {
        device_table_delete_param_t *p_dev_table_delete = (device_table_delete_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data = 0x00;

        if (device_table_delete(8, p_dev_table_delete->ieee_addr, NULL) != true)
        {
            rsp_data = 0x8b;/*NOT_FOUND*/
        }
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_REMOVE_SPECIFIC_DEVICE),
                                0x0000, 0x00, 0x00, &rsp_data, sizeof(rsp_data));

    }
    break;

    case GW_CMD_DB_DEVICE_TABLE_CREATE:
    {
        device_table_create_param_t *p_dev_table_create = (device_table_create_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[11];

        //rsp data format
        //+-----------|--------------+-------------+
        //| status(1) | ieee addr(8) | nwk addr(2) |
        //+-----------|--------------+-------------+

        if (device_table_create(p_dev_table_create->ieee_addr, p_dev_table_create->nwk_addr) != true)
        {
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
            memcpy(&rsp_data[1], p_dev_table_create->ieee_addr, 8);
            memcpy(&rsp_data[9], &p_dev_table_create->nwk_addr, 2);
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_CREATE),
                                    0x0000, 0x00, 0x00, rsp_data, sizeof(rsp_data));
        }

    }
    break;

    case GW_CMD_DB_DEVICE_TABLE_UPDATE:
    {
        device_table_update_param_t *p_dev_table_update = (device_table_update_param_t *)(pkt + GW_CMD_HEADER_LEN);


        //rsp data format
        //+-----------|-------------+--------------+--------------+
        //| status(1) | nwk addr(2) | device id(2) | end point(1) |
        //+-----------|-------------+--------------+--------------+

        if (device_table_update(p_dev_table_update->nwk_addr,
                                p_dev_table_update->device_id,
                                p_dev_table_update->end_point) != true)
        {
            uint8_t rsp_data[6];
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
            memcpy(&rsp_data[1], &p_dev_table_update->nwk_addr, 2);
            memcpy(&rsp_data[3], &p_dev_table_update->device_id, 2);
            rsp_data[5] = p_dev_table_update->end_point;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_UPDATE),
                                    0x0000, 0x00, 0x00, rsp_data, sizeof(rsp_data));
        }
    }
    break;

    case GW_CMD_DB_DEVICE_TABLE_REMOVE:
    {
        device_table_delete_param_t *p_dev_table_delete = (device_table_delete_param_t *)(pkt + GW_CMD_HEADER_LEN);

        device_table_delete(8, p_dev_table_delete->ieee_addr, NULL);
    }
    break;


    default:
        break;
    }

}

static void _cmd_group_table_handle(uint32_t cmd_id, uint8_t *pkt)
{
    info_color(LOG_RED, " _cmd_group_table_handle %d\n", cmd_id);
    switch (cmd_id)
    {
    case GW_CMD_DB_GROUP_TABLE_GET:
    {
        group_table_get_by_idx_cmd_forward(0);
    }
    break;

    case GW_CMD_DB_GROUP_TABLE_GET_BY_IDX:
    {
        group_table_get_by_idx_param_t *p_group_table_get = (group_table_get_by_idx_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[120], rsp_data_len = 0, next_idx, current_idx;

        // rsp data format
        /* maximum rsp data length:
           status(1) + group id(2) + group name length(1) + group name(group name length) + (nwk address(1) + endpoint(2)) * GROUP_TABLE_MEMBER_NUM */
        //+-----------|-------------+----------------------+-----------------+------------------+---------------+-----|------------------+---------------+
        //| status(1) | group id(2) | group name length(1) | group name(var) | nwk address 0(2) | endpoint 0(1) | ... | nwk address N(2) | endpoint N(1) |
        //+-----------|-------------+----------------------+-----------------+------------------+---------------+-----|------------------+---------------+

        current_idx = group_table_get(&rsp_data[1], &rsp_data_len, p_group_table_get->start_idx);
        if (current_idx < GROUP_TABLE_NUM)
        {
            rsp_data[0] = 0x00;
            rsp_data_len += 1;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_GET),
                                    0x0000, 0x00, 0x00, rsp_data, rsp_data_len);
            next_idx = current_idx + 1;
            group_table_get_by_idx_cmd_forward(next_idx);
        }
        else
        {
            rsp_data[0] = 0x01;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_GET),
                                    0x0000, 0x00, 0x00, rsp_data, 1);
        }
    }
    break;

    case GW_CMD_DB_GROUP_RENAME:
    {
        group_table_rename_param_t *p_group_rename = (group_table_rename_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rename_status, rsp_data[4 + GROUP_NAME_LEN];

        //rsp data format
        //+-----------|-------------+-------------+-----------+
        //| status(1) | group id(2) | name len(1) | name(var) |
        //+-----------|-------------+-------------+-----------+

        rename_status = group_table_rename(p_group_rename->group_id,
                                           p_group_rename->len,
                                           p_group_rename->name);

        rsp_data[0] = (rename_status == RENAME_SUCCESS) ? 0x00/*SUCCESS*/ :
                      (rename_status == RENAME_NOT_FOUND) ? 0x8b/*NOT_FOUND*/ :
                      (rename_status == RENAME_INVALID_LEN) ? 0x85/*INVALID_FIELD*/ : 0x01;/*FAILURE*/;
        memcpy(&rsp_data[1], &p_group_rename->group_id, 2);
        rsp_data[3] = p_group_rename->len;
        memcpy(&rsp_data[4], p_group_rename->name, p_group_rename->len);
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_RENAME),
                                0x0000, 0x00, 0x00, rsp_data, p_group_rename->len + 4);

    }
    break;

    case GW_CMD_DB_GROUP_TABLE_REMOVE_ALL:
    {
        uint8_t rsp_data;

        rsp_data = group_table_remove_all();

        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_REMOVE_ALL),
                                0x0000, 0x00, 0x00, &rsp_data, sizeof(rsp_data));

    }
    break;

    case GW_CMD_DB_GROUP_TABLE_CREATE:
    {
        group_table_create_param_t *p_group_create = (group_table_create_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[3];

        //rsp data format
        //+-----------|-------------+
        //| status(1) | group id(2) |
        //+-----------|-------------+

        memcpy(&rsp_data[1], &p_group_create->group_id, 2);
        if (group_table_create(p_group_create->group_id) != true)
        {
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
        }
        else
        {
            rsp_data[0] = 0x00;/*SUCCESS*/
        }
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_CREATE),
                                0x0000, 0x00, 0x00, rsp_data, sizeof(rsp_data));
    }
    break;

    case GW_CMD_DB_GROUP_TABLE_MEMBER_ADD:
    {
        group_member_add_remove_param_t *p_group_member_add = (group_member_add_remove_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[6];

        if (group_table_update(p_group_member_add->group_id,
                               p_group_member_add->nwk_addr,
                               p_group_member_add->end_point) != true)
        {
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
            memcpy(&rsp_data[1], &p_group_member_add->group_id, 2);
            memcpy(&rsp_data[3], &p_group_member_add->nwk_addr, 2);
            rsp_data[5] = p_group_member_add->end_point;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_MEMBER_ADD),
                                    0x0000, 0x00, 0x00, rsp_data, sizeof(rsp_data));
        }
    }
    break;

    case GW_CMD_DB_GROUP_TABLE_MEMBER_REMOVE:
    {
        group_member_add_remove_param_t *p_group_member_delete = (group_member_add_remove_param_t *)(pkt + GW_CMD_HEADER_LEN);

        group_table_delete(p_group_member_delete->group_id,
                           p_group_member_delete->nwk_addr,
                           p_group_member_delete->end_point);
    }
    break;

    case GW_CMD_DB_GROUP_TABLE_DEVICE_ALL_GROUPS_REMOVE:
    {
        group_all_groups_remove_param_t *p_all_group_delete = (group_all_groups_remove_param_t *)(pkt + GW_CMD_HEADER_LEN);
        group_table_delete(ALL_GROUP,
                           p_all_group_delete->nwk_addr,
                           p_all_group_delete->end_point);
    }
    break;



    default:
        break;
    }

}

static void _cmd_scene_table_handle(uint32_t cmd_id, uint8_t *pkt)
{
    info_color(LOG_RED, "_cmd_scene_table_handle %d\n", cmd_id);

    switch (cmd_id)
    {
    case GW_CMD_DB_SCENE_TABLE_GET:
        scene_table_get_by_idx_cmd_forward(0);
        break;

    case GW_CMD_DB_SCENE_TABLE_GET_BY_IDX:
    {
        scene_table_get_by_idx_param_t *p_scene_table_get = (scene_table_get_by_idx_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[121], rsp_data_len = 0, next_idx, current_idx;

        //rsp data format
        //maximum rsp data length: 1 + 2 + 1 + GROUP_NAME_LEN + (1 + 2) * GROUP_TABLE_MEMBER_NUM
        //+-----------|-------------+-------------+----------------------+-----------------+------------------+---------------+-----|------------------+---------------+
        //| status(1) | group id(2) | scene id(1) | scene name length(1) | scene name(var) | nwk address 0(2) | endpoint 0(1) | ... | nwk address N(2) | endpoint N(1) |
        //+-----------|-------------+-------------+----------------------+-----------------+------------------+---------------+-----|------------------+---------------+

        current_idx = scene_table_get(&rsp_data[1], &rsp_data_len, p_scene_table_get->start_idx);
        if (current_idx < SCENE_TABLE_NUM)
        {
            rsp_data[0] = 0x00;
            rsp_data_len += 1;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_GET),
                                    0x0000, 0x00, 0x00, rsp_data, rsp_data_len);
            next_idx = current_idx + 1;
            scene_table_get_by_idx_cmd_forward(next_idx);
        }
        else
        {
            rsp_data[0] = 0x01;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_GET),
                                    0x0000, 0x00, 0x00, rsp_data, 1);
        }

    }
    break;

    case GW_CMD_DB_SCENE_RENAME:
    {
        scene_table_rename_param_t *p_scene_rename = (scene_table_rename_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rename_status, rsp_data[5 + SCENE_NAME_LEN];

        //rsp data format
        //+-----------|-------------+--------------+-------------+------------+
        //| status(1) | group id(2) |  scene id(1) | name len(1) | name (var) |
        //+-----------|-------------+--------------+-------------+------------+

        rename_status = scene_table_rename(p_scene_rename->group_id,
                                           p_scene_rename->scene_id,
                                           p_scene_rename->len,
                                           p_scene_rename->name);

        rsp_data[0] = (rename_status == RENAME_SUCCESS) ? 0x00/*SUCCESS*/ :
                      (rename_status == RENAME_NOT_FOUND) ? 0x8b/*NOT_FOUND*/ :
                      (rename_status == RENAME_INVALID_LEN) ? 0x85/*INVALID_FIELD*/ : 0x01;/*FAILURE*/;
        memcpy(&rsp_data[1], &p_scene_rename->group_id, 2);
        rsp_data[3] = p_scene_rename->scene_id;
        rsp_data[4] = p_scene_rename->len;
        memcpy(&rsp_data[5], p_scene_rename->name, p_scene_rename->len);
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_RENAME),
                                0x0000, 0x00, 0x00, rsp_data, p_scene_rename->len + 5);

    }
    break;

    case GW_CMD_DB_SCENE_TABLE_REMOVE_ALL:
    {
        uint8_t rsp_data;

        rsp_data = scene_table_remove_all();

        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_REMOVE_ALL),
                                0x0000, 0x00, 0x00, &rsp_data, sizeof(rsp_data));

    }
    break;

    case GW_CMD_DB_SCENE_TABLE_CREATE:
    {
        scene_table_create_param_t *p_scene_create = (scene_table_create_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[4];

        //rsp data format
        //+-----------|-------------+--------------+
        //| status(1) | group id(2) |  scene id(1) |
        //+-----------|-------------+--------------+

        memcpy(&rsp_data[1], &p_scene_create->group_id, 2);
        rsp_data[3] = p_scene_create->secne_id;
        if (scene_table_create(p_scene_create->group_id, p_scene_create->secne_id) != true)
        {
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
        }
        else
        {
            rsp_data[0] = 0x00;/*SUCCESS*/
        }
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_CREATE),
                                0x0000, 0x00, 0x00, rsp_data, sizeof(rsp_data));

    }
    break;

    case GW_CMD_DB_SCENE_TABLE_MEMBER_ADD:
    {
        scene_member_add_remove_param_t *p_scene_member_add = (scene_member_add_remove_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[7];

        //rsp data format
        //+-----------|-------------+-------------+--------------+--------------+
        //| status(1) | group id(2) | scene id(1) |  nwk addr(2) | end point(1) |
        //+-----------|-------------+-------------+--------------+--------------+

        if (scene_table_update(p_scene_member_add->group_id,
                               p_scene_member_add->scene_id,
                               p_scene_member_add->nwk_addr,
                               p_scene_member_add->end_point) != true)
        {
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
            memcpy(&rsp_data[1], &p_scene_member_add->group_id, 2);
            rsp_data[3] = p_scene_member_add->scene_id;
            memcpy(&rsp_data[4], &p_scene_member_add->nwk_addr, 2);
            rsp_data[6] = p_scene_member_add->end_point;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_MEMBER_ADD),
                                    0x0000, 0x00, 0x00, rsp_data, sizeof(rsp_data));
        }
    }
    break;

    case GW_CMD_DB_SCENE_TABLE_MEMBER_REMOVE:
    {
        scene_member_add_remove_param_t *p_scene_member_remove = (scene_member_add_remove_param_t *)(pkt + GW_CMD_HEADER_LEN);
        info_color(LOG_RED, "scene remove, group 0x%04x scene 0x%02x\n", p_scene_member_remove->group_id,
                   p_scene_member_remove->scene_id);
        scene_table_delete(p_scene_member_remove->group_id,
                           p_scene_member_remove->scene_id,
                           p_scene_member_remove->nwk_addr,
                           p_scene_member_remove->end_point);
    }
    break;

    default:
        break;
    }

}

static void _cmd_bind_table_handle(uint32_t cmd_id, uint8_t *pkt)
{
    info_color(LOG_RED, " _cmd_bind_table_handle %d\n", cmd_id);
    switch (cmd_id)
    {
    case GW_CMD_DB_BIND_TABLE_GET:
    {
        bind_table_get_by_idx_cmd_forward(0);
    }
    break;

    case GW_CMD_DB_BIND_TABLE_GET_BY_IDX:
    {
        bind_table_get_by_idx_param_t *p_bind_table_get = (bind_table_get_by_idx_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[22], addr_mode, addr_len, next_idx, current_idx;

        //rsp data format
        //+-----------|------------------+-----------------+---------------+------------------+------------------+-------------------|
        //| status(1) | src ieee addr(8) | src endpoint(1) | cluster id(2) | dst addr mode(1) | dst address(2/8) | dst endpoint(0/1) |
        //+-----------|------------------+-----------------+---------------+------------------+------------------+-------------------|
        current_idx = bind_table_get(&rsp_data[1], p_bind_table_get->start_idx);
        if (current_idx < BIND_TABLE_NUM)
        {
            rsp_data[0] = 0x00;
            addr_mode = rsp_data[12];
            addr_len = (addr_mode == ADDR_MODE_GROUP) ? 2 : 9;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_GET),
                                    0x0000, 0x00, 0x00, rsp_data, 1/*status*/ + 12/*src ieee addr + src endpoint + cluster id + dst addr mode*/ + addr_len);
            next_idx = current_idx + 1;
            bind_table_get_by_idx_cmd_forward(next_idx);
        }
        else
        {
            rsp_data[0] = 0x01;
            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_GET),
                                    0x0000, 0x00, 0x00, rsp_data, 1/*status*/);
        }

    }
    break;

    case GW_CMD_DB_BIND_TABLE_REMOVE_ALL:
    {
        uint8_t rsp_data;

        rsp_data = bind_table_remove_all();
        zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_REMOVE_ALL),
                                0x0000, 0x00, 0x00, &rsp_data, sizeof(rsp_data));

    }
    break;

    case GW_CMD_DB_BIND_TABLE_CRATE:
    {
        bind_table_bind_unbind_param_t *p_bind = (bind_table_bind_unbind_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint8_t rsp_data[22], addr_len = 0;

        //rsp data format
        //+-----------|------------------+-----------+---------------+-----------------+-----------+
        //| status(1) | src ieee addr(8) | src ep(1) | cluster id(2) | address mode(1) | addr(var) |
        //+-----------|------------------+-----------+---------------+-----------------+-----------+

        if (bind_table_create(p_bind) != true)
        {
            rsp_data[0] = 0x89;/*INSUFFICIENT_SPACE*/
            rsp_data[1] = p_bind->bind_info.src_ep;
            memcpy(&rsp_data[2], p_bind->bind_info.src_ieee_addr, 8);
            memcpy(&rsp_data[10], (uint8_t *)&p_bind->bind_info.cluster_id, 2);
            rsp_data[12] = p_bind->bind_info.dst_addr_mode;
            if (p_bind->bind_info.dst_addr_mode == ADDR_MODE_GROUP)
            {
                addr_len = 2;
            }
            else
            {
                addr_len = 9;
            }
            memcpy(&rsp_data[13], (uint8_t *)&p_bind->dst_addr, addr_len);

            zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_CRATE),
                                    0x0000, 0x00, 0x00, rsp_data, 1 + 8 + 1 + 2 + 1 + addr_len);
        }
    }
    break;

    case GW_CMD_DB_BIND_TABLE_REMOVE:
    {
        bind_table_bind_unbind_param_t *p_unbind = (bind_table_bind_unbind_param_t *)(pkt + GW_CMD_HEADER_LEN);

        bind_table_delete(p_unbind);
    }
    break;


    default:
        break;
    }

}

static void _cmd_zc_info_handle(uint32_t cmd_id, uint8_t *pkt)
{
    uint8_t ieee_addr[8] = {0x11, 0x33, 0x55, 0x77, 0xa1, 0xb1, 0xc1, 0xd1}; /*fake ieee address of zc for saving information*/

    //info_color(LOG_RED, "_cmd_zc_info_handle %d\n", cmd_id);
    switch (cmd_id)
    {
    case GW_CMD_DB_ZC_INFO_GET:
    case GW_CMD_DB_ZC_INFO_INTERNAL_UPDATE:
    {
        device_table_get_rsp_t device_data;
        uint8_t table_idx = 0, rsp_data[4], current_idx = 0;

        //device data format
        //+--------------+-------------+-----------+------------+--------------------+-----------------|
        //| ieee addr(8) | nwk addr(2) | pan id(2) | channel(1) | device name len(1) |device name(var) |
        //+--------------+-------------+-----------+------------+--------------------+-----------------|

        //rsp data format
        //+-----------|------------+------------+
        //| status(1) |  pan id(2) | channel(1) |
        //+-----------|------------+------------+
        current_idx = device_table_get(&device_data, table_idx);
        while (current_idx < DEVICE_TABLE_NUM)
        {
            if (device_data.device_info.nwk_addr == 0x0000)
            {
                rsp_data[0] = 0x00;/*success*/
                memcpy((uint8_t *)&g_pan_id, (uint8_t *)&device_data.device_info.device_id, 2);
                memcpy((uint8_t *)&rsp_data[1], (uint8_t *)&device_data.device_info.device_id, 2);

                g_channel = device_data.device_info.end_point;
                rsp_data[3] = device_data.device_info.end_point;


                if (cmd_id == GW_CMD_DB_ZC_INFO_GET)
                {
                    zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_ZC_INFO_BASE | GW_CMD_DB_ZC_INFO_GET),
                                            0x0000, 0x00, 0x00, rsp_data, 1/*status*/ + 2/*pan id*/ + 1/*channel*/);
                }
                else
                {
                    if (memcmp(device_data.device_info.ieee_addr, ieee_addr, 8) == 0)
                    {
                        info_color(LOG_YELLOW, "PAN ID 0x%04x\n", g_pan_id);
                        info_color(LOG_YELLOW, "Channel %d\n", (g_channel & 0x7F));
                    }
                    else
                    {
                        info_color(LOG_YELLOW, "initialize each tables...waiting for start command\n");
                        g_pan_id = 0xFFFF;
                        g_channel = 0xFF;
                        group_table_remove_all();
                        scene_table_remove_all();
                        device_table_remove_all(true, true);
                    }

                }

                break;
            }
            table_idx = current_idx + 1;

            current_idx = device_table_get(&device_data, table_idx);
        }

        if (current_idx == DEVICE_TABLE_NUM)
        {
            if (cmd_id == GW_CMD_DB_ZC_INFO_GET)
            {
                rsp_data[0] = 0x8b;/*NOT_FOUND*/
                zigbee_gateway_cmd_send(GW_CMD_APP_CMD_RSP_GEN(GW_CMD_DB_BASE | GW_CMD_DB_ZC_INFO_BASE | GW_CMD_DB_ZC_INFO_GET),
                                        0x0000, 0x00, 0x00, rsp_data, 1/*status*/);
                info_color(LOG_YELLOW, "PAN ID & Channel not found\n");
            }
            else if (cmd_id == GW_CMD_DB_ZC_INFO_INTERNAL_UPDATE)
            {
                info_color(LOG_YELLOW, "initialize each tables and waiting for start command\n");
                group_table_remove_all();
                scene_table_remove_all();
                device_table_remove_all(true, true);
            }

        }
    }
    break;


    case GW_CMD_DB_ZC_INFO_CREATE:
    {
        zc_info_create_param_t *p_zc_info_set = (zc_info_create_param_t *)(pkt + GW_CMD_HEADER_LEN);
        uint16_t zc_nwk_addr = 0x0000;

        if (device_table_create(ieee_addr, zc_nwk_addr) == true)
        {
            if (device_table_update(zc_nwk_addr, p_zc_info_set->pan_id, p_zc_info_set->channel) != true)
            {
                info_color(LOG_RED, "zc info update fail 0x%04x %d\n", p_zc_info_set->pan_id, p_zc_info_set->channel);
            }
            else
            {
                g_pan_id = p_zc_info_set->pan_id;
                g_channel = p_zc_info_set->channel;
                if (p_zc_info_set->sys_reboot)
                {
                    Sys_Software_Reset();
                }
            }
        }
        else
        {
            info_color(LOG_RED, "zc info create fail\n");
        }
    }
    break;

    case GW_CMD_DB_ZC_INFO_REMOVE:
        device_table_delete(8, ieee_addr, NULL);
        break;
    }
}

void gw_cmd_db_service_handle(uint32_t cmd, uint8_t *pkt)
{
    uint32_t cmd_id = cmd & 0xFFFF;
    if ( (cmd_id >= GW_CMD_DB_DEVICE_TABLE_BASE) &&
            (cmd_id < GW_CMD_DB_DEVICE_TABLE_BASE + GW_CMD_DB_CMD_OFFSET))
    {
        _cmd_device_table_handle(cmd_id - GW_CMD_DB_DEVICE_TABLE_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_DB_GROUP_TABLE_BASE) &&
              (cmd_id < GW_CMD_DB_GROUP_TABLE_BASE + GW_CMD_DB_CMD_OFFSET))
    {
        _cmd_group_table_handle(cmd_id - GW_CMD_DB_GROUP_TABLE_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_DB_SCENE_TABLE_BASE) &&
              (cmd_id < GW_CMD_DB_SCENE_TABLE_BASE + GW_CMD_DB_CMD_OFFSET))
    {
        _cmd_scene_table_handle(cmd_id - GW_CMD_DB_SCENE_TABLE_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_DB_BIND_TABLE_BASE) &&
              (cmd_id < GW_CMD_DB_BIND_TABLE_BASE + GW_CMD_DB_CMD_OFFSET))
    {
        _cmd_bind_table_handle(cmd_id - GW_CMD_DB_BIND_TABLE_BASE, pkt);
    }
    else if ( (cmd_id >= GW_CMD_DB_ZC_INFO_BASE) &&
              (cmd_id < GW_CMD_DB_ZC_INFO_BASE + GW_CMD_DB_CMD_OFFSET))
    {
        _cmd_zc_info_handle(cmd_id - GW_CMD_DB_ZC_INFO_BASE, pkt);
    }


}
