/** @file ble_cmd_att_gatt.c
 *
 * @brief Define BLE stt / gatt command definition, structure and functions.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <string.h>
#include "FreeRTOS.h"
#include "ble_api.h"
#include "ble_att_gatt.h"
#include "ble_printf.h"

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/
/** Set suggested data length.
 */
ble_err_t ble_cmd_suggest_data_len_set(uint16_t tx_octets)
{
    int status;
    ble_tlv_t                           *p_ble_tlv;
    ble_gatt_suggested_data_len_param_t *p_data_len_param;

    status = BLE_ERR_OK;
    // set preferred data length
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_suggested_data_len_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_PREFERRED_DATA_LENGTH_SET;
        p_ble_tlv->length = sizeof(ble_gatt_suggested_data_len_param_t);
        p_data_len_param = (ble_gatt_suggested_data_len_param_t *)p_ble_tlv->value;
        p_data_len_param->tx_octets = tx_octets;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PREFERRED_DATA_LENGTH_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PREFERRED_DATA_LENGTH_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set preferred MTU size.
 */
ble_err_t ble_cmd_default_mtu_size_set(uint8_t host_id, uint16_t mtu)
{
    int status;
    ble_tlv_t            *p_ble_tlv;
    ble_gatt_mtu_param_t *p_mtu_param;

    status = BLE_ERR_OK;
    // set preferred MTU size
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_mtu_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_PREFERRED_MTU_SET;
        p_ble_tlv->length = sizeof(ble_gatt_mtu_param_t);
        p_mtu_param = (ble_gatt_mtu_param_t *)p_ble_tlv->value;
        p_mtu_param->host_id = host_id;
        p_mtu_param->mtu = mtu;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PREFERRED_MTU_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PREFERRED_MTU_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** ATT_MTU exchange request.
 */
ble_err_t ble_cmd_mtu_size_update(uint8_t host_id, uint16_t mtu)
{
    int status;
    ble_tlv_t            *p_ble_tlv;
    ble_gatt_mtu_param_t  *p_mtu_param;

    status = BLE_ERR_OK;
    // set preferred MTU size
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_mtu_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_EXCHANGE_MTU_REQ;
        p_ble_tlv->length = sizeof(ble_gatt_mtu_param_t);
        p_mtu_param = (ble_gatt_mtu_param_t *)p_ble_tlv->value;
        p_mtu_param->host_id = host_id;
        p_mtu_param->mtu = mtu;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<EXCHANGE_MTU_REQ> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<EXCHANGE_MTU_REQ> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set data length update.
 */
ble_err_t ble_cmd_data_len_update(uint8_t host_id, uint16_t tx_octets)
{
    int status;
    ble_tlv_t                 *p_ble_tlv;
    ble_gatt_data_len_param_t *p_data_len_param;

    status = BLE_ERR_OK;
    // set preferred data length
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_len_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_DATA_LENGTH_UPDATE;
        p_ble_tlv->length = sizeof(ble_gatt_data_len_param_t);
        p_data_len_param = (ble_gatt_data_len_param_t *)p_ble_tlv->value;
        p_data_len_param->host_id = host_id;
        p_data_len_param->tx_octets = tx_octets;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<DATA_LENGTH_UPDATE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<DATA_LENGTH_UPDATE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Get BLE GATT MTU Size
 */
ble_err_t ble_cmd_mtu_size_get(uint8_t host_id, uint16_t *mtu)
{
    int status;
    ble_tlv_t                *p_ble_tlv;
    ble_gatt_get_mtu_param_t *p_mtu_param;

    status = BLE_ERR_OK;
    // set preferred MTU size
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_get_mtu_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_MTU_GET;
        p_ble_tlv->length = sizeof(ble_gatt_get_mtu_param_t);
        p_mtu_param = (ble_gatt_get_mtu_param_t *)p_ble_tlv->value;
        p_mtu_param->host_id = host_id;
        p_mtu_param->p_mtu = mtu;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<MTU_GET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<MTU_GET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Read Response
 */
ble_err_t ble_cmd_gatt_read_rsp(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_READ_RSP;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_RSP> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_RSP> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Read By Type Response
 */
ble_err_t ble_cmd_gatt_read_by_type_rsp(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_READ_BY_TYPE_RSP;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_BY_TYPE_RSP> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_BY_TYPE_RSP> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Read Blob Response
 */
ble_err_t ble_cmd_gatt_read_blob_rsp(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_READ_BLOB_RSP;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_BLOB_RSP> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_BLOB_RSP> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Error Response
 */
ble_err_t ble_cmd_gatt_error_rsp(ble_gatt_err_rsp_param_t *p_param)
{
    int status;
    ble_tlv_t                *p_ble_tlv;
    ble_gatt_err_rsp_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_err_rsp_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_ERROR_RSP;
        p_ble_tlv->length = sizeof(ble_gatt_err_rsp_param_t);
        p_data_param = (ble_gatt_err_rsp_param_t *)p_ble_tlv->value;
        p_data_param->host_id = p_param->host_id;
        p_data_param->handle_num = p_param->handle_num;
        p_data_param->opcode = p_param->opcode;
        p_data_param->err_rsp = p_param->err_rsp;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ERROR_RSP> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ERROR_RSP> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Notification
 */
ble_err_t ble_cmd_gatt_notification(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_NOTIFICATION;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<NOTIFICATION> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<NOTIFICATION> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Indication
 */
ble_err_t ble_cmd_gatt_indication(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_INDICATION;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<INDICATION> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<INDICATION> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Write Request
 */
ble_err_t ble_cmd_gatt_write_req(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_WRITE_REQ;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<WRITE_REQ> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<WRITE_REQ> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE Write Command
 */
ble_err_t ble_cmd_gatt_write_cmd(ble_gatt_data_param_t *p_param)
{
    int status;
    ble_tlv_t             *p_ble_tlv;
    ble_gatt_data_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_WRITE_CMD;
        p_ble_tlv->length = sizeof(ble_gatt_data_param_t);
        p_data_param = (ble_gatt_data_param_t *)p_ble_tlv->value;
        memcpy(p_data_param, p_param, sizeof(ble_gatt_data_param_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<WRITE_CMD> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<WRITE_CMD> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE GATT Read Request
 */
ble_err_t ble_cmd_gatt_read_req(ble_gatt_read_req_param_t *p_param)
{
    int status;
    ble_tlv_t                 *p_ble_tlv;
    ble_gatt_read_req_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_read_req_param_t));
    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_READ_REQ;
        p_ble_tlv->length = sizeof(ble_gatt_read_req_param_t);
        p_data_param = (ble_gatt_read_req_param_t *)p_ble_tlv->value;
        p_data_param->host_id = p_param->host_id;
        p_data_param->handle_num = p_param->handle_num;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_REQ> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_REQ> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE GATT Read Long Characteristic Value
 */
ble_err_t ble_cmd_gatt_read_blob_req(ble_gatt_read_blob_req_param_t *p_param)
{
    int status;
    ble_tlv_t                      *p_ble_tlv;
    ble_gatt_read_blob_req_param_t *p_data_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_gatt_read_blob_req_param_t));
    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_GATT_READ_BLOB_REQ;
        p_ble_tlv->length = sizeof(ble_gatt_read_blob_req_param_t);
        p_data_param = (ble_gatt_read_blob_req_param_t *)p_ble_tlv->value;
        p_data_param->host_id = p_param->host_id;
        p_data_param->handle_num = p_param->handle_num;
        p_data_param->offset = p_param->offset;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_BLOB_REQ> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_BLOB_REQ> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}
