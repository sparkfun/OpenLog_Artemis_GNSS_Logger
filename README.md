SparkFun OpenLog Artemis : GNSS Logger
===========================================================

[![SparkFun OpenLog Artemis](https://cdn.sparkfun.com//assets/parts/1/5/7/5/3/16832-SparkFun_OpenLog_Artemis-01.jpg)](https://www.sparkfun.com/products/16832)

[*SparkFun OpenLog Artemis (SPX-16832)*](https://www.sparkfun.com/products/16832)

The OpenLog Artemis is an open source datalogger that comes preprogrammed to automatically log IMU, GPS, serial data, and various pressure, humidity, and distance sensors. All without writing a single line of code! OLA automatically detects, configures, and logs Qwiic sensors. OLA is designed for users who just need to capture a bunch of data to SD and get back to their larger project.

The firmware in this repo is dedicated to logging data from the latest u-blox GNSS modules. [You can find the main OpenLog Artemis repo here](https://github.com/sparkfun/OpenLog_Artemis).

Data is logged to SD card in u-blox's UBX format without being processed. The files can be analyzed with (e.g.) [u-center](https://www.u-blox.com/en/product/u-center) or RTKLIB.
You will find everything you need to know about exploring precision GPS/GNSS with RTKLIB over at [rtklibexplorer.wordpress.com](https://rtklibexplorer.wordpress.com/).

OpenLog Artemis is highly configurable over an easy to use serial interface. Simply plug in a USB C cable and open a terminal at 115200kbps. The logging output is automatically streamed to both the terminal and the microSD. Pressing any key will open the configuration menu.

The OpenLog Artemis automatically scans, detects, configures, and logs various Qwiic GNSS sensors plugged into the board (no soldering required!). Currently, auto-detection is supported on the following Qwiic products:

* [ZED-F9P](https://www.sparkfun.com/products/15136)
* [ZED-F9P SMA](https://www.sparkfun.com/products/16481)
* [ZED-F9R](https://www.sparkfun.com/products/16344)
* [NEO-M9N](https://www.sparkfun.com/products/15712)
* More boards are being added all the time!

_Please note: the M8 family of GNSS modules are not supported by this version of the firmware. You can of course use the standard [OpenLog Artemis firmware](https://github.com/sparkfun/OpenLog_Artemis) if you want to log data from those._

The menus will let you configure:

* Which messages are logged. You can currently select from:
  * UBX-NAV-CLOCK     (Clock Solution)
  * UBX-NAV-DOP       (Dilution Of Precision)
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
  * UBX-NAV-ATT       (Attitude Solution) (Currently only supported on the ZED-F9R)
  * UBX-NAV-RELPOSNED (Relative Position North/East/Down)
  * UBX-RXM-RAWX      (Multi-GNSS Raw Measurement)
* Logging and sleep durations. Want to log RAWX data for 10 minutes once per day? You can absolutely do that!
* Power management. You can choose to put the GNSS module to sleep or disconnect its power completely between logging sessions.

New features are constantly being added so we’ve released an easy to use firmware upgrade tool. No need to install Arduino or a bunch of libraries, simply open the [Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI), load the latest OLA firmware, and add features to OpenLog Artemis as they come out! Full instructions are available in [UPGRADE.md](./UPGRADE.md).

Repository Contents
-------------------

* **/Binaries** - The binary files for the different versions of the OLA firmware.
* **/Firmware** - The main sketch that runs on the OpenLog Artemis.
* **/Utils** - Python utilities to help debug any logging gremlins.

Documentation
--------------

* **[UPGRADE.md](./UPGRADE.md)** - contains full instructions on how to upgrade the firmware on the OLA using the [Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI).
* **[CONTRIBUTING.md](./CONTRIBUTING.md)** - guidance on how to contribute to this library.
* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - OLA includes a large number of libraries that will need to be installed before compiling will work.
* **Hookup Guide** - Coming soon! You can find the OLA Hookup Guide [here](https://learn.sparkfun.com/tutorials/openlog-artemis-hookup-guide).

License Information
-------------------

This product is _**open source**_!

Various bits of the code have different licenses applied. Anything SparkFun wrote is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
