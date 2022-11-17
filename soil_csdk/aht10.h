#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define AHT10_I2CADDR_DEFAULT 0x38   ///< AHT10 default i2c address
#define AHT10_CMD_CALIBRATE 0xE1     ///< Calibration command
#define AHT10_CMD_TRIGGER 0xAC       ///< Trigger reading command
#define AHT10_CMD_SOFTRESET 0xBA     ///< Soft reset command
#define AHT10_STATUS_BUSY 0x80       ///< Status bit for busy
#define AHT10_STATUS_CALIBRATED 0x08 ///< Status bit for calibrated

void aht_begin(i2c_inst_t *i2c);
void aht_init();
void aht_rst();
void aht_read();
void aht_measure();
void aht_read(double *temp, double *humi);