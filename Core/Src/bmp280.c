#include "main.h"
#include "bmp280.h"


uint8_t Read8(BMP280_t *bmp, uint8_t reg) {
    uint8_t val;
    HAL_I2C_Mem_Read(bmp->bmpI2c, ((bmp->address)<<1), reg, 1, &val, 1, BMP280_I2C_TIMEOUT);
    return val;
}

uint16_t Read16(BMP280_t *bmp, uint8_t reg) {
    uint8_t val[2];
    HAL_I2C_Mem_Read(bmp->bmpI2c, ((bmp->address)<<1), reg, 1, val, 2, BMP280_I2C_TIMEOUT);
    // LSB|MSB
    return ((val[1]) << 8) | val[0];
}

uint32_t Read24(BMP280_t *bmp, uint8_t reg) {
    uint8_t val[3];
    HAL_I2C_Mem_Read(bmp->bmpI2c, ((bmp->address)<<1), reg, 1, val, 3, BMP280_I2C_TIMEOUT);
    return val[0] << 16 | val[1] << 8 | val [2];
}   

void Write8(BMP280_t *bmp, uint8_t reg, uint8_t val) {

    HAL_I2C_Mem_Write(bmp->bmpI2c, ((bmp->address)<<1), reg, 1, &val, 1, BMP280_I2C_TIMEOUT);
}

void BMP280_SetMode(BMP280_t *bmp, uint8_t mode) {
    uint8_t tmpMode;
    if (mode > 3) mode = 3;

    tmpMode = Read8(bmp, BMP280_CONTROL);

    tmpMode = tmpMode & 0xFC; // xxxx xx00
    tmpMode |= mode;

    Write8(bmp, BMP280_CONTROL, tmpMode);

}

void BMP280_SetPressureOversampling(BMP280_t *bmp, uint8_t POversampling) {
    uint8_t tmpOversampling;
    if (POversampling > 5) POversampling = 5;

    tmpOversampling = Read8(bmp, BMP280_CONTROL);

    tmpOversampling = tmpOversampling & 0b11100011; // xxx0 00xx
    tmpOversampling |= POversampling << 2;

    Write8(bmp, BMP280_CONTROL, tmpOversampling);
}

void BMP280_SetTemperatureOversampling(BMP280_t *bmp, uint8_t TOversampling) {
    uint8_t tmpOversampling;
    if (TOversampling > 5) TOversampling = 5;

    tmpOversampling = Read8(bmp, BMP280_CONTROL);

    tmpOversampling = tmpOversampling & 0b00011111; // xxx0 00xx
    tmpOversampling |= TOversampling << 5;

    Write8(bmp, BMP280_CONTROL, tmpOversampling);

}


uint32_t BMP28_ReadTempRaw(BMP280_t *bmp) {
    uint32_t rawTemp;
    rawTemp = Read24(bmp, BMP280_TEMPDATA);
    rawTemp >>= 4;
    return rawTemp;
}

uint32_t BMP28_ReadPressRaw(BMP280_t *bmp) {
    uint32_t rawPres;
    rawPres = Read24(bmp, BMP280_PRESSUREDATA);
    rawPres >>= 4;
    return rawPres;
}

float BMP28_ReadTemperature(BMP280_t *bmp) {
    int32_t var1, var2, T;
    int32_t adc_T;

    adc_T = BMP28_ReadTempRaw(bmp);

    var1 = ((((adc_T>>3) - ((int32_t)bmp->t1<<1))) * ((int32_t)bmp->t2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t)bmp->t1)) * ((adc_T>>4) - ((int32_t)bmp->t1))) >> 12) *
    ((int32_t)bmp->t3)) >> 14;
    bmp->t_fine = var1 + var2;
    T = (bmp->t_fine * 5 + 128) >> 8;

    return (float)(T/100.0);
}

float BMP28_ReadPressure(BMP280_t *bmp) {
    int32_t var1, var2;
    uint32_t p;

    int32_t adc_P;
    float temperature;
    temperature = BMP28_ReadTemperature(bmp);
    adc_P = BMP28_ReadPressRaw(bmp);

    var1 = (((int32_t)bmp->t_fine)>>1) - (int32_t)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)bmp->p6);
    var2 = var2 + ((var1*((int32_t)bmp->p5))<<1);
    var2 = (var2>>2)+(((int32_t)bmp->p4)<<16);
    var1 = (((bmp->p3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)bmp->p2) * var1)>>1))>>18;
    var1 =((((32768+var1))*((int32_t)bmp->p1))>>15);
    if (var1 == 0)
    {
    return 0; // avoid exception caused by division by zero
    }
    p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
    if (p < 0x80000000)
    {
    p = (p << 1) / ((uint32_t)var1);
    }
    else
    {
    p = (p / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)bmp->p9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((int32_t)(p>>2)) * ((int32_t)bmp->p8))>>13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + bmp->p7) >> 4));
    return (float)p/100.0;


}

uint8_t BMP280_Init(BMP280_t *bmp, I2C_HandleTypeDef *i2c, uint8_t addr) {
    uint8_t chipID;
    bmp->address = addr;
    bmp->bmpI2c = i2c;
    chipID = Read8(bmp, BMP280_CHIPID);
    if (chipID != 0x58) {
        return 1;
    }

    bmp->t1 = Read16(bmp, BMP280_DIG_T1);
    bmp->t2 = Read16(bmp, BMP280_DIG_T2);
    bmp->t3 = Read16(bmp, BMP280_DIG_T3);

    bmp->p1 = Read16(bmp, BMP280_DIG_P1);
    bmp->p2 = Read16(bmp, BMP280_DIG_P2);
    bmp->p3 = Read16(bmp, BMP280_DIG_P3);
    bmp->p4 = Read16(bmp, BMP280_DIG_P4);
    bmp->p5 = Read16(bmp, BMP280_DIG_P5);
    bmp->p6 = Read16(bmp, BMP280_DIG_P6);
    bmp->p7 = Read16(bmp, BMP280_DIG_P7);
    bmp->p8 = Read16(bmp, BMP280_DIG_P8);
    bmp->p8 = Read16(bmp, BMP280_DIG_P9);
    BMP280_SetPressureOversampling(bmp, BMP280_ULTRAHIGHRES);
    BMP280_SetTemperatureOversampling(bmp, BMP280_TEMPERATURE_20BIT);
    BMP280_SetMode(bmp, BMP280_NORMALMODE);
    return 0;
}