#define packetLength 512

void storeData(void)
{
  if (qwiicOnline.uBlox && qwiicAvailable.uBlox)
  {
    gpsSensor_ublox.checkUblox(); // Check for the arrival of new data and process it.
    gpsSensor_ublox.checkCallbacks(); // Check if any callbacks are waiting to be processed.
  
    while (gpsSensor_ublox.fileBufferAvailable() >= packetLength) // Check to see if we have enough data ready to store
    {
      gpsSensor_ublox.checkUblox(); // Check for the arrival of new data and process it.
  
      uint8_t myBuffer[packetLength]; // Create our own buffer to hold the data while we write it to SD card
  
      gpsSensor_ublox.extractFileBufferData((uint8_t *)&myBuffer, packetLength); // Extract exactly packetLength bytes from the UBX file buffer and put them into myBuffer
  
      digitalWrite(PIN_STAT_LED, HIGH);
      if (settings.logData && settings.sensor_uBlox.log && online.microSD && online.dataLogging)
        gnssDataFile.write(myBuffer, packetLength); // Write exactly packetLength bytes from myBuffer to the ubxDataFile on the SD card
      digitalWrite(PIN_STAT_LED, LOW);
    }
  
    static uint16_t oldMaxBufferBytes = 0;
  
    if (settings.printMajorDebugMessages == true)
    {
      uint16_t maxBufferBytes = gpsSensor_ublox.getMaxFileBufferAvail(); // Get how full the file buffer has been (not how full it is now)
  
      if (maxBufferBytes > oldMaxBufferBytes)
      {
        oldMaxBufferBytes = maxBufferBytes;
        Serial.print(F("storeData: getMaxFileBufferAvail has reached "));
        Serial.println(maxBufferBytes);
      }
    }
  }
}

void storeFinalData(void)
{
  if (qwiicOnline.uBlox && qwiicAvailable.uBlox)
  {
    gpsSensor_ublox.checkUblox(); // Check for the arrival of new data and process it.
    gpsSensor_ublox.checkCallbacks(); // Check if any callbacks are waiting to be processed.
  
    while (gpsSensor_ublox.fileBufferAvailable() > 0) // Check to see if we have any data ready to store
    {
      uint8_t myBuffer[packetLength]; // Create our own buffer to hold the data while we write it to SD card
      uint16_t bytesToWrite;
  
      if (gpsSensor_ublox.fileBufferAvailable() >= packetLength)
        bytesToWrite = packetLength;
      else
        bytesToWrite = gpsSensor_ublox.fileBufferAvailable();
      
      gpsSensor_ublox.extractFileBufferData((uint8_t *)&myBuffer, bytesToWrite); // Extract bytes from the UBX file buffer and put them into myBuffer
  
      digitalWrite(PIN_STAT_LED, HIGH);
      if (settings.logData && settings.sensor_uBlox.log && online.microSD && online.dataLogging)
        gnssDataFile.write(myBuffer, bytesToWrite); // Write exactly bytesToWrite bytes from myBuffer to the ubxDataFile on the SD card
      digitalWrite(PIN_STAT_LED, LOW);
    }
  }
}
