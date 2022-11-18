

# MaPie LoraWAN Soil Moisture



```c++
/*
Version:		V1.0
Author:			Vincent
Create Date:	2022/11/18
Note:
*/
```



# Makerfabs

[Makerfabs home page](https://www.makerfabs.com/)

[Makerfabs Wiki](https://makerfabs.com/wiki/index.php?title=Main_Page)



# MaPie LoraWAN Soil Moisture

## Intruduce

The Lorawan soil moisture sensor is based on RP2040, it collects local air temperature/ humidity with sensor AHT10, and detects the soil humidity with capacitor-humility measurement solution. It can transmit the local environment data to the gateway or devices with the **Lorawan** communication, and it suits for the applications such as smart-farm, irrigation, agriculture, etc.

In applications, always you do not need to check the air/soil state continuously, have a test of them for few seconds after then minutes/hours sleeping is normally Ok for most projects. To save power, there the Air/ Soil measuring functional could be shut down in the working, so they can be only powered ON a short time and then a long time power off. With MCU in sleeping mode and low power consumption Lora module, this module can work with 3 AAA batteries for more than one year. Besides, this sensor is coated with waterproof paint, which makes it working longer in wet soil.





# C Version

## Feature

Original C SDK

Low power hibernation current 2.5ma。



## C SDK Set

Install the development environment and extension libraries.

```shell
cd 
mkdir code
cd code

# Download pico sdk and pico extra library
git clone https://github.com/raspberrypi/pico-sdk.git
git clone https://github.com/raspberrypi/pico-extras.git

# Update libraries
cd pico-sdk
git submodule update --init
cd ../pico-extras
git submodule update --init

# Set environment path
export PICO_SDK_PATH=~/code/pico-sdk
export PICO_EXTRAS_PATH=~/code/pico-extras

# Other applications may be required depending on the system.
# Such as build-essential
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib


 
```

## Lorawan Set

Change code in main.c

```python
#define DEVEUI "6081F9EFE6C31D65"
#define APPEUI "6081F9D6B0D9B9F4"
#define APPKEY "9ED8865D20830BF00EDC6DFB572CD254"
```

Modify to your own Lorawan platform application EUI, such as Thing Net Work or Heliums.

 

## Compiler Program

Run cmake and make.

```shell
cd MaPie-LoraWAN-Soil-Moisture\soil_csdk

# Create build dir
mkdir build
cd build

cmake ..

make

```



## Burn Fireware

Connect MaPie to PC.

Hold boot button and press reset button.

Then copy the UF2 file in the build folder to the USB flash drive device that appears.

 





# MicroPython Version

## Burn Fireware

Download MicroPython from the Raspberry PI Pico's official tutorial:

[https://www.raspberrypi.com/products/raspberry-pi-pico/](https://www.raspberrypi.com/products/raspberry-pi-pico/)



## Lorawan Set

Change code in soil.py

```python
DEVEUI = "6081F9EFE6C31D65"
APPEUI = "6081F9D6B0D9B9F4"
APPKEY = "9ED8865D20830BF00EDC6DFB572CD254"
```

Modify to your own Lorawan platform application EUI, such as Thing Net Work or Heliums.

 

## Burn Fireware

Connect MaPie to PC.

Use Thonny IDE, upload all file to MaPie.