void menuPower()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Power Options");

    Serial.print(F("1) Turn off Qwiic bus power when sleeping : "));
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println("Yes");
    else Serial.println("No");

    Serial.print(F("2) Use pin 32 to Stop Logging             : "));
    if (settings.useGPIO32ForStopLogging == true) Serial.println("Yes");
    else Serial.println("No");

#if(HARDWARE_VERSION_MAJOR >= 1)
    Serial.print(F("3) Power LED During Sleep                 : "));
    if (settings.enablePwrLedDuringSleep == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print(F("4) Low Battery Voltage Detection          : "));
    if (settings.enableLowBatteryDetection == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled")); 

    Serial.print(F("5) Low Battery Threshold (V)              : "));
    char tempStr[16];
    olaftoa(settings.lowBatteryThreshold, tempStr, 2, sizeof(tempStr) / sizeof(char));
    Serial.printf("%s\r\n", tempStr);

    Serial.print(F("6) VIN measurement correction factor      : "));
    olaftoa(settings.vinCorrectionFactor, tempStr, 3, sizeof(tempStr) / sizeof(char));
    Serial.printf("%s\r\n", tempStr);

    Serial.println(F("7) Print battery voltage"));
#endif

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == 'x')
      break;
    else if (incoming == '1')
    {
      settings.powerDownQwiicBusBetweenReads ^= 1;
    }
    else if (incoming == '2')
    {
      if (settings.useGPIO32ForStopLogging == true)
      {
        // Disable stop logging
        settings.useGPIO32ForStopLogging = false;
        detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
        pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
        pin_config(PinName(PIN_STOP_LOGGING), g_AM_HAL_GPIO_INPUT); // Make sure the pin does actually get re-configured
        stopLoggingSeen = false; // Make sure the flag is clear
      }
      else
      {
        // Enable stop logging
        settings.useGPIO32ForStopLogging = true;
        pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
        pin_config(PinName(PIN_STOP_LOGGING), g_AM_HAL_GPIO_INPUT_PULLUP); // Make sure the pin does actually get re-configured
        delay(1); // Let the pin stabilize
        attachInterrupt(PIN_STOP_LOGGING, stopLoggingISR, FALLING); // Enable the interrupt
        pinMode(PIN_STOP_LOGGING, INPUT_PULLUP); //Re-attach the pull-up (bug in v2.1.0 of the core)
        stopLoggingSeen = false; // Make sure the flag is clear
      }
    }
#if(HARDWARE_VERSION_MAJOR >= 1)
    else if (incoming == '3')
    {
      settings.enablePwrLedDuringSleep ^= 1;
    }
    else if (incoming == '4')
    {
      settings.enableLowBatteryDetection ^= 1;
    }
    else if (incoming == '5')
    {
      Serial.println(F("Please enter the new low battery threshold:"));
      float tempBT = (float)getDouble(menuTimeout); //Timeout after x seconds
      if ((tempBT < 3.0) || (tempBT > 6.0))
        Serial.println(F("Error: Threshold out of range"));
      else
        settings.lowBatteryThreshold = tempBT;
    }
    else if (incoming == '6')
    {
      Serial.println(F("Please measure the voltage on the MEAS pin and enter it here:"));
      float tempCF = (float)getDouble(menuTimeout); //Timeout after x seconds
      int div3 = analogRead(PIN_VIN_MONITOR); //Read VIN across a 1/3 resistor divider
      float vin = (float)div3 * 3.0 * 2.0 / 16384.0; //Convert 1/3 VIN to VIN (14-bit resolution)
      tempCF = tempCF / vin; //Calculate the new correction factor
      if ((tempCF < 1.0) || (tempCF > 2.0))
        Serial.println(F("Error: Correction factor out of range"));
      else
        settings.vinCorrectionFactor = tempCF;
    }
    else if (incoming == '7')
    {
      for(int i = 0; i < 50; i++)
      {
        Serial.print(F("Battery Voltage (V): "));
        char tempStr[16];
        olaftoa(readVIN(), tempStr, 2, sizeof(tempStr) / sizeof(char));
        Serial.printf("%s\r\n", tempStr); // Read and print the battery voltage;
        delay(100);
      }
    }
#endif
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
