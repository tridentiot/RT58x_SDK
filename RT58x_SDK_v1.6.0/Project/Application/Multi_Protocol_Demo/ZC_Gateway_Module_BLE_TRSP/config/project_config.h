/**************************************************************************//**
 * @file     project_config.h
 * @version
 * @brief   define project configuration
 *
 * @copyright
 ******************************************************************************/

#include "chip_define.h"
#include "cm3_mcu.h"

#include "crypto.h"

#ifndef ___PROJECT_CONFIG_H__
#define ___PROJECT_CONFIG_H__

// <<< Use Configuration Wizard in Context Menu >>>\n

#define MODULE_ENABLE(module)     (module > 0)

/* If system support multi-tasking, some hardware need mutex to protect*/
//<q> Multitasking
#define SUPPORT_MULTITASKING               1

//==========================================================
// <h> Driver
//==========================================================

//==========================================================
// <h> UART0
//==========================================================
/*System use UART0 */
//<q> SUPPORT_UART0
#define SUPPORT_UART0                      1

//<q> UART0_USER_HANDLE_RECV_RX
#define UART0_USER_HANDLE_RECV_RX          0
// </h>

//==========================================================
// <h> UART1
//==========================================================
/*System use UART1 */
//<q> SUPPORT_UART1
#define SUPPORT_UART1                      1

//<q> SUPPORT_UART1_TX_DMA
#define SUPPORT_UART1_TX_DMA               1

//<q> SUPPORT_UART1_RX_DMA
#define SUPPORT_UART1_RX_DMA               1

/*enable this option, UART1 will support CTS/RTS pin for flow control*/
//<q> SUPPORT_UART1_FLOWCNTL
#define SUPPORT_UART1_FLOWCNTL             0
// </h>

//==========================================================
// <h> UART2
//==========================================================
/*System use UART2 */
//<q> SUPPORT_UART2
#define SUPPORT_UART2                      1

//<q> SUPPORT_UART2_TX_DMA
#define SUPPORT_UART2_TX_DMA               1

//<q> SUPPORT_UART2_RX_DMA
#define SUPPORT_UART2_RX_DMA               1
// </h>

//<q> SUPPORT_QSPI_DMA
#define SUPPORT_QSPI_DMA                   1

//==========================================================
// <h> Crypto
//==========================================================
/*Support AES  */
//<q> CRYPTO_AES_ENABLE
#define CRYPTO_AES_ENABLE                  1

/*Support ECC SECP192R1/P192 curve */
//<q> CRYPTO_SECP192R1_ENABLE
#define CRYPTO_SECP192R1_ENABLE            1

/*Support ECC SECP256R1/P256 curve */
//<q> CRYPTO_SECP256R1_ENABLE
#define CRYPTO_SECP256R1_ENABLE            1

/*Support ECC SECT163R2/B163 curve */
//<q> CRYPTO_SECT163R2_ENABLE
#define CRYPTO_SECT163R2_ENABLE            1
// </h>
// </h>

//==========================================================
// <h> FreeRTOS
//==========================================================
//<q> SUPPORT_FREERTOS_PORT
#define SUPPORT_FREERTOS_PORT              1
// </h>

//==========================================================
// <h> CLI Console
//==========================================================
#define USE_BSP_UART_DRV                    1
#define DEBUG_CONSOLE_UART_ID               0
//#define DEBUG_CONSOLE_UART_BAUDRATE         UART_BAUDRATE_2000000

#define SUPPORT_DEBUG_CONSOLE_CLI           0
#define SUPPORT_SHELL_CMD_HISTORY           1

#define CONFIG_ENABLE_LOG_TEE               1
#define NEW_SDK_ARCH                        1

//==========================================================
// <h> BLE
//==========================================================
//<o> BLE_SUPPORT_NUM_CONN_MAX  <0-8>
//<i> The range of BLE connection links is 0 to 8.
//<i> BLE_SUPPORT_NUM_CONN_MAX is set to 0 if BLE_CONNECTION_ENABLE is disabled.
#define BLE_SUPPORT_NUM_CONN_MAX            1

//==========================================================
// <h> Zigbee
//==========================================================


//==========================================================
// <h> RF FW
//==========================================================
#define RF_FW_INCLUDE_MULTI_2P4G            TRUE

#define RF_FW_INCLUDE_BLE                   FALSE

#define RF_FW_INCLUDE_PCI                   FALSE
// </h>

//==========================================================
// <h> RF TX Power compensation
//==========================================================
//<q> RF_TX_POWER_COMP
#define RF_TX_POWER_COMP                    0
// </h>

// <<< end of configuration section >>>

#endif
