
//Display the options
//If user doesn't respond within a few seconds, return to main loop
void menuMain()
{
  //Disable debug messages when menu is open
  bool prevPrintMajorDebugMessages = settings.printMajorDebugMessages;
  bool prevPrintMinorDebugMessages = settings.printMinorDebugMessages;
  settings.printMajorDebugMessages = false;
  settings.printMinorDebugMessages = false;

  //Disable terminal output when menu is open
  bool prevTerminalOutput = settings.enableTerminalOutput;
  settings.enableTerminalOutput = false;

  //Disable GNSS debug messages when menu is open
  if (qwiicOnline.uBlox && qwiicAvailable.uBlox)
  {
    gpsSensor_ublox.disableDebugging();
  }
  
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Main Menu"));

    Serial.println(F("1) Configure Logging"));

    Serial.println(F("2) Configure GNSS Device"));

    Serial.println(F("3) Configure Qwiic Bus"));

    Serial.println(F("4) Configure Power Options"));

    if (settings.logData && online.microSD && online.dataLogging)
    {
      Serial.println(F("f) Open New Log File"));
    }

    if (qwiicAvailable.uBlox && qwiicOnline.uBlox)
    {
      Serial.println(F("g) Reset GNSS"));
    }

    Serial.println(F("r) Reset all OLA settings to default"));

    Serial.println("q) Quit: Close log file and power down");

    Serial.println(F("d) Debug Menu"));

    Serial.println(F("x) Return to logging"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      menuLogRate(&prevTerminalOutput);
    else if (incoming == '2')
      menuConfigure_uBlox();
    else if (incoming == '3')
      menuConfigure_QwiicBus();
    else if (incoming == '4')
      menuPower();
    else if (incoming == 'f')
      openNewLogFile();
    else if (incoming == 'g')
    {
      Serial.println(F("\r\nResetting GNSS to factory defaults. Continue? Press 'y':"));
      byte gContinue = getByteChoice(menuTimeout);
      if (gContinue == 'y')
      {
        resetGNSS();
        Serial.print(F("GNSS reset. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        Serial.println(F("bps..."));
        while (1);
      }
      else
        Serial.println(F("GNSS reset aborted"));
    }
    else if (incoming == 'd')
      menuDebug(&prevPrintMajorDebugMessages, &prevPrintMinorDebugMessages);
    else if (incoming == 'r')
    {
      Serial.println(F("\r\nResetting settings to factory defaults. Continue? Press 'y':"));
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        closeLogFile();
        EEPROM.erase();
        if (sd.exists("OLA_GNSS_settings.cfg"))
          sd.remove("OLA_GNSS_settings.cfg");

        Serial.print(F("Settings erased. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        Serial.println(F("bps..."));
        delay(sdPowerDownDelay); // Give the SD card time to shut down
        powerDown();
      }
      else
        Serial.println(F("Reset aborted"));
    }
    else if (incoming == 'q')
    {
      Serial.println("\r\nQuit? Press 'y' to confirm:");
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        closeLogFile();
        Serial.print(F("Log file is closed. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        Serial.println(F("bps..."));
        delay(sdPowerDownDelay); // Give the SD card time to shut down
        powerDown();
      }
      else
        Serial.println(F("Quit aborted"));
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  Serial.println(F("\r\nReturning to logging..."));
  
  //Restore debug messages
  settings.printMajorDebugMessages = prevPrintMajorDebugMessages;
  settings.printMinorDebugMessages = prevPrintMinorDebugMessages;

  settings.enableTerminalOutput = prevTerminalOutput;

  recordSettings(); //Once all menus have exited, record the new settings to EEPROM and config file

  //Once all menus have exited, start any sensors that are available, logging, but not yet online/begun.
  //This will re-enable the GNSS debug messages if desired, set the clock speed and update the pull-ups
  beginSensors();

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars

  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  measurementStartTime = rtcMillis();

}

void menuConfigure_QwiicBus()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Qwiic Bus"));

    Serial.print(F("1) Set Max Qwiic Bus Speed          : "));
    Serial.println(settings.qwiicBusMaxSpeed);
    Serial.print(F("2) Qwiic (I2C) Pull-Up (kOhms)      : "));
    switch (settings.qwiicBusPullUps)
    {
      case 0:
      case 6:
      case 12:
      case 24:
        Serial.println(settings.qwiicBusPullUps);
        break;
      case 1:
        Serial.println(F("1.5"));
        break;
      default:
        Serial.println(F("UNKNOWN"));
        break;
    }
#if(HARDWARE_VERSION_MAJOR >= 1)
    Serial.print(F("3) Turn off bus power when sleeping : "));
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println(F("Yes"));
    else Serial.println(F("No"));
#endif

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      if (settings.qwiicBusMaxSpeed == 100000)
        settings.qwiicBusMaxSpeed = 400000;
      else
        settings.qwiicBusMaxSpeed = 100000;
    }
    else if (incoming == '2')
    {
      switch (settings.qwiicBusPullUps)
      {
        case 0:
          settings.qwiicBusPullUps = 1;
          break;
        case 1:
          settings.qwiicBusPullUps = 6;
          break;
        case 6:
          settings.qwiicBusPullUps = 12;
          break;
        case 12:
          settings.qwiicBusPullUps = 24;
          break;
        case 24:
        default:
          settings.qwiicBusPullUps = 0;
          break;
      }
    }
#if(HARDWARE_VERSION_MAJOR >= 1)
    else if (incoming == '3')
      settings.powerDownQwiicBusBetweenReads ^= 1;
#endif
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

}
