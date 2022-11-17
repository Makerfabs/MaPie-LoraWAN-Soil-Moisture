import os
import utime
from machine import Pin, UART

DEVEUI = "6081F9EFE6C31D65"
APPEUI = "6081F9D6B0D9B9F4"
APPKEY = "9ED8865D20830BF00EDC6DFB572CD254"


RST_KEY = Pin(20, Pin.OUT)

uart = UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))


def sendCMD_waitRespLine(cmd, timeout=3000):
    print("CMD: " + cmd)
    cmd += "\r\n"
    uart.write(cmd)
    waitRespLine(timeout)
    print()

def waitRespLine(timeout=3000):
    prvMills = utime.ticks_ms()
    while (utime.ticks_ms()-prvMills) < timeout:
        if uart.any():
            try:
                #print(uart.readline().decode('utf8'))
                #print(uart.readline())
                print(uart.read().decode('utf8'))
                #print(uart.read())
            except:
                print("...")

def lorawan_reset():
    print("Reset Lorawan")
    RST_KEY.off()
    utime.sleep(2)
    RST_KEY.on()

    utime.sleep(3)
    print("Reset Over")

def main():
    print(os.uname())
    print(uart)

    lorawan_reset()

    waitRespLine()

    # sendCMD_waitRespLine("AT+CGMI")
    sendCMD_waitRespLine("AT+CGMI")
    sendCMD_waitRespLine("AT+CGMM")
    sendCMD_waitRespLine("AT+CGMR")
    
    sendCMD_waitRespLine("AT+CDEVEUI=" + DEVEUI)
    sendCMD_waitRespLine("AT+CAPPEUI=" + APPEUI)
    sendCMD_waitRespLine("AT+CAPPKEY=" + APPKEY)
    
    sendCMD_waitRespLine("AT+CCLASS=0")
    sendCMD_waitRespLine("AT+CJOINMODE=0")
    sendCMD_waitRespLine("AT+CJOIN=1,0,8,8")
    
    while False:
        waitRespLine()
        utime.sleep(1)
    
    
    utime.sleep(10)
    waitRespLine()
    
    sendCMD_waitRespLine("AT+DTRX=1,2,5,0123456789")
    
    utime.sleep(10)
    waitRespLine()
    sendCMD_waitRespLine("AT+DTRX=1,2,6,ff8800ff8800")
    


    print()
    print("Over")


main()

