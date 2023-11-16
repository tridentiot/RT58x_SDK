/** @file ble_cmd_privacy.c
 *
 * @brief Define BLE Privacy command definition, structure and functions.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <string.h>
#include "FreeRTOS.h"
#include "ble_api.h"
#include "ble_privacy.h"
#include "ble_printf.h"

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/

/** Set Privacy enable & privacy mode
 *
*/
ble_err_t ble_cmd_privacy_enable(ble_set_privacy_cfg_t *p_param)
{
    int status;
    ble_tlv_t      *p_ble_tlv;
    ble_set_privacy_cfg_t *p_privacy_cfg;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_set_privacy_cfg_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_PRIVACY_ENABLE;
        p_ble_tlv->length = sizeof(ble_set_privacy_cfg_t);
        p_privacy_cfg = (ble_set_privacy_cfg_t *)p_ble_tlv->value;
        p_privacy_cfg->host_id = p_param->host_id;
        p_privacy_cfg->privacy_mode = p_param->privacy_mode;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PRIVACY_ENABLE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PRIVACY_ENABLE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Set Privacy disable
 *
*/
ble_err_t ble_cmd_privacy_disable(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_PRIVACY_DISABLE;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PRIVACY_DISABLE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<PRIVACY_DISABLE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}
