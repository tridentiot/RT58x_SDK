
#ifndef __APP_H__
#define __APP_H__

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
typedef struct
{
    uint8_t br_mac[8];
    uint8_t br_networkkey[16];
    uint8_t br_channel;
    uint16_t br_panid;
} __attribute__((packed)) app_commission_data_t;

typedef struct
{
    uint8_t started;
    app_commission_data_t data;
} __attribute__((packed)) app_commission_t;

typedef struct
{
    uint8_t mask;
    uint16_t lens;
    app_commission_data_t data;
    uint8_t crc;
    uint8_t mask_end;
} __attribute__((packed)) ble_commission_data_t;
//=============================================================================
//                Public Function Declaration
//=============================================================================

void app_commission_start();
void app_commission_erase();
void app_commission_data_write(app_commission_data_t *a_commission_data);
bool app_commission_data_check();
void app_commission_get(app_commission_t *a_commission);

#ifdef __cplusplus
};
#endif
#endif //__APP_H__