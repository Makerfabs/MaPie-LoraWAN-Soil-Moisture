#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/sleep.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"

#include "aht10.h"

void device_gpio_init();
void blink(uint pin, uint num, uint time);
void soil_read();
void soil_pwm_set();
void bat_read();

void at_init();
void uart_read_to_timeout(uint64_t timeout_ms);
void send_AT(const char *cmd, uint64_t timeout_ms);

void i2c_scan();

static void sleep_callback(void);
static void my_rtc_set(void);
static void my_rtc_sleep(int8_t minute_to_sleep_to, int8_t second_to_sleep_to);
void reset_all_clock(uint scb_orig, uint clock0_orig, uint clock1_orig);
void print_clock(void);
void print_rtc(void);
void sleep_task(void);

// For RTC
#define DEF_YEAR 2022
#define DEF_MONTH 1
#define DEF_DAY 1
#define DEF_DOT 0
#define DEF_HOUR 0
#define DEF_MIN 0
#define DEF_SEC 0