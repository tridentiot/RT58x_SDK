/**
 * @file rfb_comm_ble.c
 * @author
 * @date
 * @brief Apis for RFB communication subsystem and ruci format
 *
 * More detailed description can go here
 *
 *
 * @see http://
 */
/**************************************************************************************************
*    INCLUDES
*************************************************************************************************/
#include "cm3_mcu.h"
#include "ruci.h"
#include "rfb_comm.h"
#include "rfb.h"
/**************************************************************************************************
 *    MACROS
 *************************************************************************************************/



/**************************************************************************************************
 *    CONSTANTS AND DEFINES
 *************************************************************************************************/



/**************************************************************************************************
 *    TYPEDEFS
 *************************************************************************************************/

/**************************************************************************************************
 *    GLOBAL VARIABLES
 *************************************************************************************************/

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/

extern RF_MCU_RX_CMDQ_ERROR rfb_event_read(uint8_t *packet_length, uint8_t *event_address);
extern void enter_critical_section(void);
extern void leave_critical_section(void);
extern void rfb_send_cmd(uint8_t *cmd_address, uint8_t cmd_length);
/**************************************************************************************************
 *    GLOBAL FUNCTIONS
 *************************************************************************************************/
RFB_EVENT_STATUS  rfb_comm_ble_initiate(void)
{
    ruci_para_initiate_ble_t sBleInitCmd = {0};
    ruci_para_cnf_event_t sCnfEvent = {0};
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    SET_RUCI_PARA_INITIATE_BLE(&sBleInitCmd);

    RUCI_ENDIAN_CONVERT((uint8_t *)&sBleInitCmd, RUCI_INITIATE_BLE);

    enter_critical_section();
    rfb_send_cmd((uint8_t *)&sBleInitCmd, RUCI_LEN_INITIATE_BLE);
    event_status = rfb_event_read(&event_len, (uint8_t *)&sCnfEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t *)&sCnfEvent, RUCI_CNF_EVENT);
    if (event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
    {
        return RFB_CNF_EVENT_NOT_AVAILABLE;
    }
    if (sCnfEvent.pci_cmd_subheader != RUCI_CODE_INITIATE_BLE)
    {
        return RFB_CNF_EVENT_CONTENT_ERROR;
    }
    if (sCnfEvent.status != RFB_EVENT_SUCCESS)
    {
        return (RFB_EVENT_STATUS)sCnfEvent.status;
    }
    return RFB_EVENT_SUCCESS;
}

RFB_EVENT_STATUS rfb_comm_ble_modem_set(uint8_t data_rate, uint8_t coded_scheme)
{
    ruci_para_set_ble_modem_t sbleModemSetCmd = {0};
    ruci_para_cnf_event_t sCnfEvent = {0};
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    SET_RUCI_PARA_SET_BLE_MODEM(&sbleModemSetCmd, data_rate, coded_scheme);

    RUCI_ENDIAN_CONVERT((uint8_t *)&sbleModemSetCmd, RUCI_SET_BLE_MODEM);

    enter_critical_section();
    rfb_send_cmd((uint8_t *)&sbleModemSetCmd, RUCI_LEN_SET_BLE_MODEM);
    event_status = rfb_event_read(&event_len, (uint8_t *)&sCnfEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t *)&sCnfEvent, RUCI_CNF_EVENT);
    if (event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
    {
        return RFB_CNF_EVENT_NOT_AVAILABLE;
    }
    if (sCnfEvent.pci_cmd_subheader != RUCI_CODE_SET_BLE_MODEM)
    {
        return RFB_CNF_EVENT_CONTENT_ERROR;
    }
    if (sCnfEvent.status != RFB_EVENT_SUCCESS)
    {
        return (RFB_EVENT_STATUS)sCnfEvent.status;
    }
    return RFB_EVENT_SUCCESS;
}

RFB_EVENT_STATUS rfb_comm_ble_mac_set(uint32_t sfd_content, uint8_t whitening_en, uint8_t whitening_init_value, uint32_t crc_init_value)
{
    ruci_para_set_ble_mac_t sbleMacSetCmd = {0};
    ruci_para_cnf_event_t sCnfEvent = {0};
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    SET_RUCI_PARA_SET_BLE_MAC(&sbleMacSetCmd, sfd_content, whitening_en, whitening_init_value, crc_init_value);

    RUCI_ENDIAN_CONVERT((uint8_t *)&sbleMacSetCmd, RUCI_SET_BLE_MAC);

    enter_critical_section();
    rfb_send_cmd((uint8_t *)&sbleMacSetCmd, RUCI_LEN_SET_BLE_MAC);
    event_status = rfb_event_read(&event_len, (uint8_t *)&sCnfEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t *)&sCnfEvent, RUCI_CNF_EVENT);
    if (event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
    {
        return RFB_CNF_EVENT_NOT_AVAILABLE;
    }
    if (sCnfEvent.pci_cmd_subheader != RUCI_CODE_SET_BLE_MAC)
    {
        return RFB_CNF_EVENT_CONTENT_ERROR;
    }
    if (sCnfEvent.status != RFB_EVENT_SUCCESS)
    {
        return (RFB_EVENT_STATUS)sCnfEvent.status;
    }
    return RFB_EVENT_SUCCESS;
}
