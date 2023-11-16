#ifndef APP_H
#define APP_H

#if OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT
#define DEF_CHANNEL 6
#else
#define DEF_CHANNEL 21
#endif

#define UDP_PORT 5678

void _app_init(void);
void _app_process_action();
void _app_exit(void);

void _Udp_Data_Send(uint16_t PeerPort, otIp6Address PeerAddr, uint8_t *data, uint16_t buffer_lens);

#endif
