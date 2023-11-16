/**************************************************************************//**
 * @file     project_config.h
 * @version
 * @brief   define project config
 *
 * @copyright
 ******************************************************************************/

#include "chip_define.h"

#ifndef ___PROJECT_CONFIG_H__
#define ___PROJECT_CONFIG_H__


#define MODULE_ENABLE(module)     (module > 0)

/*
 * If system support multi-tasking, some hardware need mutex to protect
 */
#define SUPPORT_MULTITASKING               0

/* This define is temp used, it will be removed in production IC finially.
 * in production IC, the uart will use native DMA.
 * For shutter ASIC, it should define UART_USE_PDMA    1
 * For product ASIC in future, it should remove the following define.
 */
#define UART_USE_PDMA                      1

/*System use UART0 */
#define SUPPORT_UART0                      1

/*System use UART1 */
#define SUPPORT_UART1                      1

#define SUPPORT_UART1_TX_DMA               1

#define SUPPORT_UART1_RX_DMA               1

/*enable this option, UART1 will support CTS/RTS pin for flow control*/
#define SUPPORT_UART1_FLOWCNTL             0

/*System use UART2 */
#define SUPPORT_UART2                      1

#define SUPPORT_UART2_TX_DMA               1

#define SUPPORT_UART2_RX_DMA               1


#define SUPPORT_QSPI_DMA                   1

/*Support AES  */
#define CRYPTO_AES_ENABLE                  1

/*Support ECC SECP192R1/P192 curve */
#define CRYPTO_SECP192R1_ENABLE            1

/*Support ECC SECP256R1/P256 curve */
#define CRYPTO_SECP256R1_ENABLE            1

/*Support ECC SECT163R2/B163 curve */
#define CRYPTO_SECT163R2_ENABLE            1

#define SUPPORT_FREERTOS_PORT              1
#define SUPPORT_FREERTOS_SLEEP             0
#define SUPPORT_FREERTOS_NEST_CRITICAL     0
#define SUPPORT_FREERTOS_NEST_CRITICAL     0

/*Disable RCO 40Khz Frequency */
#define RCO40K_CALIBRATION_DISABLE         0

/*Enable external slow clock source gpio and gpio pin define*/
#define EXT32K_GPIO_ENABLE                 0
/*Config EXT32K_GPIO_X 0~7, GPIO0~GPIO7*/
#define EXT32K_GPIO_X                      7

#define FLASHCTL_FreeRTOS_ISR_ENABLE 1

#endif
