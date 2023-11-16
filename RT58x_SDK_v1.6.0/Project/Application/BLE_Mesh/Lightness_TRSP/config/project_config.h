/**************************************************************************//**
 * @file     project_config.h
 * @version
 * @brief   define project config
 *
 * @copyright
 ******************************************************************************/

#include "chip_define.h"
#include "cm3_mcu.h"

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
#define UART0_USER_HANDLE_RECV_RX          1
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
// <h> Log Setting
//==========================================================
//<q> CONFIG_ENABLE_LOG
#define CONFIG_ENABLE_LOG_TEE               1

// </h>
#endif

//==========================================================
// <h> CLI Console
//==========================================================
#define DEBUG_CONSOLE_UART_ID               0
#define DEBUG_CONSOLE_UART_BAUDRATE         UART_BAUDRATE_115200


//<q> SUPPORT_DEBUG_CONSOLE_CLI
#define SUPPORT_DEBUG_CONSOLE_CLI           0

//<q> SUPPORT_SHELL_CMD_HISTORY
#define SUPPORT_SHELL_CMD_HISTORY           0
// </h>

//==========================================================
// <h> BLE
//==========================================================
//<q> BLE_ADVERTISING_ENABLE
#define BLE_ADVERTISING_ENABLE              1

//<q> BLE_SCAN_ENABLE
#define BLE_SCAN_ENABLE                     1

//<q> BLE_CONNECTION_ENABLE (enable connection related functions)
#define BLE_CONNECTION_ENABLE               1

//<q> BLE_SM_ENABLE  (enable security manager)
#define BLE_SM_ENABLE                       1

//<o> BLE_SUPPORT_NUM_CONN_MAX  <1-8>
#define BLE_SUPPORT_NUM_CONN_MAX            3

//==========================================================
// <h> RF FW
//==========================================================
#define RF_FW_INCLUDE_MULTI_2P4G           FALSE

#define RF_FW_INCLUDE_BLE                  TRUE

#define RF_FW_INCLUDE_PCI                  FALSE

//==========================================================
// <h> RF TX Power compensation
//==========================================================
//<q> RF_TX_POWER_COMP
#define RF_TX_POWER_COMP                    0

#define UART_CACHE_SIZE                     512


// </h>
