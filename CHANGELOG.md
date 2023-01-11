Change Log
======================

* V3.0
  * Updated to resolve Issue #28 - code now uses SparkFun u-blox GNSS library v3
  * Note: v3 will only work on modules that use the Configuration Interface (F9 / M10)
  * For M8 modules, please continue to use v2.2
  * Also adds the NMEA and UBX output to TX pin option - requested in #26
* V2.2 :
  * Corrects the file sync issue reported in Issue #24
* V2.1 :
  * settings.enableTerminalOutput is now updated and stored correctly after leaving the menus
  * Add GNSS settings to settings file
  * Add a menu option for the Qwiic pull-ups making it easier to experiment with 400kHz
* V2.0 :
  * Update using v2.1.0 of the SparkFun Apollo3 (Artemis) core and v2.0.9 of the SparkFun u-blox GNSS library
  * Please note that v2.1.1 of Apollo3 contains a feature which makes communication with u-blox modules over I<sup>2</sup>C problematic. Please use v2.1.0 if you are compiling the code yourself.
  * Added support for ESF, HNR and NMEA messages
  * Uses Bill Greiman's SdFat v2.0.7 - supports both FAT32 and exFAT
* V1.3 :
  * Fixed the I2C_BUFFER_LENGTH gremlin in storeData.ino (thank you @adamgarbo)
  * Added improved log file timestamping - same as the OLA
  * Add functionality to enable/disable GNSS constellations (thank you @adamgarbo)
  * Add low battery detection
  * Added support for NAV_DOP
  * Added support for NAV_ATT (on the ZED-F9R HPS module)
  * Removed the hard-coded key values. The code now uses the key definitions from u-blox_config_keys.h
* V1.2 :
  * Add delay to allow GPS to intialize on v10 hardware
  * Unhid the debug menu
* V1.1 :
  * Upgrades to match v14 of the OpenLog Artemis
  * Support for the V10 hardware
* V1.0 :
  * Initial release based on v13 of the OpenLog Artemis
