#include "all_head.h"

#define LED_PIN 25
#define POWER_PIN 8
#define LORA_RST 20

#define PWM_PIN 9

#define BAT_PIN 29
#define BAT_ADC 3

#define SOIL_PIN 26
#define SOIL_ADC 0

#define SOIL_ADC_AIR 3018
#define SOIL_ADC_WATER 2568
#define SOIL_ADC_UNIT ((SOIL_ADC_AIR - SOIL_ADC_WATER) / 100)

#define AT_UART uart0
#define AT_UART_TX 0
#define AT_UART_RX 1
#define AT_TIMEOUT 3000

#define DEVEUI "6081F9EFE6C31D65"
#define APPEUI "6081F9D6B0D9B9F4"
#define APPKEY "9ED8865D20830BF00EDC6DFB572CD254"

#define AHT_SCL 7
#define AHT_SDA 6
#define AHT_I2C i2c1

double temp = 0.0;
double humi = 0.0;
int percent = 0;
int bat = 0;

static bool awake;

int main()
{
	stdio_init_all();

	device_gpio_init();
	soil_pwm_set();
	i2c_init(AHT_I2C, 400000);
	// i2c_scan();
	aht_begin(AHT_I2C);

	// save values for later
	uint scb_orig = scb_hw->scr;
	uint clock0_orig = clocks_hw->sleep_en0;
	uint clock1_orig = clocks_hw->sleep_en1;

	// Start the Real time clock
	rtc_init();

	blink(LED_PIN, 2, 1000);
	printf("GPIO init over\n");

	// Init Lorawan
	at_init();

	// Read Temperature, Humidity, soil adc ,bat adc

	while (1)
	{
		for (int i = 0; i < 3; i++)
		{
			aht_read(&temp, &humi);
			sleep_ms(1000);
		}
		soil_read();
		bat_read();

		printf("Temp:%.2lf\tHumi:%.2lf\tSOIL:%d\tBAT:%d\n", temp, humi, percent, bat);
		uart_read_to_timeout(10000);

		break;
	}

	// Close AHT11
	gpio_put(POWER_PIN, 0);

	char data_str[80];
	sprintf(data_str, "AT+DTRX=1,2,6,%02x%02x%02x%02x", (int)temp, (int)humi, percent, bat);
	printf("%s\n", data_str);

	// send_AT("AT+DTRX=1,2,6,FF0088", AT_TIMEOUT);
	send_AT(data_str, AT_TIMEOUT);
	uart_read_to_timeout(10000);

	// Sleep
	blink(LED_PIN, 3, 300);
	printf("Prepare to sleep\n");
	sleep_task();

	// reset processor and clocks back to defaults
	reset_all_clock(scb_orig, clock0_orig, clock1_orig);
	blink(LED_PIN, 3, 300);

	// restart
	watchdog_enable(10, 0);
	while (true)
		sleep_ms(100);
}

//------------------GPIO Function----------------------
void device_gpio_init()
{
	// Init
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	gpio_init(POWER_PIN);
	gpio_set_dir(POWER_PIN, GPIO_OUT);

	gpio_init(LORA_RST);
	gpio_set_dir(LORA_RST, GPIO_OUT);

	adc_init();
	adc_gpio_init(SOIL_PIN);
	adc_gpio_init(BAT_PIN);

	gpio_set_function(AT_UART_TX, GPIO_FUNC_UART);
	gpio_set_function(AT_UART_RX, GPIO_FUNC_UART);

	gpio_set_function(AHT_SDA, GPIO_FUNC_I2C);
	gpio_set_function(AHT_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(AHT_SDA);
	gpio_pull_up(AHT_SCL);

	// Sensor Power
	gpio_put(POWER_PIN, 1);

	// Reset Lora
	gpio_put(LORA_RST, 0);
	sleep_ms(2000);
	gpio_put(LORA_RST, 1);
}

void blink(uint pin, uint num, uint time)
{
	for (uint i = 0; i < num; i++)
	{
		gpio_put(pin, 1);
		sleep_ms(time);
		gpio_put(pin, 0);
		sleep_ms(time);
	}
}

//---------------Soil & Bat ADC----------------------------
void soil_read()
{
	// Read ADC
	adc_select_input(SOIL_ADC);
	uint16_t result = adc_read();
	percent = (result - SOIL_ADC_WATER) / SOIL_ADC_UNIT;
	if (percent > 100)
		percent = 100;
	if (percent < 0)
		percent = 0;
	// printf("Soil adc value: %d percent:%d%\n", result, percent);
}

void bat_read()
{
	adc_select_input(BAT_ADC);
	uint16_t result = adc_read();

	bat = (int)(result * 69 * 33 / 4096.0 / 47);
}

void soil_pwm_set()
{
	// PWM set
	uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
	gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
	// printf("GPIO:%d Slice:%d\n", PWM_PIN, slice_num);

	pwm_config config = pwm_get_default_config();
	pwm_config_set_clkdiv(&config, 31.25); // 2MHz 125MHz /2 /31.25
	pwm_init(slice_num, &config, 1);
	pwm_set_enabled(slice_num, true);

	pwm_set_wrap(slice_num, 1);
	pwm_set_chan_level(slice_num, PWM_CHAN_B, 1);
}

//----------------- AT ----------------------------
void at_init()
{
	// Set up our UART with the required speed.
	uart_init(AT_UART, 9600);

	sleep_ms(100);

	send_AT("AT+CGMR", AT_TIMEOUT);

	char cmd[80];

	sprintf(cmd, "%s%s", "AT+CDEVEUI=", DEVEUI);
	send_AT(cmd, AT_TIMEOUT);
	sprintf(cmd, "%s%s", "AT+CAPPEUI=", APPEUI);
	send_AT(cmd, AT_TIMEOUT);
	sprintf(cmd, "%s%s", "AT+CAPPKEY=", APPKEY);
	send_AT(cmd, AT_TIMEOUT);

	send_AT("AT+CCLASS=0", AT_TIMEOUT);
	send_AT("AT+CJOINMODE=0", AT_TIMEOUT);
	send_AT("AT+CJOIN=1,0,8,8", AT_TIMEOUT);
}

void uart_read_to_timeout(uint64_t timeout_ms)
{
	char line[80];
	int i = 0;

	uint64_t runtime = time_us_64();
	uint64_t timeout_us = timeout_ms * 1000;
	while (1)
	{
		if (uart_is_readable(AT_UART))
		{
			char c;
			c = uart_getc(AT_UART);

			if (c == '\n' || i > 78)
			{
				line[i++] = '\n';
				line[i] = '\0';

				printf("RX\t->\t%s", line);
				i = 0;
			}
			else if (c == '\r')
				continue;
			else
			{
				line[i++] = c;
			}
		}

		if ((time_us_64() - runtime) > timeout_us)
		{
			// Out put remain chars
			if (i != 0)
			{
				line[i++] = '\n';
				line[i] = '\0';
				printf("RX\t->\t%s", line);
			}

			break;
		}

		sleep_ms(10);
	}
}

void send_AT(const char *cmd, uint64_t timeout_ms)
{
	uart_puts(AT_UART, cmd);
	uart_puts(AT_UART, "\r\n");
	uart_default_tx_wait_blocking();

	sleep_ms(100);

	uart_read_to_timeout(timeout_ms);
}

//----------------- I2C ------------------------
// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr)
{
	return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void i2c_scan()
{
	printf("\nI2C Bus Scan\n");
	printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

	for (int addr = 0; addr < 128; ++addr)
	{
		if (addr % 16 == 0)
		{
			printf("%02x ", addr);
		}

		// Perform a 1-byte dummy read from the probe address. If a slave
		// acknowledges this address, the function returns the number of bytes
		// transferred. If the address byte is ignored, the function returns
		// -1.

		// Skip over any reserved addresses.
		int ret;
		uint8_t rxdata;
		if (reserved_addr(addr))
			ret = PICO_ERROR_GENERIC;
		else
			ret = i2c_read_blocking(AHT_I2C, addr, &rxdata, 1, false);

		printf(ret < 0 ? "." : "@");
		printf(addr % 16 == 15 ? "\n" : "  ");
		sleep_ms(10);
	}
	printf("Done.\n");
}

//------------------Sleep Function----------------------

static void sleep_callback(void)
{
	awake = true;
	return;
}

static void my_rtc_set()
{
	datetime_t t = {
		.year = DEF_YEAR,
		.month = DEF_MONTH,
		.day = DEF_DAY,
		.dotw = DEF_DOT, // 0 is Sunday, so 5 is Friday
		.hour = DEF_HOUR,
		.min = DEF_MIN,
		.sec = DEF_SEC};

	// Reset real time clock to a value
	if (rtc_set_datetime(&t) == 1)
	{
		printf("Set clock success:\n");
		print_rtc();
	}
}

static void my_rtc_sleep(int8_t minute_to_sleep_to, int8_t second_to_sleep_to)
{

	datetime_t t_alarm = {
		.year = DEF_YEAR,
		.month = DEF_MONTH,
		.day = DEF_DAY,
		.dotw = DEF_DOT,
		.hour = DEF_HOUR,
		.min = minute_to_sleep_to,
		.sec = second_to_sleep_to};

	printf("Going to sleep.......\n");
	sleep_goto_sleep_until(&t_alarm, &sleep_callback);
}

void reset_all_clock(uint scb_orig, uint clock0_orig, uint clock1_orig)
{

	// Re-enable ring Oscillator control
	rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

	// reset procs back to default
	scb_hw->scr = scb_orig;
	clocks_hw->sleep_en0 = clock0_orig;
	clocks_hw->sleep_en1 = clock1_orig;

	// reset clocks
	clocks_init();
	stdio_init_all();

	return;
}

void print_clock(void)
{
	uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
	uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
	uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
	uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
	uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
	uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
	uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
	uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

	printf("pll_sys  = %dkHz\n", f_pll_sys);
	printf("pll_usb  = %dkHz\n", f_pll_usb);
	printf("rosc     = %dkHz\n", f_rosc);
	printf("clk_sys  = %dkHz\n", f_clk_sys);
	printf("clk_peri = %dkHz\n", f_clk_peri);
	printf("clk_usb  = %dkHz\n", f_clk_usb);
	printf("clk_adc  = %dkHz\n", f_clk_adc);
	printf("clk_rtc  = %dkHz\n", f_clk_rtc);
}

void print_rtc()
{
	datetime_t t;
	rtc_get_datetime(&t);
	printf("Now time:%d/%d/%d %d:%d:%d\n", t.year, t.month, t.day, t.hour, t.min, t.sec);
}

void sleep_task()
{

	print_clock();
	print_rtc();

	awake = false;
	my_rtc_set();

	printf("Sleep from xosc\n");

	sleep_run_from_xosc();

	my_rtc_sleep(45, 0);

	// will return here and awake should be true
	while (!awake)
	{
		blink(LED_PIN, 10, 100);
		printf("Should be sleeping\n");
	}
}