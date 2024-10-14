void loadSettings()
{
  //First load any settings from NVM
  //After, we'll load settings from config file if available
  //We'll then re-record settings so that the settings from the file over-rides internal NVM settings

  //Check to see if EEPROM is blank
  uint32_t testRead = 0;
  if (EEPROM.get(0, testRead) == 0xFFFFFFFF)
  {
    recordSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
    Serial.println(F("Default settings applied"));
  }

  //Check that the current settings struct size matches what is stored in EEPROM
  //Misalignment happens when we add a new feature or setting
  int tempSize = 0;
  EEPROM.get(0, tempSize); //Load the sizeOfSettings
  if (tempSize != sizeof(settings))
  {
    Serial.println(F("Settings wrong size. Default settings applied"));
    recordSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
  }

  //Check that the olaIdentifier is correct
  //(It is possible for two different versions of the code to have the same sizeOfSettings - which causes problems!)
  int tempIdentifier = 0;
  EEPROM.get(sizeof(int), tempIdentifier); //Load the identifier from the EEPROM location after sizeOfSettings (int)
  if (tempIdentifier != OLA_IDENTIFIER)
  {
    Serial.println("Settings are not valid for this variant of the OLA. Default settings applied");
    recordSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
  }

  //Read current settings
  EEPROM.get(0, settings);

  loadSettingsFromFile(); //Load any settings from config file. This will over-write any pre-existing EEPROM settings.
  //Record these new settings to EEPROM and config file to be sure they are the same
  //(do this even if loadSettingsFromFile returned false)
  recordSettings();
}

//Record the current settings struct to EEPROM and then to config file
void recordSettings()
{
  settings.sizeOfSettings = sizeof(settings);
  EEPROM.put(0, settings);
  recordSettingsToFile();
}

//Export the current settings to a config file
void recordSettingsToFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_GNSS_settings.cfg"))
      sd.remove("OLA_GNSS_settings.cfg");

    #if SD_FAT_TYPE == 1
    File32 settingsFile;
    #elif SD_FAT_TYPE == 2
    ExFile settingsFile;
    #elif SD_FAT_TYPE == 3
    FsFile settingsFile;
    #else // SD_FAT_TYPE == 0
    File settingsFile;
    #endif  // SD_FAT_TYPE
    if (settingsFile.open("OLA_GNSS_settings.cfg", O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create settings file");
      return;
    }

    settingsFile.println("sizeOfSettings=" + (String)settings.sizeOfSettings);
    settingsFile.println("olaIdentifier=" + (String)settings.olaIdentifier);
    settingsFile.println("nextDataLogNumber=" + (String)settings.nextDataLogNumber);

    // Convert uint64_t to string
    // Based on printLLNumber by robtillaart
    // https://forum.arduino.cc/index.php?topic=143584.msg1519824#msg1519824
    char tempTimeRev[20]; // Char array to hold to usBetweenReadings (reversed order)
    char tempTime[20]; // Char array to hold to usBetweenReadings (correct order)
    uint64_t usBR = settings.usBetweenReadings;
    unsigned int i = 0;
    if (usBR == 0ULL) // if usBetweenReadings is zero, set tempTime to "0"
    {
      tempTime[0] = '0';
      tempTime[1] = 0;
    }
    else
    {
      while (usBR > 0)
      {
        tempTimeRev[i++] = (usBR % 10) + '0'; // divide by 10, convert the remainder to char
        usBR /= 10; // divide by 10
      }
      unsigned int j = 0;
      while (i > 0)
      {
        tempTime[j++] = tempTimeRev[--i]; // reverse the order
        tempTime[j] = 0; // mark the end with a NULL
      }
    }
    
    settingsFile.println("usBetweenReadings=" + (String)tempTime);

    usBR = settings.usLoggingDuration;
    i = 0;
    if (usBR == 0ULL) // if usLoggingDuration is zero, set tempTime to "0"
    {
      tempTime[0] = '0';
      tempTime[1] = 0;
    }
    else
    {
      while (usBR > 0)
      {
        tempTimeRev[i++] = (usBR % 10) + '0'; // divide by 10, convert the remainder to char
        usBR /= 10; // divide by 10
      }
      unsigned int j = 0;
      while (i > 0)
      {
        tempTime[j++] = tempTimeRev[--i]; // reverse the order
        tempTime[j] = 0; // mark the end with a NULL
      }
    }
    
    settingsFile.println("usLoggingDuration=" + (String)tempTime);

    usBR = settings.usSleepDuration;
    i = 0;
    if (usBR == 0ULL) // if usSleepDuration is zero, set tempTime to "0"
    {
      tempTime[0] = '0';
      tempTime[1] = 0;
    }
    else
    {
      while (usBR > 0)
      {
        tempTimeRev[i++] = (usBR % 10) + '0'; // divide by 10, convert the remainder to char
        usBR /= 10; // divide by 10
      }
      unsigned int j = 0;
      while (i > 0)
      {
        tempTime[j++] = tempTimeRev[--i]; // reverse the order
        tempTime[j] = 0; // mark the end with a NULL
      }
    }
    
    settingsFile.println("usSleepDuration=" + (String)tempTime);

    settingsFile.println("openNewLogFile=" + (String)settings.openNewLogFile);
    settingsFile.println("enableSD=" + (String)settings.enableSD);
    settingsFile.println("enableTerminalOutput=" + (String)settings.enableTerminalOutput);
    settingsFile.println("logData=" + (String)settings.logData);
    settingsFile.println("serialTerminalBaudRate=" + (String)settings.serialTerminalBaudRate);
    settingsFile.println("showHelperText=" + (String)settings.showHelperText);
    settingsFile.println("printMajorDebugMessages=" + (String)settings.printMajorDebugMessages);
    settingsFile.println("printMinorDebugMessages=" + (String)settings.printMinorDebugMessages);
    settingsFile.println("powerDownQwiicBusBetweenReads=" + (String)settings.powerDownQwiicBusBetweenReads);
    settingsFile.println("qwiicBusMaxSpeed=" + (String)settings.qwiicBusMaxSpeed);
    settingsFile.println("enablePwrLedDuringSleep=" + (String)settings.enablePwrLedDuringSleep);
    settingsFile.println("useGPIO32ForStopLogging=" + (String)settings.useGPIO32ForStopLogging);
    settingsFile.println("frequentFileAccessTimestamps=" + (String)settings.frequentFileAccessTimestamps);
    settingsFile.println("enableLowBatteryDetection=" + (String)settings.enableLowBatteryDetection);
    settingsFile.print("lowBatteryThreshold="); settingsFile.println(settings.lowBatteryThreshold);
    settingsFile.print("vinCorrectionFactor="); settingsFile.println(settings.vinCorrectionFactor);
    settingsFile.print("hnrNavigationRate="); settingsFile.println(settings.hnrNavigationRate);
    settingsFile.print("printGNSSDebugMessages="); settingsFile.println(settings.printGNSSDebugMessages);
    settingsFile.print("qwiicBusPullUps="); settingsFile.println(settings.qwiicBusPullUps);
    settingsFile.println("outputUBX=" + (String)settings.outputUBX);
    settingsFile.println("outputNMEA=" + (String)settings.outputNMEA);
    settingsFile.println("serialTXBaudRate=" + (String)settings.serialTXBaudRate);

    settingsFile.print("GNSS:log="); settingsFile.println(settings.sensor_uBlox.log);
    settingsFile.print("GNSS:powerManagement="); settingsFile.println(settings.sensor_uBlox.powerManagement);
    settingsFile.print("GNSS:enableGPS="); settingsFile.println(settings.sensor_uBlox.enableGPS);
    settingsFile.print("GNSS:enableGLO="); settingsFile.println(settings.sensor_uBlox.enableGLO);
    settingsFile.print("GNSS:enableGAL="); settingsFile.println(settings.sensor_uBlox.enableGAL);
    settingsFile.print("GNSS:enableBDS="); settingsFile.println(settings.sensor_uBlox.enableBDS);
    settingsFile.print("GNSS:enableQZSS="); settingsFile.println(settings.sensor_uBlox.enableQZSS);
    settingsFile.print("GNSS:logUBXNAVPOSECEF="); settingsFile.println(settings.sensor_uBlox.logUBXNAVPOSECEF);
    settingsFile.print("GNSS:logUBXNAVSTATUS="); settingsFile.println(settings.sensor_uBlox.logUBXNAVSTATUS);
    settingsFile.print("GNSS:logUBXNAVDOP="); settingsFile.println(settings.sensor_uBlox.logUBXNAVDOP);
    settingsFile.print("GNSS:logUBXNAVATT="); settingsFile.println(settings.sensor_uBlox.logUBXNAVATT);
    settingsFile.print("GNSS:logUBXNAVPVT="); settingsFile.println(settings.sensor_uBlox.logUBXNAVPVT);
    settingsFile.print("GNSS:logUBXNAVODO="); settingsFile.println(settings.sensor_uBlox.logUBXNAVODO);
    settingsFile.print("GNSS:logUBXNAVVELECEF="); settingsFile.println(settings.sensor_uBlox.logUBXNAVVELECEF);
    settingsFile.print("GNSS:logUBXNAVVELNED="); settingsFile.println(settings.sensor_uBlox.logUBXNAVVELNED);
    settingsFile.print("GNSS:logUBXNAVHPPOSECEF="); settingsFile.println(settings.sensor_uBlox.logUBXNAVHPPOSECEF);
    settingsFile.print("GNSS:logUBXNAVHPPOSLLH="); settingsFile.println(settings.sensor_uBlox.logUBXNAVHPPOSLLH);
    settingsFile.print("GNSS:logUBXNAVCLOCK="); settingsFile.println(settings.sensor_uBlox.logUBXNAVCLOCK);
    settingsFile.print("GNSS:logUBXNAVRELPOSNED="); settingsFile.println(settings.sensor_uBlox.logUBXNAVRELPOSNED);
    settingsFile.print("GNSS:logUBXRXMSFRBX="); settingsFile.println(settings.sensor_uBlox.logUBXRXMSFRBX);
    settingsFile.print("GNSS:logUBXRXMRAWX="); settingsFile.println(settings.sensor_uBlox.logUBXRXMRAWX);
    settingsFile.print("GNSS:logUBXRXMMEASX="); settingsFile.println(settings.sensor_uBlox.logUBXRXMMEASX);
    settingsFile.print("GNSS:logUBXTIMTM2="); settingsFile.println(settings.sensor_uBlox.logUBXTIMTM2);
    settingsFile.print("GNSS:logUBXESFMEAS="); settingsFile.println(settings.sensor_uBlox.logUBXESFMEAS);
    settingsFile.print("GNSS:logUBXESFRAW="); settingsFile.println(settings.sensor_uBlox.logUBXESFRAW);
    settingsFile.print("GNSS:logUBXESFSTATUS="); settingsFile.println(settings.sensor_uBlox.logUBXESFSTATUS);
    settingsFile.print("GNSS:logUBXESFALG="); settingsFile.println(settings.sensor_uBlox.logUBXESFALG);
    settingsFile.print("GNSS:logUBXESFINS="); settingsFile.println(settings.sensor_uBlox.logUBXESFINS);
    settingsFile.print("GNSS:logUBXHNRPVT="); settingsFile.println(settings.sensor_uBlox.logUBXHNRPVT);
    settingsFile.print("GNSS:logUBXHNRATT="); settingsFile.println(settings.sensor_uBlox.logUBXHNRATT);
    settingsFile.print("GNSS:logUBXHNRINS="); settingsFile.println(settings.sensor_uBlox.logUBXHNRINS);
    settingsFile.print("GNSS:logNMEADTM="); settingsFile.println(settings.sensor_uBlox.logNMEADTM);
    settingsFile.print("GNSS:logNMEAGBS="); settingsFile.println(settings.sensor_uBlox.logNMEAGBS);
    settingsFile.print("GNSS:logNMEAGGA="); settingsFile.println(settings.sensor_uBlox.logNMEAGGA);
    settingsFile.print("GNSS:logNMEAGLL="); settingsFile.println(settings.sensor_uBlox.logNMEAGLL);
    settingsFile.print("GNSS:logNMEAGNS="); settingsFile.println(settings.sensor_uBlox.logNMEAGNS);
    settingsFile.print("GNSS:logNMEAGRS="); settingsFile.println(settings.sensor_uBlox.logNMEAGRS);
    settingsFile.print("GNSS:logNMEAGSA="); settingsFile.println(settings.sensor_uBlox.logNMEAGSA);
    settingsFile.print("GNSS:logNMEAGST="); settingsFile.println(settings.sensor_uBlox.logNMEAGST);
    settingsFile.print("GNSS:logNMEAGSV="); settingsFile.println(settings.sensor_uBlox.logNMEAGSV);
    settingsFile.print("GNSS:logNMEARLM="); settingsFile.println(settings.sensor_uBlox.logNMEARLM);
    settingsFile.print("GNSS:logNMEARMC="); settingsFile.println(settings.sensor_uBlox.logNMEARMC);
    settingsFile.print("GNSS:logNMEATXT="); settingsFile.println(settings.sensor_uBlox.logNMEATXT);
    settingsFile.print("GNSS:logNMEAVLW="); settingsFile.println(settings.sensor_uBlox.logNMEAVLW);
    settingsFile.print("GNSS:logNMEAVTG="); settingsFile.println(settings.sensor_uBlox.logNMEAVTG);
    settingsFile.print("GNSS:logNMEAZDA="); settingsFile.println(settings.sensor_uBlox.logNMEAZDA);
    settingsFile.print("GNSS:ubloxI2Caddress="); settingsFile.println(settings.sensor_uBlox.ubloxI2Caddress);
    settingsFile.print("GNSS:disableNMEAOnUART1="); settingsFile.println(settings.sensor_uBlox.disableNMEAOnUART1);

    updateDataFileAccess(&settingsFile); // Update the file access time & date
    settingsFile.close();
  }
}

//If a config file exists on the SD card, load them and overwrite the local settings
//Heavily based on ReadCsvFile from SdFat library
//Returns true if some settings were loaded from a file
//Returns false if a file was not opened/loaded
bool loadSettingsFromFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_GNSS_settings.cfg"))
    {
      #if SD_FAT_TYPE == 1
      File32 settingsFile;
      #elif SD_FAT_TYPE == 2
      ExFile settingsFile;
      #elif SD_FAT_TYPE == 3
      FsFile settingsFile;
      #else // SD_FAT_TYPE == 0
      File settingsFile;
      #endif  // SD_FAT_TYPE
      if (settingsFile.open("OLA_GNSS_settings.cfg", O_READ) == false)
      {
        Serial.println("Failed to open settings file");
        return (false);
      }

      char line[60];
      int lineNumber = 0;

      while (settingsFile.available()) {
        int n = settingsFile.fgets(line, sizeof(line));
        if (n <= 0) {
          Serial.printf("Failed to read line %d from settings file\r\n", lineNumber);
        }
        else if (line[n - 1] != '\n' && n == (sizeof(line) - 1)) {
          Serial.printf("Settings line %d too long\r\n", lineNumber);
          if (lineNumber == 0)
          {
            //If we can't read the first line of the settings file, give up
            Serial.println("Giving up on settings file");
            return (false);
          }
        }
        else if (parseLine(line) == false) {
          Serial.printf("Failed to parse line %d: %s\r\n", lineNumber, line);
          if (lineNumber == 0)
          {
            //If we can't read the first line of the settings file, give up
            Serial.println("Giving up on settings file");
            return (false);
          }
        }

        lineNumber++;
      }

      Serial.println("Config file read complete");
      settingsFile.close();
      return (true);
    }
    else
    {
      Serial.println("No config file found. Using settings from EEPROM.");
      //The defaults of the struct will be recorded to a file later on.
      return (false);
    }
  }

  Serial.println("Config file read failed: SD offline");
  return (false); //SD offline
}

// Check for extra characters in field or find minus sign.
char* skipSpace(char* str) {
  while (isspace(*str)) str++;
  return str;
}

//Convert a given line from file into a settingName and value
//Sets the setting if the name is known
bool parseLine(char* str) {
  char* ptr;

  //Debug
  //Serial.printf("Line contents: %s", str);
  //Serial.flush();

  // Set strtok start of line.
  str = strtok(str, "=");
  if (!str) return false;

  //Store this setting name
  char settingName[40];
  sprintf(settingName, "%s", str);

  //Move pointer to end of line
  str = strtok(nullptr, "\n");
  if (!str) return false;

  //Serial.printf("s = %s\r\n", str);
  //Serial.flush();

  // Convert string to double.
  double d = strtod(str, &ptr);
  if (str == ptr || *skipSpace(ptr)) return false;

  //Serial.printf("d = %lf\r\n", d);
  //Serial.flush();

  // Get setting name
  if (strcmp(settingName, "sizeOfSettings") == 0)
  {
    //We may want to cause a factory reset from the settings file rather than the menu
    //If user sets sizeOfSettings to -1 in config file, OLA will factory reset
    if (d == -1)
    {
      EEPROM.erase();
      sd.remove("OLA_GNSS_settings.cfg");
      Serial.println("OpenLog Artemis has been factory reset. Freezing. Please restart and open terminal at 115200bps.");
      while(1);
    }

    //Check to see if this setting file is compatible with this version of OLA
    if (d != sizeof(settings))
      Serial.printf("Warning: Settings size is %d but current firmware expects %d. Attempting to use settings from file.\r\n", d, sizeof(settings));

  }
  else if (strcmp(settingName, "olaIdentifier") == 0)
    settings.olaIdentifier = d;
  else if (strcmp(settingName, "nextDataLogNumber") == 0)
    settings.nextDataLogNumber = d;
  else if (strcmp(settingName, "usBetweenReadings") == 0)
    settings.usBetweenReadings = d;
  else if (strcmp(settingName, "usLoggingDuration") == 0)
    settings.usLoggingDuration = d;
  else if (strcmp(settingName, "usSleepDuration") == 0)
    settings.usSleepDuration = d;
  else if (strcmp(settingName, "openNewLogFile") == 0)
    settings.openNewLogFile = d;
  else if (strcmp(settingName, "enableSD") == 0)
    settings.enableSD = d;
  else if (strcmp(settingName, "enableTerminalOutput") == 0)
    settings.enableTerminalOutput = d;
  else if (strcmp(settingName, "logData") == 0)
    settings.logData = d;
  else if (strcmp(settingName, "serialTerminalBaudRate") == 0)
    settings.serialTerminalBaudRate = d;
  else if (strcmp(settingName, "showHelperText") == 0)
    settings.showHelperText = d;
  else if (strcmp(settingName, "printMajorDebugMessages") == 0)
    settings.printMajorDebugMessages = d;
  else if (strcmp(settingName, "printMinorDebugMessages") == 0)
    settings.printMinorDebugMessages = d;
  else if (strcmp(settingName, "powerDownQwiicBusBetweenReads") == 0)
    settings.powerDownQwiicBusBetweenReads = d;
  else if (strcmp(settingName, "qwiicBusMaxSpeed") == 0)
    settings.qwiicBusMaxSpeed = d;
  else if (strcmp(settingName, "enablePwrLedDuringSleep") == 0)
    settings.enablePwrLedDuringSleep = d;
  else if (strcmp(settingName, "useGPIO32ForStopLogging") == 0)
    settings.useGPIO32ForStopLogging = d;
  else if (strcmp(settingName, "frequentFileAccessTimestamps") == 0)
    settings.frequentFileAccessTimestamps = d;
  else if (strcmp(settingName, "enableLowBatteryDetection") == 0)
    settings.enableLowBatteryDetection = d;
  else if (strcmp(settingName, "lowBatteryThreshold") == 0)
    settings.lowBatteryThreshold = d;
  else if (strcmp(settingName, "vinCorrectionFactor") == 0)
    settings.vinCorrectionFactor = d;
  else if (strcmp(settingName, "hnrNavigationRate") == 0)
    settings.hnrNavigationRate = d;
  else if (strcmp(settingName, "printGNSSDebugMessages") == 0)
    settings.printGNSSDebugMessages = d;
  else if (strcmp(settingName, "qwiicBusPullUps") == 0)
    settings.qwiicBusPullUps = d;
  else if (strcmp(settingName, "outputUBX") == 0)
    settings.outputUBX = d;
  else if (strcmp(settingName, "outputNMEA") == 0)
    settings.outputNMEA = d;
  else if (strcmp(settingName, "serialTXBaudRate") == 0)
    settings.serialTXBaudRate = d;

  else if (strcmp(settingName, "GNSS:log") == 0)
    settings.sensor_uBlox.log = d;
  else if (strcmp(settingName, "GNSS:powerManagement") == 0)
    settings.sensor_uBlox.powerManagement = d;
  else if (strcmp(settingName, "GNSS:enableGPS") == 0)
    settings.sensor_uBlox.enableGPS = d;
  else if (strcmp(settingName, "GNSS:enableGLO") == 0)
    settings.sensor_uBlox.enableGLO = d;
  else if (strcmp(settingName, "GNSS:enableGAL") == 0)
    settings.sensor_uBlox.enableGAL = d;
  else if (strcmp(settingName, "GNSS:enableBDS") == 0)
    settings.sensor_uBlox.enableBDS = d;
  else if (strcmp(settingName, "GNSS:enableQZSS") == 0)
    settings.sensor_uBlox.enableQZSS = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVPOSECEF") == 0)
    settings.sensor_uBlox.logUBXNAVPOSECEF = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVSTATUS") == 0)
    settings.sensor_uBlox.logUBXNAVSTATUS = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVDOP") == 0)
    settings.sensor_uBlox.logUBXNAVDOP = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVATT") == 0)
    settings.sensor_uBlox.logUBXNAVATT = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVPVT") == 0)
    settings.sensor_uBlox.logUBXNAVPVT = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVODO") == 0)
    settings.sensor_uBlox.logUBXNAVODO = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVVELECEF") == 0)
    settings.sensor_uBlox.logUBXNAVVELECEF = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVVELNED") == 0)
    settings.sensor_uBlox.logUBXNAVVELNED = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVHPPOSECEF") == 0)
    settings.sensor_uBlox.logUBXNAVHPPOSECEF = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVHPPOSLLH") == 0)
    settings.sensor_uBlox.logUBXNAVHPPOSLLH = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVCLOCK") == 0)
    settings.sensor_uBlox.logUBXNAVCLOCK = d;
  else if (strcmp(settingName, "GNSS:logUBXNAVRELPOSNED") == 0)
    settings.sensor_uBlox.logUBXNAVRELPOSNED = d;
  else if (strcmp(settingName, "GNSS:logUBXRXMSFRBX") == 0)
    settings.sensor_uBlox.logUBXRXMSFRBX = d;
  else if (strcmp(settingName, "GNSS:logUBXRXMRAWX") == 0)
    settings.sensor_uBlox.logUBXRXMRAWX = d;
  else if (strcmp(settingName, "GNSS:logUBXRXMMEASX") == 0)
    settings.sensor_uBlox.logUBXRXMMEASX = d;
  else if (strcmp(settingName, "GNSS:logUBXTIMTM2") == 0)
    settings.sensor_uBlox.logUBXTIMTM2 = d;
  else if (strcmp(settingName, "GNSS:logUBXESFMEAS") == 0)
    settings.sensor_uBlox.logUBXESFMEAS = d;
  else if (strcmp(settingName, "GNSS:logUBXESFRAW") == 0)
    settings.sensor_uBlox.logUBXESFRAW = d;
  else if (strcmp(settingName, "GNSS:logUBXESFSTATUS") == 0)
    settings.sensor_uBlox.logUBXESFSTATUS = d;
  else if (strcmp(settingName, "GNSS:logUBXESFALG") == 0)
    settings.sensor_uBlox.logUBXESFALG = d;
  else if (strcmp(settingName, "GNSS:logUBXESFINS") == 0)
    settings.sensor_uBlox.logUBXESFINS = d;
  else if (strcmp(settingName, "GNSS:logUBXHNRPVT") == 0)
    settings.sensor_uBlox.logUBXHNRPVT = d;
  else if (strcmp(settingName, "GNSS:logUBXHNRATT") == 0)
    settings.sensor_uBlox.logUBXHNRATT = d;
  else if (strcmp(settingName, "GNSS:logUBXHNRINS") == 0)
    settings.sensor_uBlox.logUBXHNRINS = d;
  else if (strcmp(settingName, "GNSS:logNMEADTM") == 0)
    settings.sensor_uBlox.logNMEADTM = d;
  else if (strcmp(settingName, "GNSS:logNMEAGBS") == 0)
    settings.sensor_uBlox.logNMEAGBS = d;
  else if (strcmp(settingName, "GNSS:logNMEAGGA") == 0)
    settings.sensor_uBlox.logNMEAGGA = d;
  else if (strcmp(settingName, "GNSS:logNMEAGLL") == 0)
    settings.sensor_uBlox.logNMEAGLL = d;
  else if (strcmp(settingName, "GNSS:logNMEAGNS") == 0)
    settings.sensor_uBlox.logNMEAGNS = d;
  else if (strcmp(settingName, "GNSS:logNMEAGRS") == 0)
    settings.sensor_uBlox.logNMEAGRS = d;
  else if (strcmp(settingName, "GNSS:logNMEAGSA") == 0)
    settings.sensor_uBlox.logNMEAGSA = d;
  else if (strcmp(settingName, "GNSS:logNMEAGST") == 0)
    settings.sensor_uBlox.logNMEAGST = d;
  else if (strcmp(settingName, "GNSS:logNMEAGSV") == 0)
    settings.sensor_uBlox.logNMEAGSV = d;
  else if (strcmp(settingName, "GNSS:logNMEARLM") == 0)
    settings.sensor_uBlox.logNMEARLM = d;
  else if (strcmp(settingName, "GNSS:logNMEARMC") == 0)
    settings.sensor_uBlox.logNMEARMC = d;
  else if (strcmp(settingName, "GNSS:logNMEATXT") == 0)
    settings.sensor_uBlox.logNMEATXT = d;
  else if (strcmp(settingName, "GNSS:logNMEAVLW") == 0)
    settings.sensor_uBlox.logNMEAVLW = d;
  else if (strcmp(settingName, "GNSS:logNMEAVTG") == 0)
    settings.sensor_uBlox.logNMEAVTG = d;
  else if (strcmp(settingName, "GNSS:logNMEAZDA") == 0)
    settings.sensor_uBlox.logNMEAZDA = d;
  else if (strcmp(settingName, "GNSS:ubloxI2Caddress") == 0)
    settings.sensor_uBlox.ubloxI2Caddress = d;
  else if (strcmp(settingName, "GNSS:disableNMEAOnUART1") == 0)
    settings.sensor_uBlox.disableNMEAOnUART1 = d;

  else
  {
    Serial.print("Unknown setting: ");
    Serial.println(settingName);
  }

  return (true);
}
