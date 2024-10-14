//Init / begin comm with all enabled sensors
bool beginSensors()
{
  //If no sensors are available then return
  if (detectQwiicDevices() == false) //Sets Wire to 100kHz
  {
    if (settings.printMajorDebugMessages == true)
    {
      Serial.println(F("beginSensors: no qwiic devices detected!")); 
    }       
    qwiicOnline.uBlox = false;
    return (false);
  }

  Serial.println(F("Detecting and configuring the GNSS. This will take a few seconds..."));

  setQwiicPullups(); //Just in case the pull-ups have been changed, set pullups here too
  determineMaxI2CSpeed(); //Try for 400kHz but reduce if the user has selected a slower speed

  if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && ((!qwiicOnline.uBlox) || (gnssSettingsChanged == true))) // Only do this if the sensor is not online
  {
    gnssSettingsChanged = false;
    gpsSensor_ublox.setFileBufferSize(FILE_BUFFER_SIZE); // setFileBufferSize must be called _before_ .begin
    
    gpsSensor_ublox.setPacketCfgPayloadSize(MAX_PAYLOAD_SIZE > 8192 ? MAX_PAYLOAD_SIZE : 8192); // Needs to be large enough to hold a full RAWX frame

    if (settings.printGNSSDebugMessages) //Enable debug messages if desired
      gpsSensor_ublox.enableDebugging(Serial, !settings.printMinorDebugMessages);

    if (gpsSensor_ublox.begin(qwiic, settings.sensor_uBlox.ubloxI2Caddress) == true) //Wire port, Address. Default is 0x42.
    {
      // Try up to three times to get the module info
      if (gpsSensor_ublox.getModuleInfo(1100) == false) // Try to get the module info
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.println(F("beginSensors: first getModuleInfo call failed. Trying again...")); 
        }       
        if (gpsSensor_ublox.getModuleInfo(1100) == false) // Try to get the module info
        {
          if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: second getModuleInfo call failed. Trying again...")); 
          }       
          if (gpsSensor_ublox.getModuleInfo(1100) == false) // Try to get the module info
          {
            if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: third getModuleInfo call failed! Giving up..."));
              qwiicOnline.uBlox = false;
              return (false);
            }
          }
        }
      }

      // Print the PROTVER
      if (settings.printMajorDebugMessages == true)
      {
        Serial.print(F("beginSensors: GNSS module found: PROTVER="));
        Serial.print(gpsSensor_ublox.getProtocolVersionHigh());
        Serial.print(F("."));
        Serial.print(gpsSensor_ublox.getProtocolVersionLow());
        Serial.print(F(" FWVER="));
        Serial.print(gpsSensor_ublox.getFirmwareVersionHigh());
        Serial.print(F("."));
        Serial.print(gpsSensor_ublox.getFirmwareVersionLow());
        Serial.print(F(" MOD="));
        Serial.println(gpsSensor_ublox.getModuleName());
      }

      //Disable all messages
      disableMessages(1100);

      //Check if any UBX messages are enabled
      bool ubxRequired = true; // UBX is always required for ACK/NACK

      //Check if any NMEA messaged are enabled
      bool nmeaRequired = settings.sensor_uBlox.logNMEADTM | settings.sensor_uBlox.logNMEAGBS
        | settings.sensor_uBlox.logNMEAGGA | settings.sensor_uBlox.logNMEAGLL | settings.sensor_uBlox.logNMEAGNS
        | settings.sensor_uBlox.logNMEAGRS | settings.sensor_uBlox.logNMEAGSA | settings.sensor_uBlox.logNMEAGST
        | settings.sensor_uBlox.logNMEAGSV | settings.sensor_uBlox.logNMEARLM | settings.sensor_uBlox.logNMEARMC
        | settings.sensor_uBlox.logNMEATXT | settings.sensor_uBlox.logNMEAVLW
        | settings.sensor_uBlox.logNMEAVTG | settings.sensor_uBlox.logNMEAZDA;

      //Set the I2C port to output the required protocols
      uint8_t requiredProtocols = 0;
      if (ubxRequired)
        requiredProtocols |= COM_TYPE_UBX;
      if (nmeaRequired)
        requiredProtocols |= COM_TYPE_NMEA;
      if (!gpsSensor_ublox.setI2COutput(requiredProtocols))
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: setI2COutput failed!")); 
          }       
      }

      //Disable NMEA on UART1 if desired
      if (settings.sensor_uBlox.disableNMEAOnUART1)
      {
        if (!gpsSensor_ublox.setUART1Output(COM_TYPE_UBX))
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: setUART1Output failed!")); 
            }       
        }
      }

      //Save the port configuration - causes the IO system to reset!
      gpsSensor_ublox.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);
      delay(2100);
      
      // Enable/Disable the selected constellations
      // Let's do this before we set the message rate and enable messages
      enableConstellations(2100);
      
      //Set output rate
      gpsSensor_ublox.setMeasurementRate((uint16_t)(settings.usBetweenReadings / 1000ULL));

      //Set the HNR rate
      gpsSensor_ublox.setHNRNavigationRate(settings.hnrNavigationRate);

      //Configure UBX and NMEA output
      if(settings.outputUBX)
        gpsSensor_ublox.setUBXOutputPort(Serial1);
      if(settings.outputNMEA)
        gpsSensor_ublox.setNMEAOutputPort(Serial1);

      //Enable the selected messages
      enableMessages(1100);

      qwiicOnline.uBlox = true;
    }
  }

  return(true);
}

//Let's see if we can find a u-blox module on the I2C bus
bool detectQwiicDevices()
{
  bool somethingDetected = false;

  qwiic.setClock(100000); //During detection, go slow

  //qwiic.setPullups(0); //Disable pull-ups as the u-blox modules have their own pull-ups (commented by PaulZC - beginQwiic does this instead)

  //Depending on what hardware is configured, the Qwiic bus may have only been turned on a few ms ago
  //Give sensors, specifically those with a low I2C address, time to turn on
  for (int i = 0; i < 750; i++) // ZED-F9P requries ~1s to turn on
  {
    checkBattery(); // Check for low battery
    delay(1);
  }

  uint8_t address = settings.sensor_uBlox.ubloxI2Caddress;
  
  qwiic.beginTransmission(address);
  if (qwiic.endTransmission() == 0)
  {
    if (settings.printMinorDebugMessages == true)
    {
      Serial.printf("detectQwiicDevices: device found at address 0x%02X\r\n", address);
    }
    SFE_UBLOX_GNSS tempGNSS;
    if (settings.printGNSSDebugMessages) //Enable debug messages if desired
      tempGNSS.enableDebugging(Serial, !settings.printMinorDebugMessages);
    if (tempGNSS.begin(qwiic, address) == true) //Wire port, address
    {
      if (settings.printMinorDebugMessages == true)
      {
        Serial.printf("detectQwiicDevices: u-blox GNSS found at address 0x%02X\r\n", address);
      }
      qwiicAvailable.uBlox = true;
      somethingDetected = true;
    }
  }

  return (somethingDetected);
}

//Close the current log file and open a new one
//This should probably be defined in OpenLog_Artemis_GNSS_Logging as it involves files
//but it is defined here as it is u-blox-specific
void openNewLogFile()
{
  if (settings.logData && settings.sensor_uBlox.log && online.microSD && online.dataLogging) //If we are logging
  {
    if (qwiicAvailable.uBlox && qwiicOnline.uBlox) //If the u-blox is available and logging
    {
      //Disable all messages
      disableMessages(1100);
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait > 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData();
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
      storeFinalData();
      gnssDataFile.sync();

      updateDataFileAccess(&gnssDataFile); //Update the file access time stamp

      gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098

      strcpy(gnssDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));

      // O_CREAT - create the file if it does not exist
      // O_APPEND - seek to the end of the file prior to each write
      // O_WRITE - open for write
      if (gnssDataFile.open(gnssDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.println(F("openNewLogFile: failed to create new sensor data file"));
        }       
        
        online.dataLogging = false;
        return;
      }

      updateDataFileCreate(&gnssDataFile); //Update the file create time stamp

      //(Re)Enable the selected messages
      enableMessages(1100);
    }
  }
}

//Close the current log file and do not open a new one
//This should probably be defined in OpenLog_Artemis_GNSS_Logging as it involves files
//but it is defined here as it is u-blox-specific
void closeLogFile()
{
  if (settings.logData && settings.sensor_uBlox.log && online.microSD && online.dataLogging) //If we are logging
  {
    if (qwiicAvailable.uBlox && qwiicOnline.uBlox) //If the u-blox is available and logging
    {
      //Disable all messages
      disableMessages(1100);
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait > 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData();
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
      storeFinalData();
      gnssDataFile.sync();

      updateDataFileAccess(&gnssDataFile); //Update the file access time stamp

      gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
    }
  }
}

//Close the current log file and then reset the GNSS
void resetGNSS()
{
  if (settings.logData && settings.sensor_uBlox.log && online.microSD && online.dataLogging) //If we are logging
  {
    if (qwiicAvailable.uBlox && qwiicOnline.uBlox) //If the u-blox is available and logging
    {
      //Disable all messages
      disableMessages(1100);
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait >> 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData();
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
      storeFinalData();
      gnssDataFile.sync();

      updateDataFileAccess(&gnssDataFile); //Update the file access time stamp

      gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098

      //Reset the GNSS
      gpsSensor_ublox.factoryDefault(2100);

      //Wait 5 secs
      Serial.print(F("GNSS has been reset. Waiting 5 seconds."));
      for (int i = 0; i < 5; i++)
      {
        for (int j = 0; j < 1000; j++)
        {
          checkBattery(); // Check for low battery
          delay(1);
        }
        Serial.print(F("."));
      }
    }
  }
}

void disableMessages(uint16_t maxWait)
{
  Serial.println(F("Disabling all GNSS messages. This will take a few seconds..."));

  //Disable terminal output when configuring messages
  bool prevTerminalOutput = settings.enableTerminalOutput;
  settings.enableTerminalOutput = false;

  //gpsSensor_ublox.setAutoNAVPOSECEFcallback(&callbackNAVPOSECEF, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVPOSECEFrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVPOSECEF(false);

  //gpsSensor_ublox.setAutoNAVSTATUScallback(&callbackNAVSTATUS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVSTATUSrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVSTATUS(false);

  //gpsSensor_ublox.setAutoDOPcallback(&callbackNAVDOP, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoDOPrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVDOP(false);

  //gpsSensor_ublox.setAutoNAVATTcallback(&callbackNAVATT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVATTrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVATT(false);

  //gpsSensor_ublox.setAutoPVTcallback(&callbackNAVPVT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoPVTrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVPVT(false);

  //gpsSensor_ublox.setAutoNAVODOcallback(&callbackNAVODO, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVODOrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVODO(false);

  //gpsSensor_ublox.setAutoNAVVELECEFcallback(&callbackNAVVELECEF, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVVELECEFrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVVELECEF(false);

  //gpsSensor_ublox.setAutoNAVVELNEDcallback(&callbackNAVVELNED, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVVELNEDrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVVELNED(false);

  //gpsSensor_ublox.setAutoNAVHPPOSECEFcallback(&callbackNAVHPPOSECEF, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVHPPOSECEFrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVHPPOSECEF(false);

  //gpsSensor_ublox.setAutoHPPOSLLHcallback(&callbackNAVHPPOSLLH, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoHPPOSLLHrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVHPPOSLLH(false);

  //gpsSensor_ublox.setAutoNAVCLOCKcallback(&callbackNAVCLOCK, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoNAVCLOCKrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVCLOCK(false);

  //gpsSensor_ublox.setAutoRELPOSNEDcallback(&callbackNAVRELPOSNED, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoRELPOSNEDrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVRELPOSNED(false);

  //gpsSensor_ublox.setAutoRXMSFRBXcallback(&callbackRXMSFRBX, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoRXMSFRBXrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logRXMSFRBX(false);

  //gpsSensor_ublox.setAutoRXMRAWXcallback(&callbackRXMRAWX, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoRXMRAWXrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logRXMRAWX(false);

  //gpsSensor_ublox.setAutoRXMMEASXcallback(&callbackRXMMEASX, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoRXMMEASXrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logRXMMEASX(false);

  //gpsSensor_ublox.setAutoTIMTM2callback(&callbackTIMTM2, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoTIMTM2rate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logTIMTM2(false);

  //gpsSensor_ublox.setAutoESFALGcallback(&callbackESFALG, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoESFALGrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFALG(false);

  //gpsSensor_ublox.setAutoESFINScallback(&callbackESFINS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoESFINSrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFINS(false);

  //gpsSensor_ublox.setAutoESFMEAScallback(&callbackESFMEAS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoESFMEASrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFMEAS(false);

  //gpsSensor_ublox.setAutoESFRAWcallback(&callbackESFRAW, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoESFRAWrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFRAW(false);

  //gpsSensor_ublox.setAutoESFSTATUScallback(&callbackESFSTATUS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoESFSTATUSrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFSTATUS(false);

  //gpsSensor_ublox.setAutoHNRPVTcallback(&callbackHNRPVT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoHNRPVTrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logHNRPVT(false);

  //gpsSensor_ublox.setAutoHNRATTcallback(&callbackHNRATT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoHNRATTrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logHNRATT(false);

  //gpsSensor_ublox.setAutoHNRINScallback(&callbackHNRINS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.setAutoHNRINSrate(0, false, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logHNRINS(false);

  gpsSensor_ublox.newCfgValset(VAL_LAYER_RAM_BBR); // Use cfgValset to disable individual NMEA messages
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_DTM_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GBS_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GGA_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GLL_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GNS_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GRS_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GSA_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GST_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GSV_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_RLM_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_RMC_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_INFMSG_NMEA_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_VLW_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_VTG_I2C, 0);
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_ZDA_I2C, 0);
  gpsSensor_ublox.sendCfgValset(maxWait); // Send the configuration VALSET
  
  gpsSensor_ublox.setNMEALoggingMask(0);

  settings.enableTerminalOutput = prevTerminalOutput;
}

void enableMessages(uint16_t maxWait)
{
  Serial.println(F("Enabling GNSS messages. This will take a few seconds..."));

  //Disable terminal output when configuring messages
  bool prevTerminalOutput = settings.enableTerminalOutput;
  settings.enableTerminalOutput = false;

  gpsSensor_ublox.setAutoNAVPOSECEFcallbackPtr(&callbackNAVPOSECEF, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVPOSECEF(settings.sensor_uBlox.logUBXNAVPOSECEF > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVPOSECEFrate(settings.sensor_uBlox.logUBXNAVPOSECEF, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVSTATUScallbackPtr(&callbackNAVSTATUS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVSTATUS(settings.sensor_uBlox.logUBXNAVSTATUS > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVSTATUSrate(settings.sensor_uBlox.logUBXNAVSTATUS, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoDOPcallbackPtr(&callbackNAVDOP, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVDOP(settings.sensor_uBlox.logUBXNAVDOP > 0 ? true : false);
  gpsSensor_ublox.setAutoDOPrate(settings.sensor_uBlox.logUBXNAVDOP, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVATTcallbackPtr(&callbackNAVATT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVATT(settings.sensor_uBlox.logUBXNAVATT > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVATTrate(settings.sensor_uBlox.logUBXNAVATT, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoPVTcallbackPtr(&callbackNAVPVT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVPVT(settings.sensor_uBlox.logUBXNAVPVT > 0 ? true : false);
  gpsSensor_ublox.setAutoPVTrate(settings.sensor_uBlox.logUBXNAVPVT, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVODOcallbackPtr(&callbackNAVODO, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVODO(settings.sensor_uBlox.logUBXNAVODO > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVODOrate(settings.sensor_uBlox.logUBXNAVODO, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVVELECEFcallbackPtr(&callbackNAVVELECEF, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVVELECEF(settings.sensor_uBlox.logUBXNAVVELECEF > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVVELECEFrate(settings.sensor_uBlox.logUBXNAVVELECEF, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVVELNEDcallbackPtr(&callbackNAVVELNED, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVVELNED(settings.sensor_uBlox.logUBXNAVVELNED > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVVELNEDrate(settings.sensor_uBlox.logUBXNAVVELNED, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVHPPOSECEFcallbackPtr(&callbackNAVHPPOSECEF, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVHPPOSECEF(settings.sensor_uBlox.logUBXNAVHPPOSECEF > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVHPPOSECEFrate(settings.sensor_uBlox.logUBXNAVHPPOSECEF, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoHPPOSLLHcallbackPtr(&callbackNAVHPPOSLLH, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVHPPOSLLH(settings.sensor_uBlox.logUBXNAVHPPOSLLH > 0 ? true : false);
  gpsSensor_ublox.setAutoHPPOSLLHrate(settings.sensor_uBlox.logUBXNAVHPPOSLLH, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoNAVCLOCKcallbackPtr(&callbackNAVCLOCK, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVCLOCK(settings.sensor_uBlox.logUBXNAVCLOCK > 0 ? true : false);
  gpsSensor_ublox.setAutoNAVCLOCKrate(settings.sensor_uBlox.logUBXNAVCLOCK, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoRELPOSNEDcallbackPtr(&callbackNAVRELPOSNED, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logNAVRELPOSNED(settings.sensor_uBlox.logUBXNAVRELPOSNED > 0 ? true : false);
  gpsSensor_ublox.setAutoRELPOSNEDrate(settings.sensor_uBlox.logUBXNAVRELPOSNED, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoRXMSFRBXcallbackPtr(&callbackRXMSFRBX, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logRXMSFRBX(settings.sensor_uBlox.logUBXRXMSFRBX > 0 ? true : false);
  gpsSensor_ublox.setAutoRXMSFRBXrate(settings.sensor_uBlox.logUBXRXMSFRBX, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoRXMRAWXcallbackPtr(&callbackRXMRAWX, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logRXMRAWX(settings.sensor_uBlox.logUBXRXMRAWX > 0 ? true : false);
  gpsSensor_ublox.setAutoRXMRAWXrate(settings.sensor_uBlox.logUBXRXMRAWX, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoRXMMEASXcallbackPtr(&callbackRXMMEASX, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logRXMMEASX(settings.sensor_uBlox.logUBXRXMMEASX > 0 ? true : false);
  gpsSensor_ublox.setAutoRXMMEASXrate(settings.sensor_uBlox.logUBXRXMMEASX, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoTIMTM2callbackPtr(&callbackTIMTM2, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logTIMTM2(settings.sensor_uBlox.logUBXTIMTM2 > 0 ? true : false);
  gpsSensor_ublox.setAutoTIMTM2rate(settings.sensor_uBlox.logUBXTIMTM2, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoESFALGcallbackPtr(&callbackESFALG, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFALG(settings.sensor_uBlox.logUBXESFALG > 0 ? true : false);
  gpsSensor_ublox.setAutoESFALGrate(settings.sensor_uBlox.logUBXESFALG, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoESFINScallbackPtr(&callbackESFINS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFINS(settings.sensor_uBlox.logUBXESFINS > 0 ? true : false);
  gpsSensor_ublox.setAutoESFINSrate(settings.sensor_uBlox.logUBXESFINS, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoESFMEAScallbackPtr(&callbackESFMEAS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFMEAS(settings.sensor_uBlox.logUBXESFMEAS > 0 ? true : false);
  gpsSensor_ublox.setAutoESFMEASrate(settings.sensor_uBlox.logUBXESFMEAS, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoESFRAWcallbackPtr(&callbackESFRAW, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFRAW(settings.sensor_uBlox.logUBXESFRAW > 0 ? true : false);
  gpsSensor_ublox.setAutoESFRAWrate(settings.sensor_uBlox.logUBXESFRAW, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoESFSTATUScallbackPtr(&callbackESFSTATUS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logESFSTATUS(settings.sensor_uBlox.logUBXESFSTATUS > 0 ? true : false);
  gpsSensor_ublox.setAutoESFSTATUSrate(settings.sensor_uBlox.logUBXESFSTATUS, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoHNRPVTcallbackPtr(&callbackHNRPVT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logHNRPVT(settings.sensor_uBlox.logUBXHNRPVT > 0 ? true : false);
  gpsSensor_ublox.setAutoHNRPVTrate(settings.sensor_uBlox.logUBXHNRPVT, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoHNRATTcallbackPtr(&callbackHNRATT, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logHNRATT(settings.sensor_uBlox.logUBXHNRATT > 0 ? true : false);
  gpsSensor_ublox.setAutoHNRATTrate(settings.sensor_uBlox.logUBXHNRATT, false, VAL_LAYER_RAM_BBR, maxWait);

  gpsSensor_ublox.setAutoHNRINScallbackPtr(&callbackHNRINS, VAL_LAYER_RAM_BBR, maxWait);
  gpsSensor_ublox.logHNRINS(settings.sensor_uBlox.logUBXHNRINS > 0 ? true : false);
  gpsSensor_ublox.setAutoHNRINSrate(settings.sensor_uBlox.logUBXHNRINS, false, VAL_LAYER_RAM_BBR, maxWait);

  uint32_t nmeaMessages = 0;

  gpsSensor_ublox.newCfgValset(VAL_LAYER_RAM_BBR); // Use cfgValset to disable individual NMEA messages

  if (settings.sensor_uBlox.logNMEADTM > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_DTM;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_DTM_I2C, settings.sensor_uBlox.logNMEADTM);

  if (settings.sensor_uBlox.logNMEAGBS > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GBS;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GBS_I2C, settings.sensor_uBlox.logNMEAGBS);

  if (settings.sensor_uBlox.logNMEAGGA > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GGA;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GGA_I2C, settings.sensor_uBlox.logNMEAGGA);

  if (settings.sensor_uBlox.logNMEAGLL > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GLL;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GLL_I2C, settings.sensor_uBlox.logNMEAGLL);

  if (settings.sensor_uBlox.logNMEAGNS > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GNS;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GNS_I2C, settings.sensor_uBlox.logNMEAGNS);

  if (settings.sensor_uBlox.logNMEAGRS > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GRS;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GRS_I2C, settings.sensor_uBlox.logNMEAGRS);

  if (settings.sensor_uBlox.logNMEAGSA > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GSA;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GSA_I2C, settings.sensor_uBlox.logNMEAGSA);

  if (settings.sensor_uBlox.logNMEAGST > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GST;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GST_I2C, settings.sensor_uBlox.logNMEAGST);

  if (settings.sensor_uBlox.logNMEAGSV > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_GSV;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GSV_I2C, settings.sensor_uBlox.logNMEAGSV);

  if (settings.sensor_uBlox.logNMEARLM > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_RLM;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_RLM_I2C, settings.sensor_uBlox.logNMEARLM);

  if (settings.sensor_uBlox.logNMEARMC > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_RMC;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_RMC_I2C, settings.sensor_uBlox.logNMEARMC);

  if (settings.sensor_uBlox.logNMEATXT > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_TXT;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_INFMSG_NMEA_I2C, settings.sensor_uBlox.logNMEATXT);

  if (settings.sensor_uBlox.logNMEAVLW > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_VLW;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_VLW_I2C, settings.sensor_uBlox.logNMEAVLW);

  if (settings.sensor_uBlox.logNMEAVTG > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_VTG;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_VTG_I2C, settings.sensor_uBlox.logNMEAVTG);

  if (settings.sensor_uBlox.logNMEAZDA > 0) nmeaMessages |= SFE_UBLOX_FILTER_NMEA_ZDA;
  gpsSensor_ublox.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_ZDA_I2C, settings.sensor_uBlox.logNMEAZDA);

  gpsSensor_ublox.sendCfgValset();

  gpsSensor_ublox.setNMEALoggingMask(nmeaMessages);

  settings.enableTerminalOutput = prevTerminalOutput;
}

boolean enableConstellations(uint16_t maxWait)
{
  boolean success = true;
  
  success &= gpsSensor_ublox.enableGNSS(settings.sensor_uBlox.enableGPS, SFE_UBLOX_GNSS_ID_GPS, VAL_LAYER_RAM_BBR, maxWait);
  success &= gpsSensor_ublox.enableGNSS(settings.sensor_uBlox.enableGLO, SFE_UBLOX_GNSS_ID_GLONASS, VAL_LAYER_RAM_BBR, maxWait);
  success &= gpsSensor_ublox.enableGNSS(settings.sensor_uBlox.enableGAL, SFE_UBLOX_GNSS_ID_GALILEO, VAL_LAYER_RAM_BBR, maxWait);
  success &= gpsSensor_ublox.enableGNSS(settings.sensor_uBlox.enableBDS, SFE_UBLOX_GNSS_ID_BEIDOU, VAL_LAYER_RAM_BBR, maxWait);
  success &= gpsSensor_ublox.enableGNSS(settings.sensor_uBlox.enableQZSS, SFE_UBLOX_GNSS_ID_QZSS, VAL_LAYER_RAM_BBR, maxWait);
  
  if (settings.printMinorDebugMessages)
  {
    if (success)
    {
      Serial.println(F("enableConstellations: successful"));
    }
    else
    {
      Serial.println(F("enableConstellations: failed!"));
    }
  }
  return (success);
}

boolean powerManagementTask(uint32_t duration, uint16_t maxWait) //Put the module to sleep for duration ms
{
  // UBX_RXM_PMREQ does not ACK so we expect the return value to be false
  return (gpsSensor_ublox.powerOff(duration, maxWait));
}

//If certain devices are attached, we need to reduce the I2C max speed
void determineMaxI2CSpeed()
{
  uint32_t maxSpeed = 400000; //Assume 400kHz - but beware! 400kHz with no pullups can cause issues.

  //If user wants to limit the I2C bus speed, do it here
  if (maxSpeed > settings.qwiicBusMaxSpeed)
    maxSpeed = settings.qwiicBusMaxSpeed;

  qwiic.setClock(maxSpeed);
}

//Read the VIN voltage
float readVIN()
{
  // Only supported on >= V10 hardware
#if(HARDWARE_VERSION_MAJOR == 0)
  return(0.0); // Return 0.0V on old hardware
#else
  int div3 = analogRead(PIN_VIN_MONITOR); //Read VIN across a 1/3 resistor divider
  float vin = (float)div3 * 3.0 * 2.0 / 16384.0; //Convert 1/3 VIN to VIN (14-bit resolution)
  vin = vin * settings.vinCorrectionFactor; //Correct for divider impedance (determined experimentally)
  return (vin);
#endif
}
