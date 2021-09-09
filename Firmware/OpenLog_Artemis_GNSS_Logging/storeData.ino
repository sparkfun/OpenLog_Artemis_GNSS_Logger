#define packetLength 512 // SdFat writes data in blocks of 512 bytes

void storeData(void)
{
  if (qwiicOnline.uBlox && qwiicAvailable.uBlox)
  {
    uint16_t fileBytesAvailable = gpsSensor_ublox.fileBufferAvailable();
    
    if (fileBytesAvailable < (FILE_BUFFER_SIZE / 2)) // Check for the arrival of new data and process it but only if the buffer is less than half full
    {
      gpsSensor_ublox.checkUblox(); // Check for the arrival of new data and process it.
      gpsSensor_ublox.checkCallbacks(); // Check if any callbacks are waiting to be processed.
    }

    while (fileBytesAvailable >= packetLength) // Check to see if we have enough data ready to store
    {
      if (fileBytesAvailable < (FILE_BUFFER_SIZE / 2))
        gpsSensor_ublox.checkUblox(); // Check for the arrival of new data and process it while writing but only if the buffer is less than half full
  
      static uint8_t myBuffer[packetLength * 10]; // Create our own buffer to hold the data while we write it to SD card

      uint16_t bytesToWrite = fileBytesAvailable / packetLength; // Calculate bytesToWrite as a multiple of packetLength
      if (bytesToWrite > 10)
        bytesToWrite = 10;
      bytesToWrite *= packetLength;
  
      gpsSensor_ublox.extractFileBufferData((uint8_t *)&myBuffer, bytesToWrite); // Extract exactly bytesToWrite bytes from the UBX file buffer and put them into myBuffer
  
      digitalWrite(PIN_STAT_LED, HIGH);
      if (settings.logData && settings.sensor_uBlox.log && online.microSD && online.dataLogging)
        gnssDataFile.write(myBuffer, bytesToWrite); // Write exactly packetLength bytes from myBuffer to the ubxDataFile on the SD card
      digitalWrite(PIN_STAT_LED, LOW);

      fileBytesAvailable -= bytesToWrite; // Subtract the number of bytes written
      if (fileBytesAvailable < packetLength) // Check if we should update fileBytesAvailable
        fileBytesAvailable = gpsSensor_ublox.fileBufferAvailable();
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

    // Sync logged data and update the access timestamp
    if ((settings.frequentFileAccessTimestamps) && (millis() > (lastDataLogSyncTime + 1000)))
    {
      gnssDataFile.sync();
      updateDataFileAccess(&gnssDataFile); //Update the file access time stamp
      lastDataLogSyncTime = millis();
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
