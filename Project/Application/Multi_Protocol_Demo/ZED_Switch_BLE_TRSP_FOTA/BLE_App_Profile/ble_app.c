/** @file user.c
 *
 * @brief BLE TRSP & FOTA peripheral role demo.
 *
 */

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "lpm.h"
#include "ble_event.h"
#include "ble_profile.h"
#include "multi_app.h"
#include "ble_app.h"
#include "ble_fota.h"
#include "zigbee_app.h"
/**************************************************************************************************
 *    CONSTANTS AND DEFINES
 *************************************************************************************************/
// MTU size
#define DEFAULT_MTU                     BLE_GATT_ATT_MTU_MAX
#define UART_BUFF_DEPTH                 2

// Advertising device name
#define DEVICE_NAME                     'E', 'D', '_', 'M', 'U', 'T', 'I', '_', 'T', 'R', 'S', 'P','_', 'F', 'O', 'T', 'A'

// Advertising parameters
#define APP_ADV_INTERVAL_MIN            160U      // 160*0.625ms=100ms
#define APP_ADV_INTERVAL_MAX            160U      // 160*0.625ms=100ms


#define BUTTON_1_STR            "Button1"
#define BUTTON_2_STR            "Button2"
#define BUTTON_3_STR            "Button3"
#define BUTTON_4_STR            "Button4"

// GAP device name
static const uint8_t         DEVICE_NAME_STR[] = {DEVICE_NAME};

// Device BLE Address
static const ble_gap_addr_t  DEVICE_ADDR = {.addr_type = RANDOM_STATIC_ADDR,
                                            .addr = {0x91, 0x92, 0x93, 0x94, 0x95, 0xC6 }
                                           };

/**************************************************************************************************
 *    GLOBAL VARIABLES
 *************************************************************************************************/

SemaphoreHandle_t semaphore_cb;
SemaphoreHandle_t semaphore_isr;
SemaphoreHandle_t semaphore_app;
TimerHandle_t  g_fota_timer;

/**************************************************************************************************
 *    LOCAL VARIABLES
 *************************************************************************************************/

static uint8_t g_advertising_host_id = BLE_HOSTID_RESERVED;

/**************************************************************************************************
 *    FUNCTION DECLARATION
 *************************************************************************************************/
static void svcs_trsps_data_init(ble_svcs_trsps_data_t *p_data);
static void svcs_gatts_data_init(ble_svcs_gatts_data_t *p_data);
static void svcs_fota_data_init(ble_svcs_fotas_data_t *p_data);
static ble_err_t adv_init(void);
static ble_err_t adv_enable(uint8_t host_id);
static bool fota_sw_timer_start(void);

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/
/* ------------------------------
 *  Handler
 * ------------------------------
 */
static void ble_svcs_trsps_evt_handler(ble_evt_att_param_t *p_param)
{
    if (p_param->gatt_role == BLE_GATT_ROLE_SERVER)
    {
        /* ----------------- Handle event from client ----------------- */
        switch (p_param->event)
        {
        case BLESERVICE_TRSPS_UDATRW01_WRITE_EVENT:
        case BLESERVICE_TRSPS_UDATRW01_WRITE_WITHOUT_RSP_EVENT:
            if (strncmp((char *)p_param->data, BUTTON_1_STR, sizeof(BUTTON_1_STR) - 1) == 0)
            {
                zigbee_app_evt_change(APP_LED_TOGGLE_EVT, false);
            }
            else if (strncmp((char *)p_param->data, BUTTON_2_STR, sizeof(BUTTON_2_STR) - 1) == 0)
            {
                zigbee_app_evt_change(APP_LED_LEVEL_UP_EVT, false);
            }
            else if (strncmp((char *)p_param->data, BUTTON_3_STR, sizeof(BUTTON_3_STR) - 1) == 0)
            {
                zigbee_app_evt_change(APP_LED_LEVEL_DOWN_EVT, false);
            }
            else if (strncmp((char *)p_param->data, BUTTON_4_STR, sizeof(BUTTON_4_STR) - 1) == 0)
            {
                zigbee_app_evt_change(APP_LED_MOVE_TEMP_EVT, false);
            }
            p_param->data[p_param->length] = '\0';
            info_color(LOG_CYAN, "%s\n", p_param->data);
            break;

        case BLESERVICE_TRSPS_UDATRW01_READ_EVENT:
        {
            ble_err_t status;
            const uint8_t readData[] = "UDATRW01 data";
            ble_gatt_data_param_t gatt_data_param;

            gatt_data_param.host_id = p_param->host_id;
            gatt_data_param.handle_num = p_param->handle_num;
            gatt_data_param.length = SIZE_STRING(readData);
            gatt_data_param.p_data = (uint8_t *)readData;

            status = ble_svcs_data_send(TYPE_BLE_GATT_READ_RSP, &gatt_data_param);;
            if (status != BLE_ERR_OK)
            {
                info_color(LOG_RED, "ble_gatt_read_rsp status: %d\n", status);
            }
        }
        break;

        default:
            break;
        }
    }
}

static void ble_svcs_fotas_evt_handler(ble_evt_att_param_t *p_param)
{
    if (p_param->gatt_role == BLE_GATT_ROLE_SERVER)
    {
        /* ----------------- Handle event from client ----------------- */
        switch (p_param->event)
        {
        case BLESERVICE_FOTAS_DATA_WRITE_WITHOUT_RSP_EVENT:
        {
            ble_fota_data(p_param->host_id, p_param->length, p_param->data);
            fota_sw_timer_start();
        }
        break;

        case BLESERVICE_FOTAS_COMMAND_WRITE_EVENT:
        {
            ble_fota_cmd(p_param->host_id, p_param->length, p_param->data);
            fota_sw_timer_start();
        }

        default:
            break;
        }
    }
}

// TRSPS Peripheral
static void app_peripheral_handler(app_req_param_t *p_param)
{
    ble_err_t status;
    uint8_t host_id;
    ble_info_link0_t *p_profile_info;

    host_id = p_param->host_id;
    p_profile_info = (ble_info_link0_t *)ble_app_link_info[host_id].profile_info;

    switch (p_param->app_req)
    {
    case APP_REQUEST_ADV_START:
        do
        {
            // service data init
            svcs_gatts_data_init(&p_profile_info->svcs_info_gatts.server_info.data);
            svcs_trsps_data_init(&p_profile_info->svcs_info_trsps.server_info.data);
            svcs_fota_data_init(&p_profile_info->svcs_info_fotas.server_info.data);

            // set preferred MTU size and data length
            status = ble_cmd_default_mtu_size_set(host_id, DEFAULT_MTU);
            if (status != BLE_ERR_OK)
            {
                info_color(LOG_RED, "ble_cmd_default_mtu_size_set() status = %d\n", status);
                break;
            }

            // enable advertising
            status = adv_init();
            if (status != BLE_ERR_OK)
            {
                info_color(LOG_RED, "adv_init() status = %d\n", status);
                break;
            }

            status = adv_enable(host_id);
            if (status != BLE_ERR_OK)
            {
                info_color(LOG_RED, "adv_enable() status = %d\n", status);
                break;
            }
            else
            {
                g_advertising_host_id = host_id;
            }
        } while (0);
        break;


    case APP_REQUEST_FOTA_TIMER_EXPIRY:
        // handle FOTA timer expired event
        ble_fota_timerexpiry_handler(host_id);
        break;

    default:
        break;
    }
}

static void ble_evt_handler(ble_evt_param_t *p_param)
{
    switch (p_param->event)
    {
    case BLE_ADV_EVT_SET_ENABLE:
    {
        ble_evt_adv_set_adv_enable_t *p_adv_enable = (ble_evt_adv_set_adv_enable_t *)&p_param->event_param.ble_evt_adv.param.evt_set_adv_enable;

        if (p_adv_enable->status == BLE_HCI_ERR_CODE_SUCCESS)
        {
            if (p_adv_enable->adv_enabled == true)
            {
                if (g_advertising_host_id != BLE_HOSTID_RESERVED)
                {
                    ble_app_link_info[g_advertising_host_id].state = STATE_ADVERTISING;
                }
                info_color(LOG_GREEN, "Advertising...\n");
            }
            else
            {
                if (g_advertising_host_id != BLE_HOSTID_RESERVED)
                {
                    ble_app_link_info[g_advertising_host_id].state = STATE_STANDBY;
                }
                info_color(LOG_GREEN, "Idle.\n");
            }
        }
        else
        {
            info_color(LOG_RED, "Advertising enable failed.\n");
        }
    }
    break;

    case BLE_GAP_EVT_CONN_COMPLETE:
    {
        ble_evt_gap_conn_complete_t *p_conn_param = (ble_evt_gap_conn_complete_t *)&p_param->event_param.ble_evt_gap.param.evt_conn_complete;

        if (p_conn_param->status != BLE_HCI_ERR_CODE_SUCCESS)
        {
            info_color(LOG_RED, "Connect failed, error code = 0x%02x\n", p_conn_param->status);
        }
        else
        {
            ble_app_link_info[p_conn_param->host_id].state = STATE_CONNECTED;
            info_color(LOG_GREEN, "Connected, ID=%d, Connected to %02x:%02x:%02x:%02x:%02x:%02x\n",
                       p_conn_param->host_id,
                       p_conn_param->peer_addr.addr[5],
                       p_conn_param->peer_addr.addr[4],
                       p_conn_param->peer_addr.addr[3],
                       p_conn_param->peer_addr.addr[2],
                       p_conn_param->peer_addr.addr[1],
                       p_conn_param->peer_addr.addr[0]);
        }
    }
    break;

    case BLE_GAP_EVT_CONN_PARAM_UPDATE:
    {
        ble_evt_gap_conn_param_update_t *p_conn_param = (ble_evt_gap_conn_param_update_t *)&p_param->event_param.ble_evt_gap.param.evt_conn_param_update;

        if (p_conn_param->status != BLE_HCI_ERR_CODE_SUCCESS)
        {
            info_color(LOG_RED, "Connection update failed, error code = 0x%02x\n", p_conn_param->status);
        }
        else
        {
            info_color(LOG_DEFAULT, "Connection updated\n");
            info_color(LOG_DEFAULT, "ID: %d, ", p_conn_param->host_id);
            info_color(LOG_DEFAULT, "Interval: %d, ", p_conn_param->conn_interval);
            info_color(LOG_DEFAULT, "Latency: %d, ", p_conn_param->periph_latency);
            info_color(LOG_DEFAULT, "Supervision Timeout: %d\n", p_conn_param->supv_timeout);
        }
    }
    break;

    case BLE_GAP_EVT_PHY_READ:
    case BLE_GAP_EVT_PHY_UPDATE:
    {
        ble_evt_gap_phy_t *p_phy_param = (ble_evt_gap_phy_t *)&p_param->event_param.ble_evt_gap.param.evt_phy;
        if (p_phy_param->status != BLE_HCI_ERR_CODE_SUCCESS)
        {
            info_color(LOG_RED, "PHY update/read failed, error code = 0x%02x\n", p_phy_param->status);
        }
        else
        {
            info_color(LOG_DEFAULT, "PHY updated/read, ID: %d, TX PHY: %d, RX PHY: %d\n", p_phy_param->host_id, p_phy_param->tx_phy, p_phy_param->rx_phy);
        }
    }
    break;

    case BLE_ATT_GATT_EVT_MTU_EXCHANGE:
    {
        ble_evt_mtu_t *p_mtu_param = (ble_evt_mtu_t *)&p_param->event_param.ble_evt_att_gatt.param.ble_evt_mtu;

        info_color(LOG_DEFAULT, "MTU Exchanged, ID:%d, size: %d\n", p_mtu_param->host_id, p_mtu_param->mtu);
    }
    break;

    case BLE_ATT_GATT_EVT_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH:
    {
        ble_evt_suggest_data_length_set_t *p_data_len_param = (ble_evt_suggest_data_length_set_t *)&p_param->event_param.ble_evt_att_gatt.param.ble_evt_suggest_data_length_set;

        if (p_data_len_param->status == BLE_HCI_ERR_CODE_SUCCESS)
        {
            info_color(LOG_DEFAULT, "Write default data length, status: %d\n", p_data_len_param->status);
        }
        else
        {
            info_color(LOG_RED, "Write default data length, status: %d\n", p_data_len_param->status);
        }
    }
    break;

    case BLE_ATT_GATT_EVT_DATA_LENGTH_CHANGE:
    {
        ble_evt_data_length_change_t *p_data_len_param = (ble_evt_data_length_change_t *)&p_param->event_param.ble_evt_att_gatt.param.ble_evt_data_length_change;
        info_color(LOG_DEFAULT, "Data length changed, ID: %d\n", p_data_len_param->host_id);
        info_color(LOG_DEFAULT, "MaxTxOctets: %d  MaxTxTime:%d\n", p_data_len_param->max_tx_octets, p_data_len_param->max_tx_time);
        info_color(LOG_DEFAULT, "MaxRxOctets: %d  MaxRxTime:%d\n", p_data_len_param->max_rx_octets, p_data_len_param->max_rx_time);
    }
    break;

    case BLE_GAP_EVT_DISCONN_COMPLETE:
    {
        ble_evt_gap_disconn_complete_t *p_disconn_param = (ble_evt_gap_disconn_complete_t *)&p_param->event_param.ble_evt_gap.param.evt_disconn_complete;
        if (p_disconn_param->status != BLE_HCI_ERR_CODE_SUCCESS)
        {
            info_color(LOG_RED, "Disconnect failed, error code = 0x%02x\n", p_disconn_param->status);
        }
        else
        {
            ble_fota_disconnect();
            ble_app_link_info[p_disconn_param->host_id].state = STATE_STANDBY;

            // re-start adv
            if (app_request_set(p_disconn_param->host_id, APP_REQUEST_ADV_START, false) == false)
            {
                // No Application queue buffer. Error.
            }

            info_color(LOG_GREEN, "Disconnect, ID:%d, Reason:0x%02x\n", p_disconn_param->host_id, p_disconn_param->reason);
        }
    }
    break;

    default:
        break;
    }
}

/* ------------------------------
 *  Methods
 * ------------------------------
 */
static bool fota_sw_timer_start(void)
{
    if ( xTimerIsTimerActive( g_fota_timer ) == pdFALSE )
    {
        if ( xTimerStart( g_fota_timer, 0 ) != pdTRUE )
        {
            // The timer could not be set into the Active state.
            return false;
        }
    }
    return true;
}

static ble_err_t adv_init(void)
{
    ble_err_t status;
    ble_adv_param_t adv_param;
    ble_adv_data_param_t adv_data_param;
    ble_adv_data_param_t adv_scan_data_param;
    ble_gap_addr_t addr_param;
    const uint8_t   SCANRSP_ADLENGTH  = (1) + sizeof(DEVICE_NAME_STR); //  1 byte data type

    // adv data
    uint8_t adv_data[] =
    {
        0x02, GAP_AD_TYPE_FLAGS, BLE_GAP_FLAGS_LIMITED_DISCOVERABLE_MODE,
    };

    // scan response data
    uint8_t adv_scan_rsp_data[] =
    {
        SCANRSP_ADLENGTH,                   // AD length
        GAP_AD_TYPE_LOCAL_NAME_COMPLETE,    // AD data type
        DEVICE_NAME,                        // the name is shown on scan list
    };

    ble_cmd_device_addr_get(&addr_param);
    do
    {
        adv_param.adv_type = ADV_TYPE_ADV_IND;
        adv_param.own_addr_type = addr_param.addr_type;
        adv_param.adv_interval_min = APP_ADV_INTERVAL_MIN;
        adv_param.adv_interval_max = APP_ADV_INTERVAL_MAX;
        adv_param.adv_channel_map = ADV_CHANNEL_ALL;
        adv_param.adv_filter_policy = ADV_FILTER_POLICY_ACCEPT_ALL;

        // set adv parameter
        status = ble_cmd_adv_param_set(&adv_param);
        if (status != BLE_ERR_OK)
        {
            info_color(LOG_RED, "ble_cmd_adv_param_set() status = %d\n", status);
            break;
        }

        // set adv data
        adv_data_param.length = sizeof(adv_data);
        memcpy(&adv_data_param.data, &adv_data, sizeof(adv_data));
        status = ble_cmd_adv_data_set(&adv_data_param);
        if (status != BLE_ERR_OK)
        {
            info_color(LOG_RED, "ble_cmd_adv_data_set() status = %d\n", status);
            break;
        }

        // set scan rsp data
        adv_scan_data_param.length = sizeof(adv_scan_rsp_data);
        memcpy(&adv_scan_data_param.data, &adv_scan_rsp_data, sizeof(adv_scan_rsp_data));
        status = ble_cmd_adv_scan_rsp_set(&adv_scan_data_param);
        if (status != BLE_ERR_OK)
        {
            info_color(LOG_RED, "ble_cmd_adv_scan_rsp_set() status = %d\n", status);
            break;
        }
    } while (0);

    return status;
}

static ble_err_t adv_enable(uint8_t host_id)
{
    ble_err_t status;

    status = ble_cmd_adv_enable(host_id);
    if (status != BLE_ERR_OK)
    {
        info_color(LOG_RED, "adv_enable() status = %d\n", status);
    }

    return status;
}

/* ------------------------------
 *  Application Initializations
 * ------------------------------
 */
static void svcs_gatts_data_init(ble_svcs_gatts_data_t *p_data)
{
    p_data->service_changed_cccd = 0;
}

static void svcs_trsps_data_init(ble_svcs_trsps_data_t *p_data)
{
    p_data->udatni01_cccd = 0;
}

static void svcs_fota_data_init(ble_svcs_fotas_data_t *p_data)
{
    p_data->command_cccd = 0;
    p_data->data_cccd = 0;
}

static ble_err_t server_profile_init(uint8_t host_id)
{
    ble_err_t status = BLE_ERR_OK;
    ble_info_link0_t *p_profile_info = (ble_info_link0_t *)ble_app_link_info[host_id].profile_info;

    // set link's state
    ble_app_link_info[host_id].state = STATE_STANDBY;

    do
    {
        // GAP Related
        // -------------------------------------
        status = ble_svcs_gaps_init(host_id, BLE_GATT_ROLE_SERVER, &(p_profile_info->svcs_info_gaps), NULL);
        if (status != BLE_ERR_OK)
        {
            break;
        }

        // set GAP device name
        status = ble_svcs_gaps_device_name_set((uint8_t *)DEVICE_NAME_STR, sizeof(DEVICE_NAME_STR));
        if (status != BLE_ERR_OK)
        {
            break;
        }

        // GATT Related
        // -------------------------------------
        status = ble_svcs_gatts_init(host_id, BLE_GATT_ROLE_SERVER, &(p_profile_info->svcs_info_gatts), NULL);
        if (status != BLE_ERR_OK)
        {
            break;
        }

        // DIS Related
        // -------------------------------------
        status = ble_svcs_dis_init(host_id, BLE_GATT_ROLE_SERVER, &(p_profile_info->svcs_info_dis), NULL);
        if (status != BLE_ERR_OK)
        {
            break;
        }

        // TRSPS Related
        // -------------------------------------
        status = ble_svcs_trsps_init(host_id, BLE_GATT_ROLE_SERVER, &(p_profile_info->svcs_info_trsps), ble_svcs_trsps_evt_handler);
        if (status != BLE_ERR_OK)
        {
            break;
        }

        // FOTAS Related
        // -------------------------------------
        status = ble_svcs_fotas_init(host_id, BLE_GATT_ROLE_SERVER, &(p_profile_info->svcs_info_fotas), ble_svcs_fotas_evt_handler);
        if (status != BLE_ERR_OK)
        {
            return status;
        }

    } while (0);

    return status;
}


/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/
void ble_app_evt_indication_cb(uint32_t data_len)
{
    int i32_err;
    uint8_t *p_buf;
    multi_app_queue_t p_app_q;

    if (xSemaphoreTake(semaphore_cb, 0) == pdPASS)
    {
        do
        {
            p_buf = pvPortMalloc(data_len);
            if (!p_buf)
            {
                xSemaphoreGive(semaphore_cb);
                break;
            }

            p_app_q.event = 0;
            i32_err = ble_event_msg_recvfrom(p_buf, &data_len);
            p_app_q.param_type = QUEUE_TYPE_BLE;
            p_app_q.param.pt_ble_tlv = (ble_tlv_t *)p_buf;

            if (i32_err == 0)
            {
                while (xQueueSendToBack(g_app_msg_q, &p_app_q, 1) != pdTRUE);
            }
            else
            {
                info_color(LOG_RED, "[%s] err = %d !\n", __func__, (ble_err_t)i32_err);
                vPortFree(p_buf);
                xSemaphoreGive(semaphore_cb);
            }
        } while (0);
    }
}

void ble_app_main(app_req_param_t *p_param)
{
    app_peripheral_handler(p_param);
}

void ble_event_parse(ble_tlv_t *pt_ble_tlv)
{
    if (pt_ble_tlv != NULL)
    {
        switch (pt_ble_tlv->type)
        {
        case BLE_APP_GENERAL_EVENT:
        {
            ble_evt_param_t *ble_evt = (ble_evt_param_t *)pt_ble_tlv->value;
            ble_evt_handler(ble_evt);
        }
        break;

        case BLE_APP_SERVICE_EVENT:
        {
            ble_evt_att_param_t *p_svcs_param = (ble_evt_att_param_t *)pt_ble_tlv->value;

            switch (p_svcs_param->gatt_role)
            {
            case BLE_GATT_ROLE_CLIENT:
                att_db_link[p_svcs_param->host_id].p_client_db[p_svcs_param->cb_index]->att_handler(p_svcs_param);
                break;

            case BLE_GATT_ROLE_SERVER:
                att_db_link[p_svcs_param->host_id].p_server_db[p_svcs_param->cb_index]->att_handler(p_svcs_param);
                break;

            default:
                break;
            }
        }
        break;

        default:
            break;
        }

        // free
        vPortFree(pt_ble_tlv);
    }
}

ble_err_t ble_init(void)
{
    ble_err_t status;
    ble_unique_code_format_t unique_code_param;
    ble_gap_addr_t device_addr_param;

    status = BLE_ERR_OK;

    do
    {
        status = ble_cmd_phy_controller_init();
        if (status != BLE_ERR_OK)
        {
            break;
        }

        status = ble_cmd_read_unique_code(&unique_code_param);
        if (status == BLE_ERR_OK)
        {
            device_addr_param.addr_type = unique_code_param.addr_type;
            memcpy(&device_addr_param.addr, &unique_code_param.ble_addr, 6);
            status = ble_cmd_device_addr_set((ble_gap_addr_t *)&device_addr_param);
            if (status != BLE_ERR_OK)
            {
                break;
            }
            status = ble_cmd_write_identity_resolving_key((ble_sm_irk_param_t *)&unique_code_param.ble_irk[0]);
            if (status != BLE_ERR_OK)
            {
                break;
            }
        }
        else
        {
            status = ble_cmd_device_addr_set((ble_gap_addr_t *)&DEVICE_ADDR);
            if (status != BLE_ERR_OK)
            {
                break;
            }
        }

        status = ble_cmd_resolvable_address_init();
        if (status != BLE_ERR_OK)
        {
            break;
        }

        status = ble_cmd_suggest_data_len_set(BLE_GATT_DATA_LENGTH_MAX);
        if (status != BLE_ERR_OK)
        {
            break;
        }

        // BLE profile init --> only 1 link (host id = 0)
        status = server_profile_init(APP_TRSP_P_HOST_ID);
        if (status != BLE_ERR_OK)
        {
            break;
        }
        ble_fota_fw_buffer_flash_check();
        ble_fota_init();

    } while (0);

    return status;
}

bool app_request_set(uint8_t host_id, app_request_t request, bool from_isr)
{
    multi_app_queue_t p_app_q;

    p_app_q.param_type = QUEUE_TYPE_APP_REQ;
    p_app_q.param.ble_app_req.host_id = host_id;
    p_app_q.param.ble_app_req.app_req = request;

    if (from_isr == false)
    {
        if (xSemaphoreTake(semaphore_app, 0) == pdTRUE)
        {
            p_app_q.event = APP_QUEUE_BLE_EVT; // from BLE
            if (xQueueSendToBack(g_app_msg_q, &p_app_q, 0) != pdTRUE)
            {
                // send error
                xSemaphoreGive(semaphore_app);
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        BaseType_t context_switch = pdFALSE;

        if (xSemaphoreTakeFromISR(semaphore_isr, &context_switch) == pdTRUE)
        {
            p_app_q.event = APP_QUEUE_ISR_BLE_EVT; // from BLE
            context_switch = pdFALSE;
            if (xQueueSendToBackFromISR(g_app_msg_q, &p_app_q, &context_switch) != pdTRUE)
            {
                context_switch = pdFALSE;
                xSemaphoreGiveFromISR(semaphore_isr, &context_switch);
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}


void fota_timer_handler( TimerHandle_t timer)
{
    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT( timer );

    // fota
    if (ble_app_link_info[APP_TRSP_P_HOST_ID].state == STATE_CONNECTED)
    {
        // FOTA timer tick and check if timer is expired
        if (ble_fota_timertick() == EXPIRED)
        {
            app_request_set(APP_TRSP_P_HOST_ID, APP_REQUEST_FOTA_TIMER_EXPIRY, false);
        }
    }
}
