/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_zcl_msg_handler.c
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

#include "zigbee_zcl_msg_handler.h"
#include "zigbee_gateway.h"

#include "bsp_led.h"

typedef void (*zcl_read_rsp_cb)(uint16_t cluster_id, uint16_t addr, uint8_t src_endp, uint8_t *pd, uint8_t pd_len);


static void _zcl_identify_handle(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata);
static void _zcl_group_handle(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata);
static void _zcl_scene_handle(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata);

static void(*zcl_cluster_gen_func_list[])(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata) =
{
    [ZB_ZCL_CLUSTER_ID_BASIC] = NULL,
    [ZB_ZCL_CLUSTER_ID_POWER_CONFIG] = NULL,
    [ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG] = NULL,
    [ZB_ZCL_CLUSTER_ID_IDENTIFY] = _zcl_identify_handle,
    [ZB_ZCL_CLUSTER_ID_GROUPS] = _zcl_group_handle,
    [ZB_ZCL_CLUSTER_ID_SCENES] = _zcl_scene_handle,
    [ZB_ZCL_CLUSTER_ID_ON_OFF] = NULL,
    [ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG] = NULL,
    [ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL] = NULL,
    [ZB_ZCL_CLUSTER_ID_ALARMS] = NULL,
    [ZB_ZCL_CLUSTER_ID_TIME] = NULL,
};


static void (*zcl_cluster_ms_func_list[])(uint8_t cmd, uint16_t datalen, uint8_t *pdata) =
{
    [ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT - ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT] = NULL,
    [ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT - ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT] = NULL,
};

static zcl_read_rsp_cb p_read_rsp_cb = NULL;

//=============================================================================
//                Private Function Declaration
//=============================================================================
static void _zcl_identify_handle(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata)
{
    zigbee_gateway_cmd_send(0x00048001, src_addr, 0, src_endp, (uint8_t *)pdata, datalen);
}

static void _zcl_group_handle(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata)
{

    zigbee_gateway_cmd_send(0x00058000 | cmd, src_addr, 0, src_endp, (uint8_t *)pdata, datalen);
    info_color(LOG_RED, " _zcl_group_handle %d src_endp %d\n", cmd, src_endp);

    switch (cmd)
    {
    case GW_CMD_APP_SRV_GROUP_ADD:
    {
        uint8_t status = pdata[0];
        uint16_t group_id;
        memcpy((uint8_t *)&group_id, pdata + 1, 2);
        if (status == 0x00)
        {
#if 0
            if (group_id_valid(group_id) == false)
            {
                info_color(LOG_RED, "group id 0x%04x invalid\n", group_id);
            }
            else
#endif
            {
                group_member_add_cmd_forward(src_addr, src_endp, group_id);
            }
        }
    }
    break;

    case GW_CMD_APP_SRV_GROUP_REMOVE:
    {
        uint8_t status = pdata[0];
        uint16_t group_id;
        memcpy((uint8_t *)&group_id, pdata + 1, 2);

        if (status == 0x00)
        {
#if 0
            if (group_id_valid(group_id) == false)
            {
                info_color(LOG_RED, "group id 0x%04x invalid\n", group_id);
            }
            else
#endif
            {
                group_member_remove_cmd_forward(src_addr, src_endp, group_id);
            }
        }
    }
    break;

    case GW_CMD_APP_SRV_GROUP_REMOVE_ALL:
    {
        group_all_groups_remove_cmd_forward(src_addr, src_endp);
    }
    break;

    }

}


static void _zcl_scene_handle(uint8_t cmd, uint16_t src_addr, uint8_t src_endp, uint16_t datalen, uint8_t *pdata)
{
    zigbee_gateway_cmd_send(0x00068000 | cmd, src_addr, 0, src_endp, (uint8_t *)pdata, datalen);

    switch (cmd)
    {
    case GW_CMD_APP_SRV_SCENE_ADD:
    case GW_CMD_APP_SRV_SCENE_STORE:
    {
        uint8_t status = pdata[0], secne_id;
        uint16_t group_id;

        if (status == 0x00)
        {
            memcpy((uint8_t *)&group_id, &pdata[1], 2);
            secne_id = pdata[3];
            scene_member_add_cmd_forward(src_addr, src_endp, group_id, secne_id);
        }
    }
    break;

    case GW_CMD_APP_SRV_SCENE_REMOVE:
    {
        uint8_t status = pdata[0], secne_id;
        uint16_t group_id;

        if (status == 0x00)
        {
            memcpy((uint8_t *)&group_id, &pdata[1], 2);
            secne_id = pdata[3];
            scene_member_remove_cmd_forward(src_addr, src_endp, group_id, secne_id);
        }
    }
    break;

    case GW_CMD_APP_SRV_SCENE_REMOVE_ALL:
    {
        uint8_t status = pdata[0];
        uint16_t group_id;

        if (status == 0x00)
        {
            memcpy((uint8_t *)&group_id, &pdata[1], 2);
            scene_member_remove_cmd_forward(src_addr, src_endp, group_id, ALL_SCENE);
        }
    }
    break;

    }
}


void zigbee_zcl_msg_read_rsp_cb_reg(void *cb)
{
    p_read_rsp_cb = (zcl_read_rsp_cb)cb;
}

void zigbee_zcl_msg_handler(sys_tlv_t *pt_tlv)
{
    zigbee_zcl_data_idc_t *pt_zcl_msg = (zigbee_zcl_data_idc_t *)pt_tlv->value;
    uint8_t i = 0;

    do
    {
        if (!pt_zcl_msg)
        {
            break;
        }

        if (pt_zcl_msg->clusterID == ZB_ZCL_CLUSTER_ID_OTA_UPGRADE)
        {
            // ignore
            break;
        }

        if (pt_zcl_msg->clusterID == ZB_ZCL_CLUSTER_ID_CUSTOM)
        {
            if (pt_zcl_msg->cmd == UART_TRANSPARENT_CMD)
            {
                info_color(LOG_YELLOW, "[%04X] ", pt_zcl_msg->srcAddr);
                for (i = 0; i < pt_zcl_msg->cmdFormatLen; i++)
                {
                    info_color(LOG_YELLOW, "%c", pt_zcl_msg->cmdFormat[i]);
                }
                info_color(LOG_YELLOW, "\n");
            }
            break;
        }

        info("Recv ZCL message 0x%04X -> 0x%04X\n", pt_zcl_msg->srcAddr, pt_zcl_msg->dstAddr);
        info("Cluster %04x cmd %d seq %d Src ep%d\n", pt_zcl_msg->clusterID, pt_zcl_msg->cmd, pt_zcl_msg->seq_num, pt_zcl_msg->srcEndpint);
        util_log_mem(UTIL_LOG_INFO, "  ", (uint8_t *)pt_zcl_msg->cmdFormat, pt_zcl_msg->cmdFormatLen, 0);

        if (pt_zcl_msg->is_common_command)
        {
            if (pt_zcl_msg->cmd == 0x01) // Read response
            {
                if (p_read_rsp_cb)
                {
                    p_read_rsp_cb(pt_zcl_msg->clusterID, pt_zcl_msg->srcAddr, pt_zcl_msg->srcEndpint, (uint8_t *)pt_zcl_msg->cmdFormat, pt_zcl_msg->cmdFormatLen);
                }
            }
            if (pt_zcl_msg->cmd == 0x0b) // defaut response
            {
                zigbee_gateway_cmd_send(0x00018800, pt_zcl_msg->srcAddr, 0, pt_zcl_msg->srcEndpint, (uint8_t *)pt_zcl_msg->cmdFormat, pt_zcl_msg->cmdFormatLen);
                gw_default_rsp_check(pt_zcl_msg->clusterID, pt_zcl_msg->cmdFormat[0], pt_zcl_msg->srcAddr, pt_zcl_msg->srcEndpint, pt_zcl_msg->cmdFormat[1]);
            }
        }
        else
        {
            if (pt_zcl_msg->clusterID <= ZB_ZCL_CLUSTER_ID_TIME)
            {

                if (zcl_cluster_gen_func_list[pt_zcl_msg->clusterID])
                    zcl_cluster_gen_func_list[pt_zcl_msg->clusterID]
                    (pt_zcl_msg->cmd, pt_zcl_msg->srcAddr, pt_zcl_msg->srcEndpint, pt_zcl_msg->cmdFormatLen, pt_zcl_msg->cmdFormat);

            }
            else if (pt_zcl_msg->clusterID >= ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT &&
                     pt_zcl_msg->clusterID <= ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT)
            {
                if (zcl_cluster_ms_func_list[pt_zcl_msg->clusterID - ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT])
                    zcl_cluster_ms_func_list[pt_zcl_msg->clusterID - ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT]
                    (pt_zcl_msg->cmd, pt_zcl_msg->cmdFormatLen, pt_zcl_msg->cmdFormat);
            }
            else
            {
                info_color(LOG_RED, "Not support cluser %04X\n", pt_zcl_msg->clusterID);
            }
        }

    } while (0);
}
