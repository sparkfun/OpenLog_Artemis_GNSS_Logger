void SerialPrintTimeString()
{
  char timeString[40];
  getTimeString(timeString);
  Serial.print(timeString);
}

//This function gets called from the SparkFun u-blox Arduino Library
//As each NMEA character comes in you can specify what to do with it
void DevUBLOXGNSS::processNMEA(char incoming)
{
  if (settings.enableTerminalOutput == true)
    Serial.write(incoming);
}

void callbackNAVPOSECEF(UBX_NAV_POSECEF_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-POSECEF"));
  }
}

void callbackNAVSTATUS(UBX_NAV_STATUS_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-STATUS"));
  }
}

void callbackNAVDOP(UBX_NAV_DOP_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-DOP"));
  }
}

void callbackNAVATT(UBX_NAV_ATT_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-ATT"));
  }
}

void callbackNAVPVT(UBX_NAV_PVT_data_t *ubxDataStruct)
{
  //Check if we need to update the RTC
  if (rtcNeedsSync == true) //Do we need to sync the RTC
  {
    if (ubxDataStruct->flags2.bits.confirmedDate && ubxDataStruct->flags2.bits.confirmedTime) //Are the date and time valid?
    {
      //If nanos is negative, set it to zero
      //The ZED-F9P Integration Manual says: "the nano value can range from -5000000 (i.e. -5 ms) to +994999999 (i.e. nearly 995 ms)."
      //"if a resolution of one hundredth of a second is adequate, negative nano values can simply be rounded up to 0 and effectively ignored."
      uint8_t centis;
      if (ubxDataStruct->nano < 0)
        centis = 0;
      else
        centis = (uint8_t)(ubxDataStruct->nano / 10000000); //Convert nanos to hundredths (centiseconds)
      myRTC.setTime(centis, ubxDataStruct->sec, ubxDataStruct->min, ubxDataStruct->hour, ubxDataStruct->day, ubxDataStruct->month, (ubxDataStruct->year - 2000)); //Set the RTC
      rtcHasBeenSyncd = true; //Set rtcHasBeenSyncd to show RTC has been sync'd
      rtcNeedsSync = false; //Clear rtcNeedsSync so we don't set the RTC multiple times
      if (settings.printMinorDebugMessages == true)
      {
        Serial.printf("callbackNAVPVT: RTC sync'd to %04d/%02d/%02d %02d:%02d:%02d.%02d\r\n", ubxDataStruct->year, ubxDataStruct->month, ubxDataStruct->day, ubxDataStruct->hour, ubxDataStruct->min, ubxDataStruct->sec, centis);
      }
    }
  }

  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();
  
    //Print the frame information
    Serial.println(F(" NAV-PVT"));
  }
}

void callbackNAVODO(UBX_NAV_ODO_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-ODO"));
  }
}

void callbackNAVVELECEF(UBX_NAV_VELECEF_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-VELECEF"));
  }
}

void callbackNAVVELNED(UBX_NAV_VELNED_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-VELNED"));
  }
}

void callbackNAVHPPOSECEF(UBX_NAV_HPPOSECEF_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-HPPOSECEF"));
  }
}

void callbackNAVHPPOSLLH(UBX_NAV_HPPOSLLH_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-HPPOSLLH"));
  }
}

void callbackNAVCLOCK(UBX_NAV_CLOCK_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-CLOCK"));
  }
}

void callbackNAVRELPOSNED(UBX_NAV_RELPOSNED_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" NAV-RELPOSNED"));
  }
}

void callbackRXMSFRBX(UBX_RXM_SFRBX_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" RXM-SFRBX"));
  }
}

void callbackRXMRAWX(UBX_RXM_RAWX_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" RXM-RAWX"));
  }
}

void callbackRXMMEASX(UBX_RXM_MEASX_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" RXM-MEASX"));
  }
}

void callbackTIMTM2(UBX_TIM_TM2_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" TIM-TM2"));
  }
}

void callbackESFALG(UBX_ESF_ALG_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" ESF-ALG"));
  }
}

void callbackESFINS(UBX_ESF_INS_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" ESF-INS"));
  }
}

void callbackESFMEAS(UBX_ESF_MEAS_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" ESF-MEAS"));
  }
}

void callbackESFRAW(UBX_ESF_RAW_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" ESF-RAW"));
  }
}

void callbackESFSTATUS(UBX_ESF_STATUS_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" ESF-STATUS"));
  }
}

void callbackHNRPVT(UBX_HNR_PVT_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" HNR-PVT"));
  }
}

void callbackHNRATT(UBX_HNR_ATT_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" HNR-ATT"));
  }
}

void callbackHNRINS(UBX_HNR_INS_data_t *ubxDataStruct)
{
  if (settings.enableTerminalOutput == true)
  {
    //Print some useful information. Let's keep this message short!
    SerialPrintTimeString();

    //Print the frame information
    Serial.println(F(" HNR-INS"));
  }
}
