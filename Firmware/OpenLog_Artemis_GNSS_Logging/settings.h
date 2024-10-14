//Sensor settings

// settings.sensor_uBlox.log     : indicates if the user has enabled u-blox logging. Set by menu 2, sub-menu 1, option 1. Possible redundancy with setings.logData as we only have one sensor.
// settings.enableSD             : defines if the SD card should be enabled by beginSD(). Only set in settings.h. Doesn't appear to be able to be changed by the user? Possibly redundant?
// settings.enableTerminalOutput : indicates if the user has enabled Terminal logging - i.e. should the UBX Class and ID be displayed for every frame. Set by menu 1, option 2.
// setings.logData               : indicates if the user has enabled SD card logging. Set by menu 1, option 1.
// online.microSD                : indicates if the SD card is ready for data logging. Set by beginSD().
// online.dataLogging            : indicates if the SD card log file is open and ready to receive data. Set by beginDataLogging().
// qwiicAvailable.uBlox          : indicates if there is a u-blox module connected. Set by detectQwiicDevices().
// qwiicOnline.uBlox             : indicates if the module has been configured, or needs to be configured. Set true by beginSensors().

//Default u-blox I2C address
#define ADR_UBLOX 0x42

//u-blox settings
struct struct_uBlox {
  bool log = true;
  bool powerManagement = true;
  bool enableGPS = true;
  bool enableGLO = true;
  bool enableGAL = true;
  bool enableBDS = true;
  bool enableQZSS = true;
  uint8_t logUBXNAVPOSECEF = 0;
  uint8_t logUBXNAVSTATUS = 0;
  uint8_t logUBXNAVDOP = 0;
  uint8_t logUBXNAVATT = 0;
  uint8_t logUBXNAVPVT = 1; // Default to a PVT rate of 1
  uint8_t logUBXNAVODO = 0;
  uint8_t logUBXNAVVELECEF = 0;
  uint8_t logUBXNAVVELNED = 0;
  uint8_t logUBXNAVHPPOSECEF = 0;
  uint8_t logUBXNAVHPPOSLLH = 0;
  uint8_t logUBXNAVCLOCK = 0;
  uint8_t logUBXNAVRELPOSNED = 0;
  uint8_t logUBXRXMSFRBX = 0;
  uint8_t logUBXRXMRAWX = 0;
  uint8_t logUBXRXMMEASX = 0;
  uint8_t logUBXTIMTM2 = 0;
  uint8_t logUBXESFMEAS = 0;
  uint8_t logUBXESFRAW = 0;
  uint8_t logUBXESFSTATUS = 0;
  uint8_t logUBXESFALG = 0;
  uint8_t logUBXESFINS = 0;
  uint8_t logUBXHNRPVT = 0;
  uint8_t logUBXHNRATT = 0;
  uint8_t logUBXHNRINS = 0;
  uint8_t logNMEADTM = 0;
  uint8_t logNMEAGBS = 0;
  uint8_t logNMEAGGA = 0;
  uint8_t logNMEAGLL = 0;
  uint8_t logNMEAGNS = 0;
  uint8_t logNMEAGRS = 0;
  uint8_t logNMEAGSA = 0;
  uint8_t logNMEAGST = 0;
  uint8_t logNMEAGSV = 0;
  uint8_t logNMEARLM = 0;
  uint8_t logNMEARMC = 0;
  uint8_t logNMEATXT = 0;
  uint8_t logNMEAVLW = 0;
  uint8_t logNMEAVTG = 0;
  uint8_t logNMEAZDA = 0;
  uint8_t ubloxI2Caddress = ADR_UBLOX; //Let's store this just in case we want to change it at some point with CFG-I2C-ADDRESS (0x20510001)
  bool disableNMEAOnUART1 = false; //Set to true to disable NMEA on UART1 (#34)
};

//This is all the settings that can be set on OpenLog. It's recorded to NVM and the config file.
struct struct_settings {
  int sizeOfSettings = 0; //sizeOfSettings **must** be the first entry and must be int
  int olaIdentifier = OLA_IDENTIFIER; // olaIdentifier **must** be the second entry
  int nextDataLogNumber = 1;
  uint64_t usBetweenReadings = 1000000ULL; //1000,000us = 1000ms = 1 readings per second.
  uint64_t usLoggingDuration = 10000000ULL; //10,000,000us = 10s logging duration
  uint64_t usSleepDuration = 0ULL; //0us = do not sleep (continuous logging)
  bool openNewLogFile = true;
  bool enableSD = true;
  bool enableTerminalOutput = true;
  bool logData = true;
  int  serialTerminalBaudRate = 115200;
  bool showHelperText = true;
  bool printMajorDebugMessages = false;
  bool printMinorDebugMessages = false;
  bool powerDownQwiicBusBetweenReads = false; // 29 chars!
  int  qwiicBusMaxSpeed = 100000; // 400kHz with no pullups causes problems, so default to 100kHz. User can select 400 later if required.
  bool enablePwrLedDuringSleep = true;
  bool useGPIO32ForStopLogging = false; //If true, use GPIO as a stop logging button
  bool frequentFileAccessTimestamps = true; // If true, the log file is sync'd and the access timestamp is updated every second
  bool enableLowBatteryDetection = false; // Low battery detection
  float lowBatteryThreshold = 3.4; // Low battery voltage threshold (Volts)
  float vinCorrectionFactor = 1.47; //Correction factor for the VIN measurement; to compensate for the divider impedance
  uint8_t hnrNavigationRate = 1; //HNR Navigation Rate (if supported)
  bool printGNSSDebugMessages = false;
  uint8_t qwiicBusPullUps = 0; // Qwiic bus pull-up resistance: 0, 1(.5), 6, 12, 24 kOhms
  bool outputUBX = false; // Output the sensor UBX data on the TX pin
  bool outputNMEA = false; // Output the sensor NMEA data on the TX pin
  int  serialTXBaudRate = 115200;
  struct_uBlox sensor_uBlox;
} settings;

//These are the devices on board OpenLog that may be on or offline.
struct struct_online {
  bool microSD = false;
  bool dataLogging = false;
} online;

//These structs define supported sensors and if they are available and online(started).
struct struct_QwiicSensors {
  bool uBlox;
};

struct_QwiicSensors qwiicAvailable = {
  .uBlox = false,
};

struct_QwiicSensors qwiicOnline = {
  .uBlox = false,
};
