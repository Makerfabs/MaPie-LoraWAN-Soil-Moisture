# Example using PWM to fade an LED.

import time
from machine import ADC, Pin, PWM


pwm = PWM(Pin(9))
adc1 = ADC(Pin(26))

freq_list = [
    600000,
    700000,
    800000,
    900000,
    1000000,
    2000000,
    3000000,
    4000000,
]


def pwm_task(freq):

    pwm.freq(freq)
    pwm.duty_u16(32768)
    time.sleep(1)

    sum = 0

    for i in range(10):
        # print(int(adc1.read_u16()/64 ))
        sum = sum + adc1.read_u16()
        time.sleep(1)
    adc_value = int(sum / 10)
    print(str(freq) + " " + str(adc_value))


def main():
    
    #test
    pwm.freq(2000000)
    pwm.duty_u16(32768)
    while 1:
        time.sleep(1)
    
    print("Start freq and adc test.")
    for i in range(6,40):
        pwm_task(i * 100000)
    print("Test over.")


main()
