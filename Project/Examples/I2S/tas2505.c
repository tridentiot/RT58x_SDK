/**************************************************************************************************
*    INCLUDES
*************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cm3_mcu.h"
#include "project_config.h"
#include "uart_drv.h"
#include "retarget.h"


#define I2C_IO_SCL          22
#define I2C_IO_SDA          23

volatile uint8_t transfer_done;
volatile uint32_t i2c_status;

/*this is pin mux setting*/
void i2c_Pin_Config(void)
{
    pin_set_mode(I2C_IO_SCL,    MODE_I2C);
    pin_set_mode(I2C_IO_SDA,    MODE_I2C);
}

void I2c_Finish(uint32_t status)
{
    //dprintf("i2c_finish return error is %08x \n", status);
    i2c_status = status;
    transfer_done = TRUE;
}

void Tas2505_I2c_Data_Convert(uint16_t reg_addr, uint8_t data)
{
    i2c_slave_data_t dev_cfg;
    uint8_t data_arr[1];

    dev_cfg.dev_addr = (0x30 >> 0x1);     /*Device 0x47 does NOT exist.*/
    dev_cfg.bFlag_16bits = 0x00;        /*register address is 8 bit mode*/
    dev_cfg.reg_addr = reg_addr;
    data_arr[0] = data;
    i2c_write(&dev_cfg, data_arr, 1, I2c_Finish);
    transfer_done = FALSE;
    while (transfer_done == FALSE) {};
}

void Init_Dac_Board_I2c(i2s_para_set_t *i2s_para)
{
    i2c_Pin_Config(); //config GPIO22 and GPIO23 to I2C

    i2c_init(I2C_CLOCK_100K);

    // I2C Script to Setup Register to Play Digital Data Through DAC and Headphone Output
    //Page switch to Page 0
    Tas2505_I2c_Data_Convert(0x00, 0x00);
    //Assert Software reset (P0, R1, D0=1)
    Tas2505_I2c_Data_Convert(0x01, 0x01);
    //Page Switch to Page 1
    Tas2505_I2c_Data_Convert(0x00, 0x01);
    //LDO output programmed as 1.8V and Level shifters powered up. (P1, R2, D5-D4=00, D3=0)
    Tas2505_I2c_Data_Convert(0x02, 0x00);

    // Page switch to Page 0
    Tas2505_I2c_Data_Convert(0x00, 0x00);
    // CODEC_CLKIN=MCLK, PLL_IN=MCLK (P0, R4, D1-D0=00)
    Tas2505_I2c_Data_Convert(0x04, 0x00);

    // CODEC_CLKIN = NDAC * MDAC * DOSR * DAC_fS
    // MDAC * DOSR / 32 = RC
    if (i2s_para->sr == I2S_SR_48K || i2s_para->sr == I2S_SR_32K)
    {
        // DAC NDAC Powered up, NDAC=1 (P0, R11, D7=1, D6-D0=0000001)
        Tas2505_I2c_Data_Convert(0x0B, 0x81);
        // DAC MDAC Powered up, MDAC=1 (P0, R12, D7=1, D6-D0=0000001)
        Tas2505_I2c_Data_Convert(0x0C, 0x81);
        // DAC OSR(9:0)-> DOSR=128 (P0, R12, D1-D0=00)
        Tas2505_I2c_Data_Convert(0x0D, 0x00);
        // DAC OSR(9:0)-> DOSR=128 (P0, R13, D7-D0=10000000)
        Tas2505_I2c_Data_Convert(0x0E, 0x80);
    }
    else if ((i2s_para->sr == I2S_SR_16K) ||
             (i2s_para->sr == I2S_SR_8K && i2s_para->width == I2S_CFG_WID_16))
    {
        // DAC NDAC Powered up, NDAC=1 (P0, R11, D7=1, D6-D0=0000001)
        Tas2505_I2c_Data_Convert(0x0B, 0x81);
        // DAC MDAC Powered up, MDAC=2 (P0, R12, D7=1, D6-D0=0000010)
        Tas2505_I2c_Data_Convert(0x0C, 0x82);
        // DAC OSR(9:0)-> DOSR=128 (P0, R12, D1-D0=00)
        Tas2505_I2c_Data_Convert(0x0D, 0x00);
        // DAC OSR(9:0)-> DOSR=128 (P0, R13, D7-D0=01000000)
        Tas2505_I2c_Data_Convert(0x0E, 0x80);
    }
    else if (i2s_para->sr == I2S_SR_8K && i2s_para->width == I2S_CFG_WID_32)
    {
        // DAC NDAC Powered up, NDAC=1 (P0, R11, D7=1, D6-D0=0000001)
        Tas2505_I2c_Data_Convert(0x0B, 0x82);
        // DAC MDAC Powered up, MDAC=2 (P0, R12, D7=1, D6-D0=0000010)
        Tas2505_I2c_Data_Convert(0x0C, 0x82);
        // DAC OSR(9:0)-> DOSR=128 (P0, R12, D1-D0=00)
        Tas2505_I2c_Data_Convert(0x0D, 0x00);
        // DAC OSR(9:0)-> DOSR=128 (P0, R13, D7-D0=01000000)
        Tas2505_I2c_Data_Convert(0x0E, 0x80);
    }

    // Codec Interface control Word length = 16bits, BCLK&WCLK inputs, I2S mode. (P0, R27, D7-D6=00, D5-D4=00, D3-D2=00)
    Tas2505_I2c_Data_Convert(0x1B, 0x00);
    // Data slot offset 00 (P0, R28, D7-D0=0000)
    Tas2505_I2c_Data_Convert(0x1C, 0x00);
    // Dac Instruction programming PRB #2 for Mono routing. Type interpolation (x8) and 3 programmable Biquads. (P0, R60, D4-D0=0010)
    Tas2505_I2c_Data_Convert(0x3C, 02);

    // Page Switch to Page 1
    Tas2505_I2c_Data_Convert(0x00, 0x01);
    // Master Reference Powered on (P1, R1, D4=1)
    Tas2505_I2c_Data_Convert(0x01, 0x10);
    // Output common mode for DAC set to 0.9V (default) (P1, R10)
    Tas2505_I2c_Data_Convert(0x0A, 0x00);
    // DAC output is routed directly to HP driver (P1, R12, D3=1)
    Tas2505_I2c_Data_Convert(0x0C, 0x08);
    // HP Voulme, 0dB Gain (P1, R22, D6-D0=0000000)
    Tas2505_I2c_Data_Convert(0x16, 0x00);
    // Power up HP (P1, R9, D5=1)
    Tas2505_I2c_Data_Convert(0x09, 0x20);
    // Unmute HP with 0dB gain (P1, R16, D4=1)
    Tas2505_I2c_Data_Convert(0x10, 0x00);

    // Page switch to Page 0
    Tas2505_I2c_Data_Convert(0x00, 0x00);
    // DAC powered up, Soft step 1 per Fs. (P0, R63, D7=1, D5-D4=01, D3-D2=00, D1-D0=00)
    Tas2505_I2c_Data_Convert(0x3F, 0x90);   //Left Channel
    //Tas2505_I2c_Data_Convert(0x3F,0xA0);  //Right Channel
    // DAC digital gain 0dB (P0, R65, D7-D0=00000000)
    Tas2505_I2c_Data_Convert(0x41, 0x00);
    // DAC volume not muted. (P0, R64, D3=0, D2=1)
    Tas2505_I2c_Data_Convert(0x40, 0x04);
}
