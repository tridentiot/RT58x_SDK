/**
 * @file mmdl_scene_sr_main.c
 * @author Joshua Jean (joshua.jean@rafaelmicro.com)
 * @brief
 * @version 0.1
 * @date 2022-07-25
 *
 * @copyright Copyright (c) 2022
 *
 */

//=============================================================================
//                Include
//=============================================================================
#include "sys_arch.h"
#include "mmdl_common.h"
#include "mmdl_scene_sr.h"
#include "mmdl_scene_common.h"

//=============================================================================
//                Private Definitions of const value
//=============================================================================

#define DEVICE_DB_START_ADDRESS  0xF0000
#define DEVICE_DB_END_ADDRESS    0xF2000

//=============================================================================
//                Private Global Variables
//=============================================================================

/* store the useful data pointer for futher using */
static uint8_t          ttl;

//=============================================================================
//                Private Functions Declaration
//=============================================================================
static void mmdl_scene_get(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model);
static void mmdl_scene_recall(uint32_t is_ack, mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model);
static void mmdl_scene_register_get(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model);
static void mmdl_scene_store(uint32_t is_ack, mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model);
static void mmdl_scene_delete(uint32_t is_ack, mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model);
static void mmdl_scene_send_status(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model, uint8_t status_code);
static void mmdl_scene_send_register_status(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model, uint8_t status_code);

void scene_info_reset(void)
{
    ds_reset_to_default();
}

void scene_info_update(uint8_t element_address, uint32_t update_address, uint32_t update_len)
{
    uint8_t  update_index = ((element_address + 1) - pib_primary_address_get());
    ds_rw_t  scene_write;

    scene_write.type = update_index;
    scene_write.len = update_len;
    scene_write.address = update_address;
    ds_write(&scene_write);
}

void mmdl_scene_sr_init(ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model)
{
    scene_state_t  *p_scene_state = (scene_state_t *)p_model->p_state_var;
    ds_rw_t        scene_db_read;
    ds_config_t    ds_cfg;
    uint8_t        update_idx = ((p_element->element_address + 1) - pib_primary_address_get());

    ds_cfg.start_address = DEVICE_DB_START_ADDRESS;
    ds_cfg.end_address = DEVICE_DB_END_ADDRESS;
    ds_initinal(ds_cfg);

    scene_db_read.type = update_idx;
    if (ds_read(&scene_db_read) == STATUS_SUCCESS)
    {
        memcpy((uint8_t *)p_scene_state, (uint8_t *)scene_db_read.address, scene_db_read.len);
    }
    else
    {
        memset(p_scene_state, 0, sizeof(scene_state_t));
    }
}

void mmdl_scene_sr_handler(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model, uint8_t is_broadcast)
{

    /* store the data in private variables */
    ttl = 30;

    /* process the message by different opcode */
    switch (LE2BE16(p_evt_msg->opcode))
    {
    case MMDL_SCENE_GET_OPCODE:
        mmdl_scene_get(p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_RECALL_OPCODE:
        mmdl_scene_recall(TRUE, p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_RECALL_NO_ACK_OPCODE:
        mmdl_scene_recall(FALSE, p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_REGISTER_GET_OPCODE:
        mmdl_scene_register_get(p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_STORE_OPCODE:
        mmdl_scene_store(TRUE, p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_STORE_NO_ACK_OPCODE:
        mmdl_scene_store(FALSE, p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_DELETE_OPCODE:
        mmdl_scene_delete(TRUE, p_evt_msg, p_element, p_model);
        break;

    case MMDL_SCENE_DELETE_NO_ACK_OPCODE:
        mmdl_scene_delete(FALSE, p_evt_msg, p_element, p_model);
        break;
    }

}

static void mmdl_scene_get(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model)
{
    mmdl_scene_send_status(p_evt_msg, p_element, p_model, MMDL_SCENE_SUCCESS);
}

static void mmdl_scene_recall(uint32_t ack_flag, mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model)
{
    scene_recall_msg_t  *p_scene_recall_msg = (scene_recall_msg_t *)p_evt_msg->parameter;
    scene_state_t       *p_scene_state   = (scene_state_t *)p_model->p_state_var;
    void                *p_extend_set_func = NULL;
    uint8_t             status = MMDL_SCENE_NOT_FOUND, i;

    if (tid == p_scene_recall_msg->tid)
    {
        return;
    }

    if (p_scene_recall_msg->scene_num == 0x0000) /*The value 0x0000 is Prohibited*/
    {
        return;
    }

    for (i = 0; i < RAF_BLE_MESH_SCENE_ENTRY_COUNT; i++)
    {
        if (p_scene_state->scene_register[i].scene_number == p_scene_recall_msg->scene_num)
        {
            status = MMDL_SCENE_SUCCESS;
            p_scene_state->current_scene = p_scene_recall_msg->scene_num;
            p_scene_state->target_scene = p_scene_recall_msg->scene_num;
            break;
        }
    }

    if (ack_flag)
    {
        mmdl_scene_send_status(p_evt_msg, p_element, p_model, status);
    }

    if (status == MMDL_SCENE_SUCCESS)
    {
        ((model_scene_call_back)(p_model->p_user_call_back))
        (p_element->element_address, SCENE_ACTION_RECALL, &(p_scene_state->scene_register[i].scene_state), &p_extend_set_func);

        if (p_extend_set_func != NULL)
        {
            ((model_scene_set_extend)p_extend_set_func)(p_element);
        }
    }
}

static void mmdl_scene_register_get(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model)
{
    mmdl_scene_send_register_status(p_evt_msg, p_element, p_model, MMDL_SCENE_SUCCESS);
}

static void mmdl_scene_store(uint32_t ack_flag, mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model)
{
    scene_store_msg_t  *p_scene_store_msg = (scene_store_msg_t *)p_evt_msg->parameter;
    scene_state_t      *p_scene_state   = (scene_state_t *)p_model->p_state_var;
    uint8_t             status = MMDL_SCENE_REGISTER_FULL, i;

    if (p_scene_store_msg->scene_num == 0x0000) /*The value 0x0000 is Prohibited*/
    {
        return;
    }

    for (i = 0; i < RAF_BLE_MESH_SCENE_ENTRY_COUNT; i++)
    {
        if (p_scene_state->scene_register[i].scene_number == p_scene_store_msg->scene_num)
        {
            p_scene_state->current_scene = p_scene_store_msg->scene_num;
            status = MMDL_SCENE_SUCCESS;
            break;
        }
    }

    if (i == RAF_BLE_MESH_SCENE_ENTRY_COUNT)
    {
        for (i = 0; i < RAF_BLE_MESH_SCENE_ENTRY_COUNT; i++)
        {
            if (p_scene_state->scene_register[i].scene_number == 0x0000)
            {
                p_scene_state->scene_register[i].scene_number = p_scene_store_msg->scene_num;
                p_scene_state->current_scene = p_scene_store_msg->scene_num;
                status = MMDL_SCENE_SUCCESS;
                break;
            }
        }
    }

    if (ack_flag)
    {
        mmdl_scene_send_register_status(p_evt_msg, p_element, p_model, status);
    }

    if (status == MMDL_SCENE_SUCCESS)
    {
        ((model_scene_call_back)(p_model->p_user_call_back)) (p_element->element_address,
                SCENE_ACTION_STORE, &(p_scene_state->scene_register[i].scene_state), NULL);
        scene_info_update(p_element->element_address, (uint32_t)p_model->p_state_var, sizeof(scene_state_t));
    }
}

static void mmdl_scene_delete(uint32_t ack_flag, mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model)
{
    scene_delete_msg_t  *p_scene_delete_msg = (scene_delete_msg_t *)p_evt_msg->parameter;
    scene_state_t       *p_scene_state   = (scene_state_t *)p_model->p_state_var;
    uint8_t             status = MMDL_SCENE_NOT_FOUND, i;

    if (p_scene_delete_msg->scene_num == 0x0000) /*The value 0x0000 is Prohibited*/
    {
        return;
    }

    for (i = 0; i < RAF_BLE_MESH_SCENE_ENTRY_COUNT; i++)
    {
        if (p_scene_state->scene_register[i].scene_number == p_scene_delete_msg->scene_num)
        {
            p_scene_state->scene_register[i].scene_number = 0x0000;
            if (p_scene_state->current_scene == p_scene_delete_msg->scene_num)
            {
                p_scene_state->current_scene = 0x0000;
            }
            status = MMDL_SCENE_SUCCESS;
            break;
        }
    }

    if (ack_flag)
    {
        mmdl_scene_send_register_status(p_evt_msg, p_element, p_model, status);
    }

    if (status == MMDL_SCENE_SUCCESS)
    {
        ((model_scene_call_back)(p_model->p_user_call_back)) (p_element->element_address,
                SCENE_ACTION_DELETE, &(p_scene_state->scene_register[i].scene_state), NULL);
        scene_info_update(p_element->element_address, (uint32_t)p_model->p_state_var, sizeof(scene_state_t));
    }
}

static void mmdl_scene_send_status(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model, uint8_t status_code)
{
    mesh_app_mdl_evt_msg_req_t  *pt_req;
    mesh_tlv_t                  *pt_tlv;
    scene_state_t               *p_scene_state   = (scene_state_t *)p_model->p_state_var;
    scene_status_msg_t          *p_scene_status_msg;

    do
    {
        pt_tlv = pvPortMalloc(sizeof(mesh_tlv_t) + sizeof(mesh_app_mdl_evt_msg_req_t) + sizeof(scene_status_msg_t));
        if (pt_tlv == NULL)
        {
            break;
        }

        pt_tlv->type = TYPE_MESH_APP_MDL_EVT_MSG_REQ;
        pt_tlv->length = sizeof(mesh_app_mdl_evt_msg_req_t) + sizeof(scene_status_msg_t);

        pt_req = (mesh_app_mdl_evt_msg_req_t *)pt_tlv->value;

        pt_req->dst_addr = p_evt_msg->src_addr;
        pt_req->src_addr = p_element->element_address;;
        pt_req->appkey_index = p_evt_msg->appkey_index;
        pt_req->ttl = ttl;
        pt_req->opcode_invalid = 0;

        pt_req->opcode = MMDL_SCENE_STATUS_OPCODE;
        pt_req->parameter_len = sizeof(scene_status_msg_t);
        p_scene_status_msg = (scene_status_msg_t *)pt_req->parameter;
        p_scene_status_msg->status_code = status_code;
        p_scene_status_msg->current_scene = p_scene_state->current_scene;
        /* TODO: remaining time must be processed */
        //p_scene_status_msg->remaining_time = 0;

        if (mesh_evt_msg_sendto(pt_tlv) != 0)
        {
            info_color(LOG_RED, "Send msg to mesh stack fail\n");
            break;
        }
    } while (0);

    if (pt_tlv)
    {
        vPortFree(pt_tlv);
    }
}

static void mmdl_scene_send_register_status(mesh_app_mdl_evt_msg_idc_t *p_evt_msg, ble_mesh_element_param_t *p_element, ble_mesh_model_param_t *p_model, uint8_t status_code)
{
    mesh_app_mdl_evt_msg_req_t   *pt_req;
    mesh_tlv_t                   *pt_tlv;
    scene_state_t                *p_scene_state   = (scene_state_t *)p_model->p_state_var;
    scene_register_status_msg_t  *p_scene_reg_status_msg;
    uint8_t                      i = 0, valid_scene = 0;
    uint16_t                     scene_number[RAF_BLE_MESH_SCENE_ENTRY_COUNT];

    do
    {
        for (i = 0; i < RAF_BLE_MESH_SCENE_ENTRY_COUNT; i++)
        {
            if (p_scene_state->scene_register[i].scene_number != 0x0000)
            {
                scene_number[valid_scene] = p_scene_state->scene_register[i].scene_number;
                valid_scene++;
            }
        }
        pt_tlv = pvPortMalloc(sizeof(mesh_tlv_t) + sizeof(mesh_app_mdl_evt_msg_req_t) +
                              sizeof(scene_register_status_msg_t) + (valid_scene * sizeof(uint16_t)));
        if (pt_tlv == NULL)
        {
            break;
        }

        pt_tlv->type = TYPE_MESH_APP_MDL_EVT_MSG_REQ;
        pt_tlv->length = sizeof(mesh_app_mdl_evt_msg_req_t) + sizeof(scene_register_status_msg_t) + (valid_scene * sizeof(uint16_t));

        pt_req = (mesh_app_mdl_evt_msg_req_t *)pt_tlv->value;

        pt_req->dst_addr = p_evt_msg->src_addr;
        pt_req->src_addr = p_element->element_address;;
        pt_req->appkey_index = p_evt_msg->appkey_index;
        pt_req->ttl = ttl;
        pt_req->opcode_invalid = 0;

        pt_req->opcode = BE2LE16(MMDL_SCENE_REGISTER_STATUS_OPCODE);
        pt_req->parameter_len = sizeof(scene_register_status_msg_t) + (valid_scene * sizeof(uint16_t));
        p_scene_reg_status_msg = (scene_register_status_msg_t *)pt_req->parameter;
        p_scene_reg_status_msg->status_code = status_code;
        p_scene_reg_status_msg->current_scene = p_scene_state->current_scene;
        memcpy((uint8_t *)p_scene_reg_status_msg->scene_list, (uint8_t *)scene_number, sizeof(uint16_t)*valid_scene);
        /* TODO: remaining time must be processed */

        if (mesh_evt_msg_sendto(pt_tlv) != 0)
        {
            info_color(LOG_RED, "Send msg to mesh stack fail\n");
            break;
        }
    } while (0);

    if (pt_tlv)
    {
        vPortFree(pt_tlv);
    }
}

