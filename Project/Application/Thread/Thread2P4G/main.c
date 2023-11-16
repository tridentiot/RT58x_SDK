#include "openthread-core-RT58x-config.h"
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/platform/logging.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/logging.h>

#include "openthread-system.h"
#include "cli/cli_config.h"
#include "common/code_utils.hpp"

#include "project_config.h"
#include "rfb.h"
#include "cm3_mcu.h"
#include "bsp.h"
#include "bsp_console.h"
#include "bsp_uart.h"
#include "util_log.h"
#include "app.h"
#include "mem_mgmt.h"

#if OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT
#define RFB_DATA_RATE FSK_300K // Supported Value: [FSK_50K; FSK_100K; FSK_150K; FSK_200K; FSK_300K]
extern void rafael_radio_subg_datarate_set(uint8_t datarate);
#endif

#define RFB_CCA_THRESHOLD 75 // Default: 75 (-75 dBm)
extern void rafael_radio_cca_threshold_set(uint8_t datarate);

otInstance *otGetInstance(void);

void _Sleep_Init()
{
    otError error;

    otLinkModeConfig config;

    config.mRxOnWhenIdle = true;
    config.mNetworkData = true;
    config.mDeviceType = true;

    error = otThreadSetLinkMode(otGetInstance(), config);

    if (error != OT_ERROR_NONE)
    {
        err("otThreadSetLinkMode failed with %d %s\r\n", error, otThreadErrorToString(error));
    }

    /* low power mode init */
    Lpm_Set_Low_Power_Level(LOW_POWER_LEVEL_SLEEP0);
    Lpm_Enable_Low_Power_Wakeup((LOW_POWER_WAKEUP_32K_TIMER | LOW_POWER_WAKEUP_UART0_RX));
}

void otTaskletsSignalPending(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
}

void UdpReceiveCallBack(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    OT_UNUSED_VARIABLE(aContext);

    char buf[1500];
    int length;
    char string[OT_IP6_ADDRESS_STRING_SIZE];

    otIp6AddressToString(&aMessageInfo->mPeerAddr, string, sizeof(string));
    length = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    info("%d bytes from \n", length);
    info("ip : %s\n", string);
    info("port : %d \n", aMessageInfo->mSockPort);

    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';

    info("Message Received : ");
    for (int i = 0; i < length; i++)
    {
        info("%02x", buf[i]);
    }
    info("\n");

    if (memcmp(&buf, "ACK_", sizeof(char) * 4) != 0)
    {
        uint8_t *tmp_buf = NULL;
        tmp_buf = mem_malloc(length + 4);
        if (tmp_buf)
        {
            memset(tmp_buf, 0x0, length + 4);
            memcpy((char *)tmp_buf, "ACK_", sizeof(char) * 4);
            memcpy(&tmp_buf[4], buf, length);
            _Udp_Data_Send(aMessageInfo->mSockPort, aMessageInfo->mPeerAddr, tmp_buf, (length + 4));
            if (tmp_buf)
            {
                mem_free(tmp_buf);
            }
        }
    }
}

int main(int argc, char *argv[])
{
#if OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT
    rafael_radio_subg_datarate_set(RFB_DATA_RATE);
#endif
    rafael_radio_cca_threshold_set(RFB_CCA_THRESHOLD);

    otSysInit(argc, argv);

    info("2P4G Thread Init ability FTD \n");

    _app_init();

    while (!otSysPseudoResetWasRequested())
    {
        _app_process_action();
    }

    _app_exit();

    return 0;
}
