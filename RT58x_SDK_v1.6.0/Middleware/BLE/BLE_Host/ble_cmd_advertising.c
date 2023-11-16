/** @file ble_cmd_advertising.c
 *
 * @brief Define BLE advertising command definition, structure and functions.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <string.h>
#include "FreeRTOS.h"
#include "ble_api.h"
#include "ble_advertising.h"
#include "ble_printf.h"

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/
/** Set BLE advertising parameters.
 *
 */
ble_err_t ble_cmd_adv_param_set(ble_adv_param_t *p_param)
{
    int status;
    ble_tlv_t       *p_ble_tlv;
    ble_adv_param_t *p_adv_param;

    status = BLE_ERR_OK;
    // adv parameter set
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_adv_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_ADV_PARAMETER_SET;
        p_ble_tlv->length = sizeof(ble_adv_param_t);
        p_adv_param = (ble_adv_param_t *)p_ble_tlv->value;
        p_adv_param->adv_type = p_param->adv_type;
        p_adv_param->own_addr_type = p_param->own_addr_type;
        p_adv_param->adv_interval_min = p_param->adv_interval_min;
        p_adv_param->adv_interval_max = p_param->adv_interval_max;
        p_adv_param->adv_channel_map = p_param->adv_channel_map;
        p_adv_param->adv_filter_policy = p_param->adv_filter_policy;
        memcpy(&p_adv_param->adv_peer_addr_param, &p_param->adv_peer_addr_param, sizeof(ble_gap_peer_addr_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_PARAM_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_PARAM_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set BLE advertising data.
 *
 */
ble_err_t ble_cmd_adv_data_set(ble_adv_data_param_t *p_param)
{
    int status;
    ble_tlv_t            *p_ble_tlv;
    ble_adv_data_param_t *p_adv_data;

    status = BLE_ERR_OK;
    // adv data set
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_adv_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_ADV_DATA_SET;
        p_ble_tlv->length = sizeof(ble_adv_data_param_t);
        p_adv_data = (ble_adv_data_param_t *)p_ble_tlv->value;
        p_adv_data->length = p_param->length;
        memcpy(p_adv_data->data, (uint8_t *)p_param->data, p_param->length);

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_DATA_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_DATA_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set BLE scan response data.
 *
 */
ble_err_t ble_cmd_adv_scan_rsp_set(ble_adv_data_param_t *p_param)
{
    int status;
    ble_tlv_t            *p_ble_tlv;
    ble_adv_data_param_t *p_scan_rsp;

    status = BLE_ERR_OK;
    // scan rsp data set
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_adv_data_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_ADV_SCAN_RSP_SET;
        p_ble_tlv->length = sizeof(ble_adv_data_param_t);
        p_scan_rsp = (ble_adv_data_param_t *)p_ble_tlv->value;
        p_scan_rsp->length = p_param->length;
        memcpy(p_scan_rsp->data, (uint8_t *)p_param->data, p_param->length);

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_SCAN_RSP_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_SCAN_RSP_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set BLE start advertising.
 *
 */
ble_err_t ble_cmd_adv_enable(uint8_t host_id)
{
    int status;
    ble_tlv_t              *p_ble_tlv;
    ble_adv_enable_param_t *ble_adv_enable_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_adv_enable_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_ADV_ENABLE;
        p_ble_tlv->length = sizeof(ble_adv_enable_param_t);
        ble_adv_enable_param = (ble_adv_enable_param_t *)p_ble_tlv->value;
        ble_adv_enable_param->host_id = host_id;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_ENABLE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_ENABLE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set BLE stop advertising.
 *
 */
ble_err_t ble_cmd_adv_disable(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_ADV_DISABLE;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_DISABLE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADV_DISABLE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}
