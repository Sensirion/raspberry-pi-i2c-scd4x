# Sensirion Raspberry Pi I2C SCD4x Driver

This document explains how to set up the SCD4x sensor to run on a Raspberry Pi
using the provided code.

[<center><img src="images/SCD41_Development_Board.png" width="300px"></center>](https://sensirion.com/my-scd-ek)

Click [here](https://sensirion.com/my-scd-ek) to learn more about the SCD4x
sensor and the SCD41 Evaluation Kit Board.

## Setup Guide

### Connecting the Sensor

Your sensor has the four different connectors: VCC, GND, SDA, SCL. Use
the following pins to connect your SCD4x:

 *SCD4x*  |    *Raspberry Pi*    | *Jumper Wire*
 :------: | :------------------: | :-----------:
   VCC    |        Pin 1         |      Red
   GND    |        Pin 6         |     Black
   SDA    |        Pin 3         |     Green
   SCL    |        Pin 5         |     Yellow

<center><img src="images/GPIO-Pinout-Diagram.png" width="900px"></center>

### Raspberry Pi

- [Install the Raspberry Pi OS on to your Raspberry Pi](https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up)
- [Enable the I2C interface in the raspi-config](https://www.raspberrypi.org/documentation/configuration/raspi-config.md)
- Download the driver from the [Sensirion GitHub Page](https://github.com/Sensirion/raspberry-pi-i2c-scd4x/tags)
  and extract the `.zip` on your Raspberry Pi
- Compile the driver
    1. Open a [terminal](https://www.raspberrypi.org/documentation/usage/terminal/)
    2. Navigate to the driver directory. E.g. `cd ~/raspberry-pi-i2c-scd4x`
    3. Run the `make` command to compile the driver

       Output:
       ```
       rm -f scd4x_i2c_example_usage
       cc -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -Wsign-conversion -fPIC -I. -o scd4x_i2c_example_usage  scd4x_i2c.h scd4x_i2c.c sensirion_i2c_hal.h sensirion_i2c.h sensirion_i2c.c \
       	sensirion_i2c_hal.c sensirion_config.h sensirion_common.h sensirion_common.c scd4x_i2c_example_usage.c
       ```
- Test your connected sensor
    - Run `./scd4x_i2c_example_usage` in the same directory you used to
      compile the driver.

      Output:
      ```
      serial: 0xbff79f073b51
      CO2: 799
      Temperature: 20.92
      Humidity: 35.95
      CO2: 900
      Temperature: 20.92
      Humidity: 36.47
      CO2: 926
      Temperature: 20.81
      Humidity: 36.85
      ...
      ```

Troubleshooting
---------------

### Building driver failed

If the execution of `make` in the compilation step 3 fails with something like

> -bash: make: command not found

your RaspberryPi likely does not have the build tools installed. Proceed as follows:

```
$ sudo apt-get update
$ sudo apt-get upgrade
$ sudo apt-get install build-essential
```

### Initialization failed

If you run `./scd4x_i2c_example_usage` but do not get sensor readings but
something like this instead

```
Error executing scd4x_get_serial_number(): -1
Error executing scd4x_start_periodic_measurement(): -1
Error executing scd4x_read_measurement(): -1
```

then go through the below troubleshooting steps.

-   Ensure that you connected the sensor correctly: All cables are fully
    plugged in and connected to the correct pin.
-   Ensure that I2C is enabled on the Raspberry Pi. For this redo the steps on
    "Enable the I2C interface in the raspi-config" in the guide above.
-   If running the same command as root with `sudo ./scd4x_i2c_example_usage`
    works then your user is likely missing access permissions for the I2C 
    interface. See the next chapter how to solve this.
    
### Missing I2C permissions

If your user is missing access to the I2C interface you should first verfiy
the user belongs to the `i2c` group.

```
$ groups
users input some other groups etc
```
If `i2c` is missing in the list add the user and restart the Raspberry Pi.

```
$ sudo adduser your-user i2c
Adding user `your-user' to group `i2c' ...
Adding user your-user to group i2c
Done.
$ sudo reboot
```

If that did not help you can make globally accessible hardware interfaces
with a udev rule. Only do this if everything else failed and you are 
reasoably confident you are the only one having access to your Pi.

Go into the `/etc/udev/rules.d` folder and add a new file named 
`local.rules`.
```
$ cd /etc/udev/rules.d/
$ sudo touch local.rules
```
Then add a single line `ACTION=="add", KERNEL=="i2c-[0-1]*", MODE="0666"` 
to the file with your favorite editor.
```
$ sudo vi local.rules
```
