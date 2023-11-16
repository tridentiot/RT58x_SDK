#include "chip_define.h"
#include "cm3_mcu.h"


#define MODULE_ENABLE(module) (module > 0)

/*System use UART0 */
#define SUPPORT_UART0                      1

/*System use UART1 */
#define SUPPORT_UART1                      1
#define SUPPORT_UART1_TX_DMA               1
#define SUPPORT_UART1_RX_DMA               1

#define SUPPORT_QSPI_DMA                   1


#define USE_BSP_UART_DRV                    1
#define DEBUG_CONSOLE_UART_ID               0

#define SET_SYS_CLK    SYS_CLK_64MHZ
#define RF_FW_INCLUDE_PCI           (TRUE)
#define RF_FW_INCLUDE_BLE           (FALSE)
#define RF_FW_INCLUDE_MULTI_2P4G    (FALSE)

#define PLAFFORM_CONFIG_ENABLE_SUBG (FALSE)

#if PLAFFORM_CONFIG_ENABLE_SUBG == TRUE
#define RFB_SUBG_ENABLED            (TRUE)
#else
#define RFB_ZIGBEE_ENABLED          (TRUE)
#endif

#if RFB_SUBG_ENABLED
#define RFB_15p4_MAC_ENABLED         (TRUE)
#endif

#define __reloc __attribute__ ((used, section("reloc_text")))