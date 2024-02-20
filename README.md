SparkFun OpenLog Artemis : GNSS Logger
===========================================================

[![SparkFun OpenLog Artemis](https://cdn.sparkfun.com//assets/parts/1/5/7/5/3/16832-SparkFun_OpenLog_Artemis-01.jpg)](https://www.sparkfun.com/products/16832)

[*SparkFun OpenLog Artemis (SPX-16832)*](https://www.sparkfun.com/products/16832)

The OpenLog Artemis is an open source datalogger that comes preprogrammed to automatically log IMU, GPS, serial data, and various pressure, humidity, and distance sensors. All without writing a single line of code! OLA automatically detects, configures, and logs Qwiic sensors. OLA is designed for users who just need to capture a bunch of data to SD and get back to their larger project.

The firmware in this repo is dedicated to logging data from the latest u-blox GNSS modules. [You can find the main OpenLog Artemis repo here](https://github.com/sparkfun/OpenLog_Artemis).

Data is logged to SD card in u-blox's UBX format without being processed. From v2.0, you can also optionally log NMEA messages too - into the same file as the UBX data. The files can be analyzed with (e.g.)
[u-center](https://www.u-blox.com/en/product/u-center) or RTKLIB. You will find everything you need to know about exploring precision GPS/GNSS with RTKLIB over at
[rtklibexplorer.wordpress.com](https://rtklibexplorer.wordpress.com/). We have written our own Python 3 [UBX Integrity Checker](./Utils) which you may find useful.

OpenLog Artemis is highly configurable over an easy to use serial interface. Simply plug in a USB C cable and open a terminal at 115200kbps. The logging output is automatically streamed to both the terminal and the microSD. Pressing any key will open the configuration menu.

The OpenLog Artemis automatically scans, detects, configures, and logs data from a Qwiic GNSS sensor plugged into the board (no soldering required!). From v2.0 (based on v2.1.0 of the Apollo3 core) all u-blox Series 8, 9 and 10 modules are supported:

* [ZED-F9P](https://www.sparkfun.com/products/15136)
* [ZED-F9P SMA](https://www.sparkfun.com/products/16481)
* [ZED-F9R](https://www.sparkfun.com/products/16344)
* [NEO-M9N](https://www.sparkfun.com/products/15712)
* [NEO-M8P](https://www.sparkfun.com/products/15005)
* [ZOE-M8Q](https://www.sparkfun.com/products/15193)
* [SAM-M8Q](https://www.sparkfun.com/products/15210)
* [NEO-M8U](https://www.sparkfun.com/products/16329)
* [MAX-M10S](https://www.sparkfun.com/products/18037)
* [SAM-M10Q](https://www.sparkfun.com/products/21834)
* [NEO-F9P](https://www.sparkfun.com/products/23288)
* More boards are being added all the time!

The menus will let you configure:

* Which messages are logged. You can currently select from:
  * UBX-NAV-CLOCK     (Clock Solution)
  * UBX-NAV-DOP       (Dilution Of Precision)
  * UBX-NAV-ODO       (Odometer)
  * UBX-NAV-POSECEF   (Position Earth-Centered Earth-Fixed)
  * UBX-NAV-POSLLH    (Position Lat/Lon/Height)
  * UBX-NAV-PVT       (Position, Velocity, Time)
  * UBX-NAV-STATUS    (Receiver Navigation Status)
  * UBX-NAV-TIMEUTC   (UTC Time Solution)
  * UBX-NAV-VELECEF   (Velocity Solution Earth-Centered Earth-Fixed)
  * UBX-NAV-VELNED    (Velocity Solution North/East/Down)
  * UBX-RXM-MEASX     (Satellite measurements for RRLP)
  * UBX-RXM-SFRBX     (Broadcast Navigation Data Subframe)
  * UBX-TIM-TM2       (Time Mark Data)
* If your module supports them, you can also log:
  * UBX-ESF-MEAS      (External Sensor Fusion Measurements) (ADR / UDR modules only)
  * UBX-ESF-RAW       (External Sensor Fusion Raw Sensor Measurements) (ADR / UDR modules only)
  * UBX-ESF-STATUS    (External Sensor Fusion Status) (ADR / UDR modules only)
  * UBX-ESF-ALG       (External Sensor Fusion IMU Alignment) (ADR / UDR modules only)
  * UBX-ESF-INS       (External Sensor Fusion Vehicle Dynamics) (ADR / UDR modules only)
  * UBX-HNR-PVT       (High Navigation Rate PVT) (ADR / UDR modules only)
  * UBX-HNR-ATT       (High Navigation Rate Attitude Solution) (ADR / UDR modules only)
  * UBX-HNR-INS       (High Navigation Rate Vehicle Dynamics (ADR / UDR modules only)
  * UBX-NAV-ATT       (Attitude Solution) (ADR / UDR modules only)
  * UBX-NAV-HPPOSECEF (High Precision Position Earth-Centered Earth-Fixed) (HPG modules only)
  * UBX-NAV-HPPOSLLH  (High Precision Position Lat/Lon/Height) (HPG modules only)
  * UBX-NAV-RELPOSNED (Relative Position North/East/Down) (HPG modules only)
  * UBX-RXM-RAWX      (Multi-GNSS Raw Measurement) (ADR/ HPG / Time Sync modules only)
* From v2.0, you can select which NMEA messages to log too:
  * DTM (Datum Reference)
  * GBS (Satellite Fault Detection)
  * GGA (Global Positioning System Fix)
  * GLL (Latitude and Longitude Position Fix and Status)
  * GNS (GNSS Fix Data)
  * GRS (GNSS Range Residuals)
  * GSA (DOP and Active Satellites)
  * GST (GNSS Pseudorange Error Statistics)
  * GSV (GNSS Satellites In View)
  * RMC (Recommended Minimum Data)
  * TXT (Text Transmission)
  * VLW (Dual Ground / Water Distance)
  * VTG (Course Over Ground and Ground Speed)
  * ZDA (Time and Date)
* Logging and sleep durations
  * Want to log RAWX data for 10 minutes once per day? You can absolutely do that!
* Power management
  * You can choose to put the GNSS module to sleep or disconnect its power completely between logging sessions.

New features are constantly being added so we’ve released an easy to use firmware upgrade tool. No need to install Arduino or a bunch of libraries, simply open the [Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI), load the latest OLA firmware, and add features to OpenLog Artemis as they come out! Full instructions are available in [UPGRADE.md](./UPGRADE.md).

Versions
--------

The latest version of this firmware (v3.0) was written for the latest u-blox modules (F9 and M10) which no longer support messages like UBX-CFG-PRT and UB-CFG-MSG.
v3.0 uses the Configuration Interface (VALSET and VALGET) to configure the module. It is built around v3 of the SparkFun u-blox GNSS library.

Please note: v3.0 is not backward compatible with v2.2. If you have an older M8 module, please continue to use v2.2. v3.0 will only work with newer modules
(F9, M10) which support the Configuration Interface.

Please see [CHANGELOG.md](./CHANGELOG.md) for details.

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
* **Hookup Guide** - You can find the OpenLog Artemis Hookup Guide [here](https://learn.sparkfun.com/tutorials/openlog-artemis-hookup-guide).

License Information
-------------------

This product is _**open source**_!

Please see [LICENSE.md](./LICENSE.md) for full details.

- Your friends at SparkFun.
