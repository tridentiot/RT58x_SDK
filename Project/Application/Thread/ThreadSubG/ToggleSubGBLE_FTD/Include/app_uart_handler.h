
#ifndef __APP_UART_H__
#define __APP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
//                Include (Better to prevent)
//=============================================================================

//=============================================================================
//                Public Definitions of const value
//=============================================================================

//=============================================================================
//                Public ENUM
//=============================================================================

//=============================================================================
//                Public Struct
//=============================================================================

//=============================================================================
//                Public Function Declaration
//=============================================================================

void app_uart_send(uint8_t *pdata, uint32_t len);
void uart1_task_start();
#ifdef __cplusplus
};
#endif
#endif //__APP_UART_H__