
from machine import ADC, Pin, PWM, I2C, UART
import aht10
import time

DEVEUI = "6081F9EFE6C31D65"
APPEUI = "6081F9D6B0D9B9F4"
APPKEY = "9ED8865D20830BF00EDC6DFB572CD254"

soil_air = 48300
soil_water = 41100
soil_unit = (soil_air - soil_water) / 100

# Pin define
pwm = PWM(Pin(9))
soil_adc = ADC(Pin(26))
bat_adc = ADC(Pin(29))
i2c = I2C(1, scl=Pin(7), sda=Pin(6), freq=400_000)
uart = UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))

POWER_PIN = Pin(8, Pin.OUT)
RST_KEY = Pin(20, Pin.OUT)

POWER_PIN.on()
pwm.freq(2_000_000)
pwm.duty_u16(32768)
time.sleep(2)

# Sensor
aht = aht10.AHT10(i2c)

# Global Value
temperature = -99
humidity = -99
soil_value = -99
data_num = 0


def lorawan_reset():
    print("Reset Lorawan")
    RST_KEY.off()
    time.sleep(2)
    RST_KEY.on()

    time.sleep(3)
    print("Reset Over")


def lorawan_init():
    waitRespLine()

    sendCMD_waitRespLine("AT+CDEVEUI=" + DEVEUI)
    sendCMD_waitRespLine("AT+CAPPEUI=" + APPEUI)
    sendCMD_waitRespLine("AT+CAPPKEY=" + APPKEY)

    sendCMD_waitRespLine("AT+CCLASS=0")
    sendCMD_waitRespLine("AT+CJOINMODE=0")
    sendCMD_waitRespLine("AT+CJOIN=1,0,8,8")


def lorawan_send(msg):
    waitRespLine()
    sendCMD_waitRespLine("AT+DTRX=1,2,6," + msg)


def sendCMD_waitRespLine(cmd, timeout=3000):
    print("CMD: " + cmd)
    cmd += "\r\n"
    uart.write(cmd)
    waitRespLine(timeout)
    print()


def waitRespLine(timeout=3000):
    prvMills = time.ticks_ms()
    while (time.ticks_ms()-prvMills) < timeout:
        if uart.any():
            try:
                # print(uart.readline().decode('utf8'))
                # print(uart.readline())
                print(uart.read().decode('utf8'))
                # print(uart.read())
            except:
                print("...")


def sensor_init():
    print(i2c.scan())


def sensor_read():
    global temperature
    global humidity
    global soil_value
    global bat_value
    global data_num

    # aht read
    temperature = int(aht.temperature())
    humidity = int(aht.humidity())

    sum = 0
    for i in range(10):
        sum = sum + soil_adc.read_u16()
        time.sleep(0.1)

    soil_value = soil2percent(sum / 10)

    bat_value = int(bat_adc.read_u16() / 65535 / 47 * 69 * 33)

    data_num = data_num + 1


def data_print():

    str_data = ""
    str_data = str_data + "SOIL:" + str(soil_value)
    str_data = str_data + " TEMP:" + str(temperature)
    str_data = str_data + " HUMI:" + str(humidity)
    str_data = str_data + " BAT:" + str(bat_value)
    str_data = str_data + " NUM:" + str(data_num)

    lora_msg = "%02x" % soil_value + "%02x" % temperature + \
        "%02x" % humidity + "%02x" % bat_value + \
        "%04x" % data_num

    print(str_data)
    print(lora_msg)

    lorawan_send(lora_msg)


def soil2percent(soil_adc):
    if soil_adc > soil_air:
        return 100
    if soil_adc < soil_water:
        return 0
    return int((soil_adc - soil_water)/soil_unit)


def main():
    lorawan_reset()
    waitRespLine()

    lorawan_init()

    sensor_init()
    while True:
        time.sleep(10)
        sensor_read()
        data_print()
        time.sleep(600)
        # time.sleep(50)


main()
