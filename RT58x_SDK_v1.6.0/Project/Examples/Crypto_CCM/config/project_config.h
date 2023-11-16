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


#define SET_SYS_CLK    SYS_CLK_64MHZ

/*
 * If system support multi-tasking, some hardware need mutex to protect
 */
#define SUPPORT_MULTITASKING               0

#if (CHIP_VERSION >= RT58X_MPA)

#elif (CHIP_VERSION == RT58X_SHUTTLE)

/* This define is ASIC version switch, it will be removed in production IC finially.
 * in production IC, the SPI will use native DMA design.
 * For shuttle ASIC, it should define SPI_USE_PDMA    1
 * For product ASIC in future, it should remove the following define.
 */

#define SPI_USE_PDMA                      1

/* This define is temp used, it will be removed in production IC finially.
 * in production IC, the uart will use native DMA.
 * For shuttle ASIC, it should define UART_USE_PDMA    1
 * For product ASIC in future, it should remove the following define.
 */
#define UART_USE_PDMA                     1

#endif

/*enable this option, UART1 will support CTS/RTS pin for flow control*/
#define SUPPORT_UART1_FLOWCNTL             0


/*Support AES  */
#define CRYPTO_AES_ENABLE                  1

/*Support ECC SECP192R1/P192 curve */
#define CRYPTO_SECP192R1_ENABLE            1

/*Support ECC SECP256R1/P256 curve */
#define CRYPTO_SECP256R1_ENABLE            1

/*Support ECC SECT163R2/B163 curve */
#define CRYPTO_SECT163R2_ENABLE            1



#if (CHIP_VERSION >= RT58X_MPA)
/*C25519 only supported by MP IC*/
#define CRYPTO_C25519_ENABLE               1

#endif

#endif
