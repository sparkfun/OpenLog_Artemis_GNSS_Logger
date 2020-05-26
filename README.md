SparkFun OpenLog Artemis
===========================================================

[![SparkFun OpenLog Artemis](https://cdn.sparkfun.com//assets/parts/1/4/4/8/0/15846-OpenLog_Artemis-01.jpg)](https://www.sparkfun.com/products/15846)

[*SparkFun OpenLog Artemis (SPX-15846)*](https://www.sparkfun.com/products/15846)

The OpenLog Artemis is an open source datalogger the comes preprogrammed to automatically log IMU, GPS, serial data, and various pressure, humidity, and distance sensors. All without writing a single line of code! OLA automatically detects, configures, and logs Qwiic sensors. OLA is designed for users who just need to capture a bunch of data to a CSV and get back to their larger project.

This version of the firmware is dedicated to logging data from the latest u-blox GNSS modules. Data is logged to SD card in u-blox's UBX format without being processed. The files can be analyzed with
(e.g.) [u-center](https://www.u-blox.com/en/product/u-center) or RTKLIB. You will find everything you need to know about exploring precision GPS/GNSS with RTKLIB over at https://rtklibexplorer.wordpress.com/

OpenLog Artemis is highly configurable over an easy to use serial interface. Simply plug in a USB C cable and open a terminal at 115200kbps. The logging output is automatically streamed to both the terminal and the microSD. Pressing any key will open the configuration menu.

The OpenLog Artemis automatically scans, detects, configures, and logs various Qwiic GNSS sensors plugged into the board (no soldering required!). Currently, auto-detection is supported on the following Qwiic products:

* [ZED-F9P](https://www.sparkfun.com/products/15136)
* [ZED-F9P SMA](https://www.sparkfun.com/products/16481)
* [NEO-M9N](https://www.sparkfun.com/products/15712)
* More boards are being added all the time!

_Please note: the M8 family of GNSS modules are not currently supported by this version of the firmware. You can of course use the standard [OpenLog Artemis firmware](https://github.com/sparkfun/OpenLog_Artemis) if you want to log data from those._

The menus will let you configure:

* Which messages are logged. You can currently select from:
  * UBX-NAV-CLOCK     (Clock Solution)
  * UBX-NAV-HPPOSECEF (High Precision Position Earth-Centered Earth-Fixed)
  * UBX-NAV-HPPOSLLH  (High Precision Position Lat/Lon/Height)
  * UBX-NAV-ODO       (Odometer)
  * UBX-NAV-POSECEF   (Position Earth-Centered Earth-Fixed)
  * UBX-NAV-POSLLH    (Position Lat/Lon/Height)
  * UBX-NAV-PVT       (Position, Velocity, Time)
  * UBX-NAV-STATUS    (Receiver Navigation Status)
  * UBX-NAV-TIMEUTC   (UTC Time Solution)
  * UBX-NAV-VELECEF   (Velocity Solution Earth-Centered Earth-Fixed)
  * UBX-NAV-VELNED    (Velocity Solution North/East/Down)
  * UBX-RXM-SFRBX     (Broadcast Navigation Data Subframe)
  * UBX-TIM-TM2       (Time Mark Data)
* If your module supports them, you can also log:
  * UBX-NAV-RELPOSNED (Relative Position North/East/Down)
  * UBX-RXM-RAWX      (Multi-GNSS Raw Measurement)
* Logging and sleep durations. Want to log RAWX data for 10 minutes once per day? You can absolutely do that!
* Power management. You can choose to put the GNSS module to sleep or disconnect its power completely between logging sessions.

New features are constantly being added so we’ve released an easy to use firmware upgrade tool. No need to install Arduino or a bunch of libraries, simply open the [Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI), load the latest OLA firmware, and add features to OpenLog Artemis as the come out!

Repository Contents
-------------------

* **/Firmware** - The main sketch that runs on the OpenLog Artemis.

Documentation
--------------

* **[Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI)** - Used to upgrade the firmware on OLA
* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - OLA includes a large number of libraries that will need to be installed before compiling will work.
* **Hookup Guide** - Coming soon!

License Information
-------------------

This product is _**open source**_!

Various bits of the code have different licenses applied. Anything SparkFun wrote is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
