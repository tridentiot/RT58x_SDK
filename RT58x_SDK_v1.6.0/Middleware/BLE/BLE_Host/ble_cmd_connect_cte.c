/** @file ble_cmd_connect_cte.c
 *
 * @brief Define BLE connection CTE command definition, structure and functions.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <string.h>
#include "FreeRTOS.h"
#include "ble_api.h"
#include "ble_connect_cte.h"
#include "ble_printf.h"

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/

/** BLE set connection CTE receive parameters
 *
 */
ble_err_t ble_cmd_connection_cte_receive_parameters_set(ble_connection_cte_rx_param_t *p_param)
{
    int status;
    ble_tlv_t *p_ble_tlv;
    ble_connection_cte_rx_param_t *p_cte_rx_param;
    uint8_t i;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_connection_cte_rx_param_t) + p_param->sw_pattern_length);

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_CTE_CONNECTION_CTE_RX_PARAMETERS_SET;
        p_ble_tlv->length = sizeof(ble_connection_cte_rx_param_t) + p_param->sw_pattern_length;
        p_cte_rx_param = (ble_connection_cte_rx_param_t *)p_ble_tlv->value;
        p_cte_rx_param->host_id = p_param->host_id;
        p_cte_rx_param->sampling_enable = p_param->sampling_enable;
        p_cte_rx_param->slot_durations = p_param->slot_durations;
        p_cte_rx_param->sw_pattern_length = p_param->sw_pattern_length;
        for (i = 0; i < p_param->sw_pattern_length; i++)
        {
            *(p_cte_rx_param->antenna_ids + i) = *(p_param->antenna_ids + i);
        }

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_RX_PARAMETERS_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_RX_PARAMETERS_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE set connection CTE transmit parameters
 *
 */
ble_err_t ble_cmd_connection_cte_transmit_parameters_set(ble_connection_cte_tx_param_t *p_param)
{
    int status;
    ble_tlv_t *p_ble_tlv;
    ble_connection_cte_tx_param_t *p_cte_tx_param;
    uint8_t i;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_connection_cte_tx_param_t) + p_param->sw_pattern_length);

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_CTE_CONNECTION_CTE_TX_PARAMETERS_SET;
        p_ble_tlv->length = sizeof(ble_connection_cte_tx_param_t) + p_param->sw_pattern_length;
        p_cte_tx_param = (ble_connection_cte_tx_param_t *)p_ble_tlv->value;
        p_cte_tx_param->host_id = p_param->host_id;
        p_cte_tx_param->cte_types = p_param->cte_types;
        p_cte_tx_param->sw_pattern_length = p_param->sw_pattern_length;
        for (i = 0; i < p_param->sw_pattern_length; i++)
        {
            *(p_cte_tx_param->antenna_ids + i) = *(p_param->antenna_ids + i);
        }

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_TX_PARAMETERS_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_TX_PARAMETERS_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE set connection request enable
 *
 */
ble_err_t ble_cmd_connection_cte_request_enable(ble_connection_cte_req_enable_t *p_param)
{
    int status;
    ble_tlv_t *p_ble_tlv;
    ble_connection_cte_req_enable_t *p_cte_req_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_connection_cte_req_enable_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_CTE_CONNECTION_CTE_REQ_SET;
        p_ble_tlv->length = sizeof(ble_connection_cte_req_enable_t);
        p_cte_req_param = (ble_connection_cte_req_enable_t *)p_ble_tlv->value;
        memcpy(&p_cte_req_param->host_id, &p_param->host_id, sizeof(ble_connection_cte_req_enable_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_REQ_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_REQ_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** BLE set connection CTE response enable
 *
 */
ble_err_t ble_cmd_connection_cte_response_enable(ble_connection_cte_rsp_enable_t *p_param)
{
    int status;
    ble_tlv_t *p_ble_tlv;
    ble_connection_cte_rsp_enable_t *p_cte_rsp_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_connection_cte_rsp_enable_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_CTE_CONNECTION_CTE_RSP_SET;
        p_ble_tlv->length = sizeof(ble_connection_cte_rsp_enable_t);
        p_cte_rsp_param = (ble_connection_cte_rsp_enable_t *)p_ble_tlv->value;
        memcpy(&p_cte_rsp_param->host_id, &p_param->host_id, sizeof(ble_connection_cte_rsp_enable_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_RSP_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONNECTION_CTE_RSP_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}
