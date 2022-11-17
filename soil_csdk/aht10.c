#include "aht10.h"

i2c_inst_t *aht_i2c;

void aht_begin(i2c_inst_t *i2c)
{
    aht_i2c = i2c;
    aht_init();
}

void aht_init()
{
    uint8_t buf[] = {0xe1, 0x08, 0x00};
    i2c_write_blocking(aht_i2c, AHT10_I2CADDR_DEFAULT, buf, 3, false);
}

void aht_rst()
{
    uint8_t buf[] = {AHT10_CMD_SOFTRESET};
    i2c_write_blocking(aht_i2c, AHT10_I2CADDR_DEFAULT, buf, 1, false);
}

void aht_measure()
{
    uint8_t buf[] = {0xac, 0x33, 0x00};
    i2c_write_blocking(aht_i2c, AHT10_I2CADDR_DEFAULT, buf, 3, false);
}

void aht_read(double *temp, double *humi)
{
    aht_measure();
    sleep_ms(80);

    uint8_t buf[] = {0x71};
    i2c_write_blocking(aht_i2c, AHT10_I2CADDR_DEFAULT, buf, 1, false);

    uint8_t data[6];
    i2c_read_blocking(aht_i2c, AHT10_I2CADDR_DEFAULT, data, 6, false);

    double humi_bin = data[1] * 256 * 16 + data[2] * 16 + (data[3] & 0xf0) / 16;
    double temp_bin = (data[3] & 0x0f) * 256 * 256 + data[4] * 256 + data[5];

    *humi = humi_bin * 100.0 / (1024 * 1024);
    *temp = temp_bin * 200.0 / (1024 * 1024) - 50;

    // printf("%02x,%02x,%02x,%02x,%02x,%02x\n", data[0], data[1], data[2], data[3], data[4], data[5]);

    // printf("Temperature:%lf C\tHumidity:%lf %\n", *temp, *humi);
}