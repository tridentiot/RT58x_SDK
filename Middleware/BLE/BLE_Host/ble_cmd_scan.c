/** @file ble_cmd_scan.c
 *
 * @brief Define BLE scan command definition, structure and functions.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <string.h>
#include "FreeRTOS.h"
#include "ble_api.h"
#include "ble_scan.h"
#include "ble_printf.h"

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/
/** Set BLE scan parameters.
 *
 */
ble_err_t ble_cmd_scan_param_set(ble_scan_param_t *p_param)
{
    int status;
    ble_tlv_t        *p_ble_tlv;
    ble_scan_param_t *ble_scan_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_scan_param_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_SCAN_PARAMETER_SET;
        p_ble_tlv->length = sizeof(ble_scan_param_t);
        ble_scan_param = (ble_scan_param_t *)p_ble_tlv->value;
        ble_scan_param->scan_type = p_param->scan_type;
        ble_scan_param->own_addr_type = p_param->own_addr_type;
        ble_scan_param->scan_interval = p_param->scan_interval;
        ble_scan_param->scan_window = p_param->scan_window;
        ble_scan_param->scan_filter_policy = p_param->scan_filter_policy;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<SCAN_PARAM_SET> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<SCAN_PARAM_SET> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Enable BLE scan.
 *
 */
ble_err_t ble_cmd_scan_enable(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_SCAN_ENABLE;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<SCAN_ENABLE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<SCAN_ENABLE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

ble_err_t ble_cmd_scan_disable(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_SCAN_DISABLE;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<SCAN_DISABLE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<SCAN_DISABLE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Parsing scanned BLE device's advertising data.
 *
 */
ble_err_t ble_cmd_scan_report_adv_data_parsing(ble_evt_scan_adv_report_t *p_rpt_data,
        ble_gap_ad_type_t           ad_type,
        uint8_t                     *p_data,
        uint8_t                     *p_data_length)
{
    ble_err_t status;
    uint8_t i = 0;

    status = BLE_ERR_INVALID_PARAMETER;

    do
    {
        if ((p_rpt_data->length == 0) || (p_rpt_data->length > BLE_ADV_DATA_SIZE_MAX))
        {
            status = BLE_ERR_INVALID_PARAMETER;
            break;
        }

        *p_data_length = 0;

        while (i < p_rpt_data->length)
        {
            if (p_rpt_data->data[i + 1] == ad_type)   // type = 2nd byte
            {
                *p_data_length = p_rpt_data->data[i] - 1;                        // data len = 1st byte - type len(1)
                memcpy(p_data, p_rpt_data->data + (i + 2), *p_data_length);      // data     = start from 3rd byte
                status = BLE_ERR_OK;
                break;
            }
            i = i + p_rpt_data->data[i] + 1;
        }
        if (status != BLE_ERR_OK)
        {
            status = BLE_ERR_INVALID_PARAMETER;
        }
    } while (0);

    return status;
}
