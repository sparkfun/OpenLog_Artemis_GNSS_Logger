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

  determineMaxI2CSpeed(); //Try for 400kHz but reduce if the user has selected a slower speed
  if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && ((!qwiicOnline.uBlox) || (gnssSettingsChanged == true))) // Only do this if the sensor is not online
  {
    gnssSettingsChanged = false;
    if (gpsSensor_ublox.begin(qwiic, settings.sensor_uBlox.ubloxI2Caddress) == true) //Wire port, Address. Default is 0x42.
    {
      // Try up to three times to get the module info
      if (getModuleInfo(1100) == false) // Try to get the module info
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.println(F("beginSensors: first getModuleInfo call failed. Trying again...")); 
        }       
        if (getModuleInfo(1100) == false) // Try to get the module info
        {
          if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: second getModuleInfo call failed. Trying again...")); 
          }       
          if (getModuleInfo(1100) == false) // Try to get the module info
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
        Serial.print(minfo.protVerMajor);
        Serial.print(F("."));
        Serial.print(minfo.protVerMinor);
        Serial.print(F(" SPG=")); //Standard Precision
        Serial.print(minfo.SPG);
        Serial.print(F(" HPG=")); //High Precision (ZED-F9P)
        Serial.print(minfo.HPG); // We can only enable RAWX on HPG and TIM modules
        Serial.print(F(" ADR=")); //Dead Reckoning (ZED-F9K)
        Serial.print(minfo.ADR);
        Serial.print(F(" UDR=")); //Untethered Dead Reckoning (NEO-M8U)
        Serial.print(minfo.UDR);
        Serial.print(F(" TIM=")); //Time sync (ZED-F9T)
        Serial.print(minfo.TIM); // We can only enable RAWX on HPG and TIM modules
        Serial.print(F(" FTS=")); //Frequency and time sync
        Serial.print(minfo.FTS); // Let's guess that we can enable RAWX on FTS modules
        Serial.print(F(" LAP=")); //Lane accurate
        Serial.print(minfo.LAP);
        Serial.print(F(" HDG=")); //Heading (ZED-F9H)
        Serial.print(minfo.HDG);
        Serial.print(F(" HPS=")); //High Precision Sensor Fusion (ZED-F9R)
        Serial.print(minfo.HPS);
        Serial.print(F(" MOD=")); //Module type
        Serial.println(minfo.mod);
      }

      //Check the PROTVER is >= 27
      if (minfo.protVerMajor < 27)
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.print(F("beginSensors: module does not support the configuration interface. Aborting!"));
        }
        qwiicOnline.uBlox = false;
        return(false);
      }
      
      //Set the I2C port to output UBX only (turn off NMEA noise)
      gpsSensor_ublox.newCfgValset8(UBLOX_CFG_I2COUTPROT_UBX, VAL_LAYER_RAM | VAL_LAYER_BBR); // CFG-I2COUTPROT-UBX : Enable UBX output on the I2C port (in RAM and BBR)
      gpsSensor_ublox.addCfgValset8(UBLOX_CFG_I2COUTPROT_NMEA, 0); // CFG-I2COUTPROT-NMEA : Disable NMEA output on the I2C port
      if (minfo.HPG == true)
      {
        gpsSensor_ublox.addCfgValset8(UBLOX_CFG_I2COUTPROT_RTCM3X, 0); // CFG-I2COUTPROT-RTCM3X : Disable RTCM3 output on the I2C port (Precision modules only)
      }
      uint8_t success = gpsSensor_ublox.sendCfgValset8(UBLOX_CFG_INFMSG_UBX_I2C, 0, 2100); // CFG-INFMSG-UBX_I2C : Disable UBX INFo messages on the I2C port (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset failed when setting the I2C port to output UBX only")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset was successful when setting the I2C port to output UBX only")); 
          }       
      }

      //Disable all messages in RAM (maxWait 2100)
      disableMessages(2100);
      
      //Disable USB port if required
      if (settings.sensor_uBlox.enableUSB == false)
      {
        success = gpsSensor_ublox.setVal8(UBLOX_CFG_USB_ENABLED, 0, VAL_LAYER_RAM, 1100); // CFG-USB-ENABLED (in RAM only)
        if (success == 0)
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset failed when disabling the USB port")); 
            }       
        }
        else
        {
          if (settings.printMinorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset was successful when disabling the USB port")); 
            }       
        }
      }

      //Disable UART1 port if required
      if (settings.sensor_uBlox.enableUART1 == false)
      {
        success = gpsSensor_ublox.setVal8(UBLOX_CFG_UART1_ENABLED, 0, VAL_LAYER_RAM, 1100); // CFG-UART1-ENABLED (in RAM only)
        if (success == 0)
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset failed when disabling UART1")); 
            }       
        }
        else
        {
          if (settings.printMinorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset was successful when disabling UART1")); 
            }       
        }
      }

      //Disable UART2 port if required
      if (settings.sensor_uBlox.enableUART2 == false)
      {
        success = gpsSensor_ublox.setVal8(UBLOX_CFG_UART2_ENABLED, 0, VAL_LAYER_RAM, 1100); // CFG-UART2-ENABLED (in RAM only)
        if (success == 0)
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset failed when disabling UART2")); 
            }       
        }
        else
        {
          if (settings.printMinorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset was successful when disabling UART2")); 
            }       
        }
      }

      //Disable SPI port if required
      if (settings.sensor_uBlox.enableSPI == false)
      {
        success = gpsSensor_ublox.setVal8(UBLOX_CFG_SPI_ENABLED, 0, VAL_LAYER_RAM, 1100); // CFG-SPI-ENABLED (in RAM only)
        if (success == 0)
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset failed when disabling the SPI port")); 
            }       
        }
        else
        {
          if (settings.printMinorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset was successful when disabling the SPI port")); 
            }       
        }
      }

      //Update settings.sensor_uBlox.minMeasIntervalGPS and settings.sensor_uBlox.minMeasIntervalAll according to module type
      if (strcmp(minfo.mod,"ZED-F9P") == 0) //Is this a ZED-F9P?
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 50; //ZED-F9P can do 20Hz RTK (*** Change this to 40 if you want to push RAWX logging to 25Hz for non-RTK applications ***)
        settings.sensor_uBlox.minMeasIntervalAll = 125; //ZED-F9P can do 8Hz RTK
      }
      else if (strcmp(minfo.mod,"ZED-F9K") == 0) //Is this a ZED-F9K?
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 33; //ZED-F9K can do 30Hz
        settings.sensor_uBlox.minMeasIntervalAll = 100; //ZED-F9K can do 10Hz (Guess!)
      }
      else if (strcmp(minfo.mod,"ZED-F9R") == 0) //Is this a ZED-F9R?
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 33; //ZED-F9R can do 30Hz
        settings.sensor_uBlox.minMeasIntervalAll = 100; //ZED-F9R can do 10Hz (Guess!)
      }
      else if (strcmp(minfo.mod,"ZED-F9H") == 0) //Is this a ZED-F9H?
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 33; //ZED-F9H can do 30Hz
        settings.sensor_uBlox.minMeasIntervalAll = 100; //ZED-F9H can do 10Hz (Guess!)
      }
      else if (strcmp(minfo.mod,"ZED-F9T") == 0) //Is this a ZED-F9T?
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 50; //ZED-F9T can do 20Hz
        settings.sensor_uBlox.minMeasIntervalAll = 125; //ZED-F9T can do 8Hz
      }
      else if (strcmp(minfo.mod,"NEO-M9N") == 0) //Is this a NEO-M9N?
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 40; //NEO-M9N can do 25Hz
        settings.sensor_uBlox.minMeasIntervalAll = 40; //NEO-M9N can do 25Hz
      }
      else
      {
        settings.sensor_uBlox.minMeasIntervalGPS = 50; //Default to 20Hz
        settings.sensor_uBlox.minMeasIntervalAll = 125; //Default to 8Hz
      }

      //Calculate measurement rate
      uint16_t measRate;
      if (settings.usBetweenReadings < (((uint32_t)settings.sensor_uBlox.minMeasIntervalGPS) * 1000)) // Check if usBetweenReadings is too low
      {
        measRate = settings.sensor_uBlox.minMeasIntervalGPS;
      }
      else if (settings.usBetweenReadings > (0xFFFF * 1000)) // Check if usBetweenReadings is too high
      {
        measRate = 0xFFFF;
      }
      else
      {
        measRate = (uint16_t)(settings.usBetweenReadings / 1000); // Convert usBetweenReadings to ms
      }
      
      //If measurement interval is less than minMeasIntervalAll then warn the user that they may need to disable all constellations except GPS
      //If measurement interval is less than minMeasIntervalRAWXAll and RAWX is enabled then also warn the user
      if ((measRate < settings.sensor_uBlox.minMeasIntervalAll) || ((measRate < settings.sensor_uBlox.minMeasIntervalRAWXAll) && (settings.sensor_uBlox.logUBXRXMRAWX == true)))
      {
        // Check if any constellations other than GPS are enabled
        if ((settings.sensor_uBlox.enableGLO) || (settings.sensor_uBlox.enableGAL) || (settings.sensor_uBlox.enableBDS) || (settings.sensor_uBlox.enableQZSS))
        {
          Serial.println(F("*** !!! WARNING !!! ***"));
          Serial.println(F("*** You may need to disable GLONASS, Galileo, BeiDou and QZSS to achieve the selected logging rate ***"));
          Serial.println(F("*** (Use the \"Configure GNSS Device\" menu to disable them) ***"));
        }        
      }

      // Enable/Disable the selected constellations in RAM (MaxWait 2100)
      // Let's do this before we set the message rate and enable messages
      enableConstellations(2100);
      
      //Set output rate
      gpsSensor_ublox.newCfgValset16(UBLOX_CFG_RATE_MEAS, measRate, VAL_LAYER_RAM); // CFG-RATE-MEAS : Configure measurement period (in RAM only)
      success = gpsSensor_ublox.sendCfgValset16(UBLOX_CFG_RATE_NAV, 1, 2100); // CFG-RATE-NAV : 1 measurement per navigation solution (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset failed when setting message interval")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset was successful when setting message interval")); 
          }       
      }

      //Enable the selected messages in RAM (MaxWait 2100)
      enableMessages(2100);

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
      Serial.printf("Device found at address 0x%02X\n", address);
    }
    if (gpsSensor_ublox.begin(qwiic, address) == true) //Wire port, address
    {
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
      //Disable all messages in RAM (maxWait 0)
      disableMessages(0);
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait > 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData(); //storeData is the workhorse. It reads I2C data and writes it to SD.
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
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

      //(Re)Enable the selected messages in RAM (MaxWait 2100)
      enableMessages(2100);
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
      //Disable all messages in RAM (maxWait 0)
      disableMessages(0);
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait > 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData(); //storeData is the workhorse. It reads I2C data and writes it to SD.
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
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
      //Disable all messages in RAM (maxWait 0)
      disableMessages(0);
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait >> 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData(); //storeData is the workhorse. It reads I2C data and writes it to SD.
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
      gnssDataFile.sync();

      updateDataFileAccess(&gnssDataFile); //Update the file access time stamp

      gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098

      //Reset the GNSS
      //Note: this method is DEPRECATED. TO DO: replace this with UBX-CFG-VALDEL ?
      gpsSensor_ublox.factoryDefault(2100);
      gpsSensor_ublox.factoryReset();

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

uint8_t disableMessages(uint16_t maxWait)
{
  //Disable all logable messages
  uint8_t success1 = true;
  success1 &= gpsSensor_ublox.newCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_CLOCK_I2C, 0, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM only)
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_ATT_I2C, 0); // CFG-MSGOUT-UBX_NAV_ATT_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_DOP_I2C, 0); // CFG-MSGOUT-UBX_NAV_DOP_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_ODO_I2C, 0); // CFG-MSGOUT-UBX_NAV_ODO_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_POSECEF_I2C, 0); // CFG-MSGOUT-UBX_NAV_POSECEF_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_POSLLH_I2C, 0); // CFG-MSGOUT-UBX_NAV_POSLLH_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_PVT_I2C, 0); // CFG-MSGOUT-UBX_NAV_PVT_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_STATUS_I2C, 0); // CFG-MSGOUT-UBX_NAV_STATUS_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_TIMEUTC_I2C, 0); // CFG-MSGOUT-UBX_NAV_TIMEUTC_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_VELECEF_I2C, 0); // CFG-MSGOUT-UBX_NAV_VELECEF_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_VELNED_I2C, 0); // CFG-MSGOUT-UBX_NAV_VELNED_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_RXM_SFRBX_I2C, 0); // CFG-MSGOUT-UBX_RXM_SFRBX_I2C
  success1 &= gpsSensor_ublox.sendCfgValset8(UBLOX_CFG_MSGOUT_UBX_TIM_TM2_I2C, 0, maxWait); // CFG-MSGOUT-UBX_TIM_TM2_I2C

  uint8_t success2 = true;
  success2 &= gpsSensor_ublox.newCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_HPPOSECEF_I2C, 0, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_HPPOSECEF_I2C
  success2 &= gpsSensor_ublox.sendCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_HPPOSLLH_I2C, 0, maxWait); // CFG-MSGOUT-UBX_NAV_HPPOSLLH_I2C

  uint8_t success3 = gpsSensor_ublox.setVal8(UBLOX_CFG_MSGOUT_UBX_NAV_RELPOSNED_I2C, 0, VAL_LAYER_RAM, maxWait); // CFG-MSGOUT-UBX_NAV_RELPOSNED_I2C

  uint8_t success4 = gpsSensor_ublox.setVal8(UBLOX_CFG_MSGOUT_UBX_RXM_RAWX_I2C, 0, VAL_LAYER_RAM, maxWait); // CFG-MSGOUT-UBX_RXM_RAWX_I2C

  uint8_t success5 = gpsSensor_ublox.setVal8(UBLOX_CFG_MSGOUT_UBX_NAV_ATT_I2C, 0, VAL_LAYER_RAM, maxWait); // CFG-MSGOUT-UBX_NAV_ATT_I2C

  uint8_t success = success1 | success2 | success3 | success4 | success5;
  if (success > 0)
  {
    if (settings.printMinorDebugMessages == true)
      {
        Serial.println(F("disableMessages: sendCfgValset / setVal8 was at least partially successful when disabling messages")); 
      }       
  }
  else if (maxWait > 0) //If maxWait was zero then we expect success to be false
  {
    if (settings.printMajorDebugMessages == true)
      {
        Serial.println(F("disableMessages: sendCfgValset / setVal8 failed when disabling messages")); 
      }       
  }
  return(success);
}

uint8_t enableMessages(uint16_t maxWait)
{
  //(Re)Enable the selected messages
  uint8_t success1 = true;
  success1 &= gpsSensor_ublox.newCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_CLOCK_I2C, settings.sensor_uBlox.logUBXNAVCLOCK, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM only)
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_DOP_I2C, settings.sensor_uBlox.logUBXNAVDOP); // CFG-MSGOUT-UBX_NAV_DOP_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_ODO_I2C, settings.sensor_uBlox.logUBXNAVODO); // CFG-MSGOUT-UBX_NAV_ODO_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_POSECEF_I2C, settings.sensor_uBlox.logUBXNAVPOSECEF); // CFG-MSGOUT-UBX_NAV_POSECEF_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_POSLLH_I2C, settings.sensor_uBlox.logUBXNAVPOSLLH); // CFG-MSGOUT-UBX_NAV_POSLLH_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_PVT_I2C, settings.sensor_uBlox.logUBXNAVPVT); // CFG-MSGOUT-UBX_NAV_PVT_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_STATUS_I2C, settings.sensor_uBlox.logUBXNAVSTATUS); // CFG-MSGOUT-UBX_NAV_STATUS_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_TIMEUTC_I2C, settings.sensor_uBlox.logUBXNAVTIMEUTC); // CFG-MSGOUT-UBX_NAV_TIMEUTC_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_VELECEF_I2C, settings.sensor_uBlox.logUBXNAVVELECEF); // CFG-MSGOUT-UBX_NAV_VELECEF_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_VELNED_I2C, settings.sensor_uBlox.logUBXNAVVELNED); // CFG-MSGOUT-UBX_NAV_VELNED_I2C
  success1 &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_MSGOUT_UBX_RXM_SFRBX_I2C, settings.sensor_uBlox.logUBXRXMSFRBX); // CFG-MSGOUT-UBX_RXM_SFRBX_I2C
  success1 &= gpsSensor_ublox.sendCfgValset8(UBLOX_CFG_MSGOUT_UBX_TIM_TM2_I2C, settings.sensor_uBlox.logUBXTIMTM2, maxWait); // CFG-MSGOUT-UBX_TIM_TM2_I2C

  uint8_t success2 = true;
  if (minfo.SPG == false)
  {
    success2 &= gpsSensor_ublox.newCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_HPPOSECEF_I2C, settings.sensor_uBlox.logUBXNAVHPPOSECEF, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_HPPOSECEF_I2C
    success2 &= gpsSensor_ublox.sendCfgValset8(UBLOX_CFG_MSGOUT_UBX_NAV_HPPOSLLH_I2C, settings.sensor_uBlox.logUBXNAVHPPOSLLH, maxWait); // CFG-MSGOUT-UBX_NAV_HPPOSLLH_I2C
  }

  uint8_t success3 = true;
  if ((minfo.HPG == true) || (minfo.HDG == true) || (minfo.ADR == true) || (minfo.LAP == true))
  {
    success3 = gpsSensor_ublox.setVal8(UBLOX_CFG_MSGOUT_UBX_NAV_RELPOSNED_I2C, settings.sensor_uBlox.logUBXNAVRELPOSNED, VAL_LAYER_RAM, maxWait); // CFG-MSGOUT-UBX_NAV_RELPOSNED_I2C
  }

  uint8_t success4 = true;  
  if ((minfo.HPG == true) || (minfo.TIM == true) || (minfo.FTS == true)) //TO DO: I'm guessing that FTS supports RAWX!
  {
    success4 = gpsSensor_ublox.setVal8(UBLOX_CFG_MSGOUT_UBX_RXM_RAWX_I2C, settings.sensor_uBlox.logUBXRXMRAWX, VAL_LAYER_RAM, maxWait); // CFG-MSGOUT-UBX_RXM_RAWX_I2C
  }
  
  uint8_t success5 = true;  
  if ((minfo.HPS == true))
  {
    success5 = gpsSensor_ublox.setVal8(UBLOX_CFG_MSGOUT_UBX_NAV_ATT_I2C, settings.sensor_uBlox.logUBXNAVATT, VAL_LAYER_RAM, maxWait); // CFG-MSGOUT-UBX_NAV_ATT_I2C
  }
  
  uint8_t success = success1 & success2 & success3 & success4 & success5;
  if (success > 0)
  {
  if (settings.printMinorDebugMessages == true)
    {
      Serial.println(F("enableMessages: sendCfgValset / setVal8 was successful when enabling messages. ")); 
    }       
  }
  else if (maxWait > 0) //If maxWait was zero then we expect success to be false
  {
  if (settings.printMajorDebugMessages == true)
    {
      Serial.println(F("enableMessages: at least one sendCfgValset / setVal8 failed when enabling messages")); 
    }       
  }
  return(success);
}

uint8_t enableConstellations(uint16_t maxWait)
{
  //Enable the selected constellations
  uint8_t success = true;
  success &= gpsSensor_ublox.newCfgValset8(UBLOX_CFG_SIGNAL_GPS_ENA, settings.sensor_uBlox.enableGPS, VAL_LAYER_RAM);  //CFG-SIGNAL-GPS_ENA   : Enable GPS (in RAM only)
  success &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_SIGNAL_GLO_ENA, settings.sensor_uBlox.enableGLO);                 //CFG-SIGNAL-GLO_ENA   : Enable GLONASS
  success &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_SIGNAL_GAL_ENA, settings.sensor_uBlox.enableGAL);                 //CFG-SIGNAL-GAL_ENA   : Enable Galileo
  success &= gpsSensor_ublox.addCfgValset8(UBLOX_CFG_SIGNAL_BDS_ENA, settings.sensor_uBlox.enableBDS);                 //CFG-SIGNAL-BDS_ENA   : Enable BeiDou
  success &= gpsSensor_ublox.sendCfgValset8(UBLOX_CFG_SIGNAL_QZSS_ENA, settings.sensor_uBlox.enableQZSS, maxWait);     //CFG-SIGNAL-QZSS_ENA  : Enable QZSS (maxWait 2100 ms)
  if (success > 0)
  {
    if (settings.printMinorDebugMessages)
    {
      Serial.println(F("enableConstellations: sendCfgValset was successful when enabling constellations"));
    }
  }
  else if (maxWait > 0) // If maxWait was zero then we expect success to be false
  {
    if (settings.printMajorDebugMessages)
    {
      Serial.println(F("enableConstellations: sendCfgValset failed when enabling constellations"));
    }
  }
  return (success);
}

uint8_t powerManagementTask(uint32_t duration, uint16_t maxWait) //Put the module to sleep for duration ms
{
  customCfg.cls = UBX_CLASS_RXM; // This is the message Class
  customCfg.id = UBX_RXM_PMREQ;  // This is the message ID
  customCfg.len = 16;            // Set the len (length)
  customCfg.startingSpot = 0;    // Set the startingSpot to zero

  //Define the payload
  customPayload[0] = 0x00; //Message version (0x00 for this version)
  customPayload[1] = 0x00; //Reserved
  customPayload[2] = 0x00; //Reserved
  customPayload[3] = 0x00; //Reserved
  customPayload[4] = duration & 0xFF; //Duration of the power management task
  customPayload[5] = (duration >> 8) & 0xFF;
  customPayload[6] = (duration >> 16) & 0xFF;
  customPayload[7] = (duration >> 24) & 0xFF;
  customPayload[8] = 0x02; //Flags : set the backup bit (leave the force bit clear - module will stay on if USB is connected)
  customPayload[9] = 0x00; //Flags
  customPayload[10] = 0x00; //Flags
  customPayload[11] = 0x00; //Flags
  customPayload[12] = 0x00; //Disable the wakeup sources
  customPayload[13] = 0x00; //wakeup sources
  customPayload[14] = 0x00; //wakeup sources
  customPayload[15] = 0x00; //wakeup sources

  // Now let's send the command.
  // UBX_RXM_PMREQ does not ACK so we expect the return value to be false
  return ((gpsSensor_ublox.sendCommand(&customCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT));
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
