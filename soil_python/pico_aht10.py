import aht10
import time
from machine import Pin, I2C

power_pin = Pin(8,Pin.OUT)
power_pin.on()
time.sleep(2)

i2c = I2C(1, scl=Pin(7), sda=Pin(6), freq=400_000)
print(i2c.scan())

aht = aht10.AHT10(i2c)

while True:
    print(aht.temperature())
    print(aht.humidity())
    time.sleep(2)
    