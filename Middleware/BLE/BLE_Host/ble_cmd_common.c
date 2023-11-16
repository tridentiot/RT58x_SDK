/** @file ble_cmd_common.c
 *
 * @brief Define BLE common definition, structure and functions.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <string.h>
#include "FreeRTOS.h"
#include "ble_api.h"
#include "ble_common.h"
#include "ble_printf.h"

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/

/** BLE Set PHY controller initialization function.
*/
ble_err_t ble_cmd_phy_controller_init(void)
{
    int status;
    ble_tlv_t                    *p_ble_tlv;
    ble_common_controller_info_t *ble_common_controller_info;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_common_controller_info_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_COMMON_CONTROLLER_INIT;
        p_ble_tlv->length = sizeof(ble_common_controller_info_t);
        ble_common_controller_info = (ble_common_controller_info_t *)p_ble_tlv->value;
        ble_common_controller_info->company_id = BLE_COMPANY_ID;
        ble_common_controller_info->version = BLE_STACK_VERSION;
        memcpy(ble_common_controller_info->le_event_mask, g_le_event_mask, BLE_EVENT_MASK_LENGTH);
        memcpy(ble_common_controller_info->public_addr, g_ble_default_public_addr, BLE_ADDR_LEN);

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONTROLLER_INIT> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CONTROLLER_INIT> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Read the unique code of BLE mac address and IRK
*/
ble_err_t ble_cmd_read_unique_code(ble_unique_code_format_t *p_param)
{
    ble_err_t status;
    uint8_t   *p_read_param;
    uint8_t   *p_cmp;

    status = BLE_ERR_OK;
    p_read_param = pvPortMalloc(256);

    do
    {
        if (p_read_param != NULL)
        {
            p_cmp = pvPortMalloc(sizeof(ble_unique_code_format_t));
            if (p_cmp != NULL)
            {
                flash_read_sec_register((uint32_t)p_read_param, SEC_BLE_MAC_REG);
                while (flash_check_busy());

                memset(p_cmp, 0, sizeof(ble_unique_code_format_t));
                if (memcmp(p_read_param, p_cmp, sizeof(ble_unique_code_format_t)) == 0)
                {
                    status = BLE_ERR_INVALID_PARAMETER;
                    vPortFree(p_read_param);
                    vPortFree(p_cmp);
                    break;
                }
                memset(p_cmp, 0xFF, sizeof(ble_unique_code_format_t));
                if (memcmp(p_read_param, p_cmp, sizeof(ble_unique_code_format_t)) == 0)
                {
                    status = BLE_ERR_INVALID_PARAMETER;
                    vPortFree(p_read_param);
                    vPortFree(p_cmp);
                    break;
                }
                memcpy(p_param, p_read_param, sizeof(ble_unique_code_format_t));
                vPortFree(p_read_param);
                vPortFree(p_cmp);
            }
            else
            {
                vPortFree(p_read_param);
                status = BLE_ERR_ALLOC_MEMORY_FAIL;
            }
        }
        else
        {
            status = BLE_ERR_ALLOC_MEMORY_FAIL;
        }
    } while (0);

    return status;
}

/** Read filter accept list size
*/
ble_err_t ble_cmd_read_filter_accept_list_size(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_COMMON_READ_FILTER_ACCEPT_LIST_SIZE;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_FILTER_ACCEPT_LIST_SIZE> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<READ_FILTER_ACCEPT_LIST_SIZE> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Clear filter accept list
*/
ble_err_t ble_cmd_clear_filter_accept_list(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_COMMON_CLEAR_FILTER_ACCEPT_LIST;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CLEAR_FILTER_ACCEPT_LIST> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<CLEAR_FILTER_ACCEPT_LIST> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Add device to filter accept list
*/
ble_err_t ble_cmd_add_device_to_accept_list(ble_filter_accept_list_t *p_accept_list)
{
    int status;
    ble_tlv_t *p_ble_tlv;
    ble_filter_accept_list_t *p_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_filter_accept_list_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_COMMON_ADD_DEVICE_TO_FILTER_ACCEPT_LIST;
        p_ble_tlv->length = sizeof(ble_filter_accept_list_t);
        p_param = (ble_filter_accept_list_t *)p_ble_tlv->value;

        memcpy(p_param, p_accept_list, sizeof(ble_filter_accept_list_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADD_DEVICE_TO_FILTER_ACCEPT_LIST> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ADD_DEVICE_TO_FILTER_ACCEPT_LIST> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

/** Remove device from filter accept list
*/
ble_err_t ble_cmd_remove_device_from_accept_list(ble_filter_accept_list_t *p_accept_list)
{
    int status;
    ble_tlv_t *p_ble_tlv;
    ble_filter_accept_list_t *p_param;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t) + sizeof(ble_filter_accept_list_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_COMMON_REMOVE_DEVICE_FROM_FILTER_ACCEPT_LIST;
        p_ble_tlv->length = sizeof(ble_filter_accept_list_t);
        p_param = (ble_filter_accept_list_t *)p_ble_tlv->value;

        memcpy(p_param, p_accept_list, sizeof(ble_filter_accept_list_t));

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<REMOVE_DEVICE_FROM_FILTER_ACCEPT_LIST> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<REMOVE_DEVICE_FROM_FILTER_ACCEPT_LIST> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}

ble_err_t ble_cmd_antenna_info_read(void)
{
    int status;
    ble_tlv_t *p_ble_tlv;

    status = BLE_ERR_OK;
    p_ble_tlv = pvPortMalloc(sizeof(ble_tlv_t));

    if (p_ble_tlv != NULL)
    {
        p_ble_tlv->type = TYPE_BLE_COMMON_ANTENNA_INFO_READ;
        p_ble_tlv->length = 0;

        status = ble_event_msg_sendto(p_ble_tlv);
        if (status != BLE_ERR_OK) // send to BLE stack
        {
            BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ANTENNA_INFO_READ> Send to BLE stack fail\n");
        }
        vPortFree(p_ble_tlv);
    }
    else
    {
        BLE_PRINTF(BLE_DEBUG_CMD_INFO, "<ANTENNA_INFO_READ> malloc fail\n");
        status = BLE_ERR_ALLOC_MEMORY_FAIL;
    }

    return (ble_err_t)status;
}
