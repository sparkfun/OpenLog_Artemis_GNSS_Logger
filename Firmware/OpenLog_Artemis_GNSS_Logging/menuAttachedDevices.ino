//Configure the attached GNSS module
void menuConfigure_uBlox()
{
  while (1)
  {
    Serial.println();
    Serial.print(F("Menu: Configure uBlox GNSS Receiver "));

    if (qwiicOnline.uBlox == false)
    {
      Serial.println(F("\r\n**No GNSS device detected on Qwiic bus**"));
    }
    else
    {
      //Print the module information
      Serial.println(gpsSensor_ublox.getModuleName());

      Serial.print(F(" 1) GNSS Logging                                           : "));
      if (settings.sensor_uBlox.log == true) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      if (settings.sensor_uBlox.log == true)
      {
        Serial.print(F(" 2) Use a power management task to put the module to sleep : "));
        if (settings.sensor_uBlox.powerManagement == true) Serial.println(F("Yes"));
        else Serial.println(F("No"));

        Serial.print(F(" 3) Enable GPS constellation                               : "));
        if (settings.sensor_uBlox.enableGPS) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));

        Serial.print(F(" 4) Enable GLONASS constellation                           : "));
        if (settings.sensor_uBlox.enableGLO) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));

        Serial.print(F(" 5) Enable Galileo constellation                           : "));
        if (settings.sensor_uBlox.enableGAL) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));

        Serial.print(F(" 6) Enable Beidou constellation                            : "));
        if (settings.sensor_uBlox.enableBDS) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));

        Serial.print(F(" 7) Enable QZSS                                            : "));
        if (settings.sensor_uBlox.enableQZSS) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));

        Serial.print(F(" 8) Disable NMEA on UART1 (helps at high message rates)    : "));
        if (settings.sensor_uBlox.disableNMEAOnUART1) Serial.println(F("Yes"));
        else Serial.println(F("No"));

        Serial.println(F(" 9) Configure UBX logging"));

        Serial.println(F("10) Configure NMEA logging"));

        Serial.flush();
      }
    }
    Serial.println(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
    {
      settings.sensor_uBlox.log ^= 1;
    }
    else if ((settings.sensor_uBlox.log == true) && (qwiicOnline.uBlox == true))
    {
      if (incoming == 2)
        settings.sensor_uBlox.powerManagement ^= 1;
      else if (incoming == 3)
        settings.sensor_uBlox.enableGPS ^= 1;
      else if (incoming == 4)
        settings.sensor_uBlox.enableGLO ^= 1;
      else if (incoming == 5)
        settings.sensor_uBlox.enableGAL ^= 1;
      else if (incoming == 6)
        settings.sensor_uBlox.enableBDS ^= 1;
      else if (incoming == 7)
        settings.sensor_uBlox.enableQZSS ^= 1;
      else if (incoming == 8)
        settings.sensor_uBlox.disableNMEAOnUART1 ^= 1;
      else if (incoming == 9)
        menuConfigure_uBloxUBX();
      else if (incoming == 10)
        menuConfigure_uBloxNMEA();
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  gnssSettingsChanged = true; //Mark gnss settings as changed so it will be started with new settings
  //qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
}

//Configure which UBX messages to log
void menuConfigure_uBloxUBX()
{
  while (1)
  {
    Serial.println();
    Serial.print(F("Menu: Configure UBX logging "));

    //Print the module information
    Serial.println(gpsSensor_ublox.getModuleName());

    Serial.print(F(" 1) Log rate for UBX-NAV-POSECEF   (Position Earth-Centered Earth-Fixed)                             : "));
    Serial.println (settings.sensor_uBlox.logUBXNAVPOSECEF);

    Serial.print(F(" 2) Log rate for UBX-NAV-STATUS    (Receiver Navigation Status)                                      : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVSTATUS);

    Serial.print(F(" 3) Log rate for UBX-NAV-DOP       (Velocity Solution North/East/Down)                               : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVDOP);

    Serial.print(F(" 4) Log rate for UBX-NAV-ATT       (Attitude Solution) (ADR / UDR Only)                              : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVATT);

    Serial.print(F(" 5) Log rate for UBX-NAV-PVT       (Position, Velocity, Time) (Used to set OLA RTC to UTC)           : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVPVT);

    Serial.print(F(" 6) Log rate for UBX-NAV-ODO       (Odometer)                                                        : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVODO);

    Serial.print(F(" 7) Log rate for UBX-NAV-VELECEF   (Velocity Solution Earth-Centered Earth-Fixed)                    : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVVELECEF);

    Serial.print(F(" 8) Log rate for UBX-NAV-VELNED    (Velocity Solution North/East/Down)                               : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVVELNED);

    Serial.print(F(" 9) Log rate for UBX-NAV-HPPOSECEF (High Precision Position Earth-Centered Earth-Fixed) (HPG Only)   : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVHPPOSECEF);

    Serial.print(F("10) Log rate for UBX-NAV-HPPOSLLH  (High Precision Position Lat/Lon/Height) (HPG Only)               : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVHPPOSLLH);

    Serial.print(F("11) Log rate for UBX-NAV-CLOCK     (Clock Solution)                                                  : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVCLOCK);

    Serial.print(F("12) Log rate for UBX-NAV-RELPOSNED (Relative Position North/East/Down) (HPG Only)                    : "));
    Serial.println(settings.sensor_uBlox.logUBXNAVRELPOSNED);

    Serial.print(F("13) Log rate for UBX-RXM-SFRBX     (Broadcast Navigation Data Subframe) (HPG / Time Sync Only)       : "));
    Serial.println(settings.sensor_uBlox.logUBXRXMSFRBX);

    Serial.print(F("14) Log rate for UBX-RXM-RAWX      (Multi-GNSS Raw Measurement) (ADR / HPG / Time Sync Only)         : "));
    Serial.println(settings.sensor_uBlox.logUBXRXMRAWX);

    Serial.print(F("15) Log rate for UBX-TIM-TM2       (Time Mark Data)                                                  : "));
    Serial.println(settings.sensor_uBlox.logUBXTIMTM2);

    Serial.print(F("16) Log rate for UBX-ESF-MEAS      (External Sensor Fusion Measurements) (ADR / UDR Only)            : "));
    Serial.println(settings.sensor_uBlox.logUBXESFMEAS);

    Serial.print(F("17) Log rate for UBX-ESF-RAW       (External Sensor Fusion Raw Sensor Measurements) (ADR / UDR Only) : "));
    Serial.println(settings.sensor_uBlox.logUBXESFRAW);

    Serial.print(F("18) Log rate for UBX-ESF-STATUS    (External Sensor Fusion Status) (ADR / USR Only)                  : "));
    Serial.println(settings.sensor_uBlox.logUBXESFSTATUS);

    Serial.print(F("19) Log rate for UBX-ESF-ALG       (External Sensor Fusion IMU Alignment) (ADR / UDR Only)           : "));
    Serial.println(settings.sensor_uBlox.logUBXESFALG);

    Serial.print(F("20) Log rate for UBX-ESF-INS       (External Sensor Fusion Vehicle Dynamics) (ADR / UDR Only)        : "));
    Serial.println(settings.sensor_uBlox.logUBXESFINS);

    Serial.print(F("21) Log rate for UBX-HNR-PVT       (High Navigation Rate Position Velocity Time) (ADR / UDR Only)    : "));
    Serial.println(settings.sensor_uBlox.logUBXHNRPVT);

    Serial.print(F("22) Log rate for UBX-HNR-ATT       (High Navigation Rate Attitude Solution) (ADR / UDR Only)         : "));
    Serial.println(settings.sensor_uBlox.logUBXHNRATT);

    Serial.print(F("23) Log rate for UBX-HNR-INS       (High Navigation Rate Vehicle Dynamics) (ADR / UDR Only)          : "));
    Serial.println(settings.sensor_uBlox.logUBXHNRINS);

    Serial.print(F("24) Log rate for UBX-RXM-MEASX     (Satellite measurements for RRLP)                                 : "));
    Serial.println(settings.sensor_uBlox.logUBXRXMMEASX);

    Serial.flush();

    Serial.println(F(" x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
      setLogRate(&settings.sensor_uBlox.logUBXNAVPOSECEF);
    else if (incoming == 2)
      setLogRate(&settings.sensor_uBlox.logUBXNAVSTATUS);
    else if (incoming == 3)
      setLogRate(&settings.sensor_uBlox.logUBXNAVDOP);
    else if (incoming == 4)
      setLogRate(&settings.sensor_uBlox.logUBXNAVATT);
    else if (incoming == 5)
      setLogRate(&settings.sensor_uBlox.logUBXNAVPVT);
    else if (incoming == 6)
      setLogRate(&settings.sensor_uBlox.logUBXNAVODO);
    else if (incoming == 7)
      setLogRate(&settings.sensor_uBlox.logUBXNAVVELECEF);
    else if (incoming == 8)
      setLogRate(&settings.sensor_uBlox.logUBXNAVVELNED);
    else if (incoming == 9)
      setLogRate(&settings.sensor_uBlox.logUBXNAVHPPOSECEF);
    else if (incoming == 10)
      setLogRate(&settings.sensor_uBlox.logUBXNAVHPPOSLLH);
    else if (incoming == 11)
      setLogRate(&settings.sensor_uBlox.logUBXNAVCLOCK);
    else if (incoming == 12)
      setLogRate(&settings.sensor_uBlox.logUBXNAVRELPOSNED);
    else if (incoming == 13)
      setLogRate(&settings.sensor_uBlox.logUBXRXMSFRBX);
    else if (incoming == 14)
      setLogRate(&settings.sensor_uBlox.logUBXRXMRAWX);
    else if (incoming == 15)
      setLogRate(&settings.sensor_uBlox.logUBXTIMTM2);
    else if (incoming == 16)
      setLogRate(&settings.sensor_uBlox.logUBXESFMEAS);
    else if (incoming == 17)
      setLogRate(&settings.sensor_uBlox.logUBXESFRAW);
    else if (incoming == 18)
      setLogRate(&settings.sensor_uBlox.logUBXESFSTATUS);
    else if (incoming == 19)
      setLogRate(&settings.sensor_uBlox.logUBXESFALG);
    else if (incoming == 20)
      setLogRate(&settings.sensor_uBlox.logUBXESFINS);
    else if (incoming == 21)
      setLogRate(&settings.sensor_uBlox.logUBXHNRPVT);
    else if (incoming == 22)
      setLogRate(&settings.sensor_uBlox.logUBXHNRATT);
    else if (incoming == 23)
      setLogRate(&settings.sensor_uBlox.logUBXHNRINS);
    else if (incoming == 24)
      setLogRate(&settings.sensor_uBlox.logUBXRXMMEASX);
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

//Configure which NMEA messages to log
void menuConfigure_uBloxNMEA()
{
  while (1)
  {
    Serial.println();
    Serial.print(F("Menu: Configure NMEA logging"));
    Serial.println();

    Serial.print(F(" 1) Log rate for NMEA DTM (Datum Reference)                                : "));
    Serial.println(settings.sensor_uBlox.logNMEADTM);

    Serial.print(F(" 2) Log rate for NMEA GBS (Satellite Fault Detection)                      : "));
    Serial.println(settings.sensor_uBlox.logNMEAGBS);

    Serial.print(F(" 3) Log rate for NMEA GGA (Global Positioning System Fix)                  : "));
    Serial.println(settings.sensor_uBlox.logNMEAGGA);

    Serial.print(F(" 4) Log rate for NMEA GLL (Latitude and Longitude Position Fix and Status) : "));
    Serial.println(settings.sensor_uBlox.logNMEAGLL);

    Serial.print(F(" 5) Log rate for NMEA GNS (GNSS Fix Data)                                  : "));
    Serial.println(settings.sensor_uBlox.logNMEAGNS);

    Serial.print(F(" 6) Log rate for NMEA GRS (GNSS Range Residuals)                           : "));
    Serial.println(settings.sensor_uBlox.logNMEAGRS);

    Serial.print(F(" 7) Log rate for NMEA GSA (DOP and Active Satellites)                      : "));
    Serial.println(settings.sensor_uBlox.logNMEAGSA);

    Serial.print(F(" 8) Log rate for NMEA GST (GNSS Pseudorange Error Statistics)              : "));
    Serial.println(settings.sensor_uBlox.logNMEAGST);

    Serial.print(F(" 9) Log rate for NMEA GSV (GNSS Satellites In View)                        : "));
    Serial.println(settings.sensor_uBlox.logNMEAGSV);

    Serial.print(F("10) Log rate for NMEA RLM (Return Link Message)                            : "));
    Serial.println(settings.sensor_uBlox.logNMEARLM);

    Serial.print(F("11) Log rate for NMEA RMC (Recommended Minimum Data)                       : "));
    Serial.println(settings.sensor_uBlox.logNMEARMC);

    Serial.print(F("12) Log rate for NMEA TXT (Text Transmission)                              : "));
    Serial.println(settings.sensor_uBlox.logNMEATXT);

    Serial.print(F("13) Log rate for NMEA VLW (Dual Ground / Water Distance)                   : "));
    Serial.println(settings.sensor_uBlox.logNMEAVLW);

    Serial.print(F("14) Log rate for NMEA VTG (Course Over Ground and Ground Speed)            : "));
    Serial.println(settings.sensor_uBlox.logNMEAVTG);

    Serial.print(F("15) Log rate for NMEA ZDA (Time and Date)                                  : "));
    Serial.println(settings.sensor_uBlox.logNMEAZDA);

    Serial.flush();

    Serial.println(F(" x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
      setLogRate(&settings.sensor_uBlox.logNMEADTM);
    else if (incoming == 2)
      setLogRate(&settings.sensor_uBlox.logNMEAGBS);
    else if (incoming == 3)
      setLogRate(&settings.sensor_uBlox.logNMEAGGA);
    else if (incoming == 4)
      setLogRate(&settings.sensor_uBlox.logNMEAGLL);
    else if (incoming == 5)
      setLogRate(&settings.sensor_uBlox.logNMEAGNS);
    else if (incoming == 6)
      setLogRate(&settings.sensor_uBlox.logNMEAGRS);
    else if (incoming == 7)
      setLogRate(&settings.sensor_uBlox.logNMEAGSA);
    else if (incoming == 8)
      setLogRate(&settings.sensor_uBlox.logNMEAGST);
    else if (incoming == 9)
      setLogRate(&settings.sensor_uBlox.logNMEAGSV);
    else if (incoming == 10)
      setLogRate(&settings.sensor_uBlox.logNMEARLM);
    else if (incoming == 11)
      setLogRate(&settings.sensor_uBlox.logNMEARMC);
    else if (incoming == 12)
      setLogRate(&settings.sensor_uBlox.logNMEATXT);
    else if (incoming == 13)
      setLogRate(&settings.sensor_uBlox.logNMEAVLW);
    else if (incoming == 14)
      setLogRate(&settings.sensor_uBlox.logNMEAVTG);
    else if (incoming == 15)
      setLogRate(&settings.sensor_uBlox.logNMEAZDA);
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void setLogRate(uint8_t *rate)
{
  Serial.println(F("\r\nEnter the new log rate (0 to 255): "));
  Serial.println(F("0 = disabled, 1 = log every interval, 2 = log every 2nd interval, etc."));
  int newRate = getNumber(menuTimeout); //Timeout after x seconds
  if (newRate < 0 || newRate > 255)
    Serial.println(F("Error: log rate out of range\r\n"));
  else
    *rate = (uint8_t)newRate;
}
