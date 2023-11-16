/* ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 * This is unpublished proprietary source code of DSR Corporation
 * The copyright notice does not evidence any actual or intended
 * publication of such source code.
 *
 * ZBOSS is a registered trademark of Data Storage Research LLC d/b/a DSR
 * Corporation
 *
 * Commercial Usage
 * Licensees holding valid DSR Commercial licenses may use
 * this file in accordance with the DSR Commercial License
 * Agreement provided with the Software or, alternatively, in accordance
 * with the terms contained in a written agreement between you and
 * DSR.
 */
/* PURPOSE:
*/

#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26

#include "zboss_api.h"
#include "radio.h"

extern rfb_zb_ctrl_t *rfb_ctrl;

void RT570_15_4_Prepare(void);
void RT570_15_4_set_auto_ack(uint8_t ack_enable);
void mac_rt570_hw_init(void);
#define ZB_TRANSCEIVER_INIT_RADIO() //JJ mac_rt570_hw_init()


#define ZB_MAC_RX_FLUSH()

void mac_rt570_handler(uint8_t InterruptStatus);


#define ZB_MAC_CLEAR_TRANS_INT()

#define ZB_MAC_GET_TRANS_INT_FLAG() 0

#define ZB_MAC_READ_INT_STATUS_REG() /* already done by int handler */




#define ZB_MAC_GET_RX_INT_STATUS_BIT()  zb_mac_get_int_status(0)

#define ZB_MAC_GET_TX_INT_STATUS_BIT() zb_mac_get_int_status(1)

#define ZB_MAC_CLEAR_RX_INT_STATUS_BIT()                        \
{                                                               \
  TRANS_CTX().int_status_0 = 0;                                 \
}

#define ZB_MAC_CLEAR_TX_INT_STATUS_BIT()                        \
{                                                               \
  TRANS_CTX().int_status_1 = 0;                                 \
  TRANS_CTX().tx_status = 0;                                    \
}

#define ZB_MAC_SET_RX_INT_STATUS_BIT() (TRANS_CTX().int_status_0 = 1)
#define ZB_MAC_SET_TX_INT_STATUS_BIT() (TRANS_CTX().int_status_1 = 1)

#define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() TRANS_CTX().tx_status

#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() 0
#define ZB_TRANS_CHECK_NO_ACK() 0

/* possibly, need further research for TI */
#define ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(addr)
#define ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(addr)
#define ZB_TRANSCEIVER_SET_PAN_COORDINATOR(pan_coord)

zb_uint32_t zb_mac_get_int_status(zb_uint8_t int_num);

void mac_rt570_clear_pending_bit(void);
void mac_rt570_set_pending_bit(void);
zb_bool_t mac_rt570_pending_bit(void);

#define ZB_MAC_TRANS_CLEAR_PENDING_BIT()  mac_rt570_clear_pending_bit()
#define ZB_MAC_TRANS_SET_PENDING_BIT()    mac_rt570_set_pending_bit()
#define ZB_MAC_TRANS_PENDING_BIT()        mac_rt570_pending_bit()


void zb_transceiver_set_short_addr(zb_uint16_t addr);
#define ZB_TRANSCEIVER_UPDATE_SHORT_ADDR() zb_transceiver_set_short_addr(MAC_PIB().mac_short_address)

void zb_transceiver_set_pan_id(zb_uint16_t pan_id);
#define ZB_TRANSCEIVER_SET_PAN_ID(pan_id) zb_transceiver_set_pan_id(pan_id)

#define ZB_TRANSCEIVER_UPDATE_PAN_ID() ZB_TRANSCEIVER_SET_PAN_ID(MAC_PIB().mac_pan_id)

/* TODO: reimplement */
#define ZB_RADIO_INT_DISABLE() ZB_OSIF_GLOBAL_LOCK()
#define ZB_RADIO_INT_ENABLE()  ZB_OSIF_GLOBAL_UNLOCK()

void zb_transceiver_update_long_mac(void);
#define ZB_TRANSCEIVER_UPDATE_LONGMAC() \
  zb_transceiver_update_long_mac()

void zb_transceiver_get_rssi(zb_uint8_t *rssi_value);
#define ZB_TRANSCEIVER_GET_RSSI zb_transceiver_get_rssi
#define ZB_TRANSCEIVER_GET_ENERGY_LEVEL ZB_TRANSCEIVER_GET_RSSI


void zb_transceiver_start_get_rssi(void);
#define ZB_TRANSCEIVER_START_GET_RSSI(_scan_duration_bi) zb_transceiver_start_get_rssi();

void zb_transceiver_set_channel(zb_uint8_t channel_number);
#define ZB_TRANSCEIVER_SET_CHANNEL(page, channel_number) zb_transceiver_set_channel(channel_number)

void mac_rt570_send_packet(zb_bufid_t buf, zb_uint8_t wait_type);

#define ZB_TRANS_SEND_FRAME(header_length, buf, wait_type)              \
  (ZB_DUMP_OUTGOING_DATA(buf),                                          \
   (void)header_length, mac_rt570_send_packet((buf), (wait_type)))


#define ZB_TRANS_REPEAT_SEND_FRAME(header_length, buf, wait_type)

zb_uint8_t mac_rt570_read_packet(zb_bufid_t buf, uint16_t packet_length, uint8_t *rx_data_address, uint8_t crc_status, uint8_t rssi, uint8_t snr);


#define ZB_TRANSCEIVER_ENABLE_AUTO_ACK()

#define ZB_TRANS_CUT_SPECIFIC_HEADER(zb_buffer)

#define ZB_RX_FLUSH_TIMEOUT (60 * ZB_TIME_ONE_SECOND)



#define TX_SEND_SUCCESS              (0x00)     /**< response by HW. */
#define TX_SEND_SUCCESS_ACK_RECEIVED (0x80)     /**< response by HW.*/
#define TX_FRAME_PENDING             (0x40)     /**< response by HW.*/
#define TX_SW_QUEUE_FULL             (0xF1)     /**< response by SW, freeRTOS queue full. */

#define ZB_TRANS_GET_TX_TIMESTAMP() (0)

#define ZB_TRANSCEIVER_SET_RX_ON_OFF(_rx_on) rfb_ctrl->auto_state_set(_rx_on)
#define ZB_TRANSCEIVER_GET_RX_ON_OFF() ZB_PIB_RX_ON_WHEN_IDLE() //(1)

#define ZB_IS_TRANSPORT_BUSY() 0
#define ZB_TRANSCEIVER_DEINIT_RADIO()


