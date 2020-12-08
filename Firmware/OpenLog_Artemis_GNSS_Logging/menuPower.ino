void menuPower()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Power Options");

#if(HARDWARE_VERSION_MAJOR >= 1) || (HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 6)
    Serial.print("1) Turn off Qwiic bus power when sleeping: ");
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println("Yes");
    else Serial.println("No");
#endif

#if(HARDWARE_VERSION_MAJOR >= 1)
    Serial.print("2) Power LED During Sleep: ");
    if (settings.enablePwrLedDuringSleep == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print(F("3) Low Battery Voltage Detection: "));
    if (settings.enableLowBatteryDetection == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("4) Low Battery Threshold (V): "));
    Serial.printf("%.2f\r\n", settings.lowBatteryThreshold);

    Serial.print(F("5) VIN measurement correction factor: "));
    Serial.printf("%.3f\r\n", settings.vinCorrectionFactor);

    Serial.println(F("6) Print battery voltage"));
#endif

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == 'x')
      break;
#if(HARDWARE_VERSION_MAJOR >= 1) || (HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 6)
    else if (incoming == '1')
    {
      settings.powerDownQwiicBusBetweenReads ^= 1;
    }
#endif
#if(HARDWARE_VERSION_MAJOR >= 1)
    else if (incoming == '2')
    {
      settings.enablePwrLedDuringSleep ^= 1;
    }
    else if (incoming == '3')
    {
      settings.enableLowBatteryDetection ^= 1;
    }
    else if (incoming == '4')
    {
      Serial.println(F("Please enter the new low battery threshold:"));
      float tempBT = (float)getDouble(menuTimeout); //Timeout after x seconds
      if ((tempBT < 3.0) || (tempBT > 6.0))
        Serial.println(F("Error: Threshold out of range"));
      else
        settings.lowBatteryThreshold = tempBT;
    }
    else if (incoming == '5')
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
    else if (incoming == '6')
    {
      for(int i = 0; i < 50; i++)
      {
        Serial.print(F("Battery Voltage (V): "));
        Serial.printf("%.2f\r\n", readVIN()); // Read and print the battery voltage;
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
