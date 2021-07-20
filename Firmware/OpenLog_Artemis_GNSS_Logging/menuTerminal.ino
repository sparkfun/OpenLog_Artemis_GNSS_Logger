void menuLogRate()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Logging"));

    Serial.print(F("1) Log to microSD                                         : "));
    if (settings.logData == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("2) Log to Terminal                                        : "));
    if (settings.enableTerminalOutput == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("3) Set Serial Baud Rate                                   : "));
    Serial.print(settings.serialTerminalBaudRate);
    Serial.println(F(" bps"));

    Serial.print(F("4) Set Standard Message Rate in Hz                        : "));
    if (settings.usBetweenReadings < 1000000ULL) //Take more than one measurement per second
    {
      //Display Integer Hertz
      int logRate = (int)(1000000ULL / settings.usBetweenReadings);
      Serial.printf("%d\r\n", logRate);
    }
    else
    {
      //Display fractional Hertz
      uint32_t logRateSeconds = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
      char tempStr[16];
      olaftoa(1.0 / logRateSeconds, tempStr, 6, sizeof(tempStr) / sizeof(char));
      Serial.printf("%s\r\n", tempStr);
    }

    Serial.print(F("5) Set Standard Message Interval in seconds               : "));
    if (settings.usBetweenReadings > 1000000ULL) //Take more than one measurement per second
    {
      uint32_t interval = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
      Serial.printf("%d\r\n", interval);
    }
    else
    {
      float rate = (float)(settings.usBetweenReadings / 1000000.0);
      char tempStr[16];
      olaftoa(rate, tempStr, 6, sizeof(tempStr) / sizeof(char));
      Serial.printf("%s\r\n", tempStr);
    }
    
    Serial.print(F("6) Set High Navigation Rate in Hz (ADR / UDR Only)        : "));
    //Display Integer Hertz
    Serial.printf("%d\r\n", settings.hnrNavigationRate);

    Serial.print(F("7) Set logging duration in seconds                        : "));
    Serial.printf("%d\r\n", (uint32_t)(settings.usLoggingDuration / 1000000ULL));

    Serial.print(F("8) Set sleep duration in seconds (0 = continuous logging) : "));
    Serial.printf("%d\r\n", (uint32_t)(settings.usSleepDuration / 1000000ULL));

    Serial.print(F("9) Open new log file after sleep                          : "));
    if (settings.openNewLogFile == true) Serial.println(F("Yes"));
    else Serial.println(F("No"));

    Serial.print(F("10) Frequent log file access timestamps: "));
    if (settings.frequentFileAccessTimestamps == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      settings.logData ^= 1;
    else if (incoming == 2)
      settings.enableTerminalOutput ^= 1;
    else if (incoming == 3)
    {
      Serial.print(F("Enter baud rate (1200 to 500000): "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 1200 || newBaud > 500000)
      {
        Serial.println(F("Error: baud rate out of range"));
      }
      else
      {
        settings.serialTerminalBaudRate = newBaud;
        recordSettings();
        Serial.printf("Terminal now set at %dbps. Please reset device and open terminal at new baud rate. Freezing...\r\n", settings.serialTerminalBaudRate);
        while (1);
      }
    }
    else if (incoming == 4)
    {
      Serial.println(F("How many readings per second would you like to log?"));
      int tempRPS = getNumber(menuTimeout); //Timeout after x seconds
      if (tempRPS < 1 || tempRPS > 30)
        Serial.println(F("Error: Readings Per Second out of range"));
      else
        settings.usBetweenReadings = 1000000UL / tempRPS;

      gnssSettingsChanged = true; //Mark gnss settings as changed so it will be started with new settings
      //qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
    }
    else if (incoming == 5)
    {
      Serial.println(F("How many seconds between readings? (1 to 30):"));
      uint64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if (tempSeconds < 1 || tempSeconds > 30ULL)
        Serial.println(F("Error: Read Interval out of range"));
      else
        //settings.recordPerSecond = tempRPS;
        settings.usBetweenReadings = 1000000ULL * tempSeconds;

      gnssSettingsChanged = true; //Mark gnss settings as changed so it will be started with new settings
      //qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
    }
    else if (incoming == 6)
    {
      Serial.println(F("How many HNR readings per second would you like to log?"));
      int tempRPS = getNumber(menuTimeout); //Timeout after x seconds
      if (tempRPS < 1 || tempRPS > 30)
        Serial.println(F("Error: Readings Per Second out of range"));
      else
        settings.hnrNavigationRate = tempRPS;

      gnssSettingsChanged = true; //Mark gnss settings as changed so it will be started with new settings
      //qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
    }
    else if (incoming == 7)
    {
      uint64_t secsBetweenReads = settings.usBetweenReadings / 1000000ULL;
      if (secsBetweenReads < 5) secsBetweenReads = 5; //Let's be sensible about this. The module will take ~2 secs to do a hot start anyway.
      Serial.printf("How many seconds would you like to log for? (%d to 129,600):", secsBetweenReads);
      uint64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if ((tempSeconds < secsBetweenReads) || tempSeconds > 129600ULL)
        Serial.println(F("Error: Logging Duration out of range"));
      else
        settings.usLoggingDuration = 1000000ULL * tempSeconds;
    }
    else if (incoming == 8)
    {
      //The Deep Sleep duration is set with am_hal_stimer_compare_delta_set, the duration of which is uint32_t
      //So the maximum we can sleep for is 2^32 / 32768 = 131072 seconds = 36.4 hours
      //Let's limit this to 36 hours = 129600 seconds
      Serial.println(F("How many seconds would you like to sleep for after logging? (0  or  10 to 129,600):"));
      uint64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if (((tempSeconds > 0) && (tempSeconds < 10)) || tempSeconds > 129600ULL)
        Serial.println(F("Error: Sleep Duration out of range"));
      else
        settings.usSleepDuration = 1000000ULL * tempSeconds;
    }
    else if (incoming == 9)
      settings.openNewLogFile ^= 1;
    else if (incoming == 10)
      settings.frequentFileAccessTimestamps ^= 1;
    else if (incoming == STATUS_PRESSED_X)
      return;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
