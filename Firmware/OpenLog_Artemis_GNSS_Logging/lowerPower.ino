// Read the battery voltage
// If it is low, increment lowBatteryReadings
// If lowBatteryReadings exceeds lowBatteryReadingsLimit then powerDown
void checkBattery(void)
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  if (settings.enableLowBatteryDetection == true)
  {
    float voltage = readVIN(); // Read the battery voltage
    if (voltage < settings.lowBatteryThreshold) // Is the voltage low?
    {
      lowBatteryReadings++; // Increment the low battery count
      if (lowBatteryReadings > lowBatteryReadingsLimit) // Have we exceeded the low battery count limit?
      {
        // Gracefully powerDown

        //Save files before going to sleep
        if (online.dataLogging == true)
        {
          unsigned long pauseUntil = millis() + 550UL; //Wait > 500ms so we can be sure SD data is sync'd
          while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
          {
            storeData();
          }

          storeFinalData();
          gnssDataFile.sync();
      
          updateDataFileAccess(&gnssDataFile); //Update the file access time stamp
      
          gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
        }

        delay(sdPowerDownDelay); // Give the SD card time to finish writing ***** THIS IS CRITICAL *****

        Serial.println(F("***      LOW BATTERY VOLTAGE DETECTED! GOING INTO POWERDOWN      ***"));
        Serial.println(F("*** PLEASE CHANGE THE POWER SOURCE AND RESET THE OLA TO CONTINUE ***"));
      
        Serial.flush(); //Finish any prints

        powerDown(); // power down and wait for reset
      }
    }
    else
    {
      lowBatteryReadings = 0; // Reset the low battery count (to reject noise)
    }    
  }
#endif
}

//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
//With leakage across the 3.3V protection diode, it's approx 3.00uA.
void powerDown()
{
  if (ignorePowerLossInterrupt) //attachInterrupt causes the interrupt to trigger with v2.1.0 of the core. Ignore the interrupt if required.
    return;
    
  detachInterrupt(PIN_POWER_LOSS); //Prevent voltage supervisor from waking us from sleep

  //Prevent stop logging button from waking us from sleep
  if (settings.useGPIO32ForStopLogging == true)
  {
    detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
    pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
  }

  //WE NEED TO POWER DOWN ASAP - we don't have time to close the SD files
  //Close file before going to sleep
  //  if (online.dataLogging == true)
  //  {
  //    gnssDataFile.sync();
  //  }

  //Serial.flush(); //Don't waste time waiting for prints to finish

  qwiic.end(); //Power down I2C

  SPI.end(); //Power down SPI

  powerControlADC(false); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART

  //Force the peripherals off
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

  //Disable pads
  for (int x = 0; x < 50; x++)
  {
    if ((x != PIN_POWER_LOSS) &&
        //(x != PIN_LOGIC_DEBUG) &&
        (x != PIN_MICROSD_POWER) &&
        (x != PIN_QWIIC_POWER) &&
        (x != PIN_IMU_POWER))
    {
      am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);
    }
  }

  //powerLEDOff();
  
  //Make sure PIN_POWER_LOSS is configured as an input for the WDT
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();

  //Keep Qwiic bus powered on if user desires it - but only for X04 to avoid a brown-out
#if(HARDWARE_VERSION_MAJOR == 0)
  if (settings.powerDownQwiicBusBetweenReads == true)
    qwiicPowerOff();
  else
    qwiicPowerOn(); //Make sure pins stays as output
#else
  qwiicPowerOff();
#endif

  //Power down cache, flash, SRAM
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); // Power down all flash and cache
  am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_384K); // Retain all SRAM

  //Keep the 32kHz clock running for RTC
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

  while (1) // Stay in deep sleep until we get reset
  {
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP); //Sleep
  }
}

//Power everything down and wait for interrupt wakeup
void goToSleep()
{
  detachInterrupt(PIN_POWER_LOSS); //Prevent voltage supervisor from waking us from sleep

  //Prevent stop logging button from waking us from sleep
  if (settings.useGPIO32ForStopLogging == true)
  {
    detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
    pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
  }
  
  if (qwiicAvailable.uBlox && qwiicOnline.uBlox) //If the u-blox is available and logging
  {
    //Disable all messages otherwise they will fill up the module's I2C buffer while we are asleep
    //(Possibly redundant if using a power management task?)
    disableMessages(1100);
    //Using a maxWait of zero means we don't wait for the ACK/NACK
    //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)
  }

  //Save files before going to sleep
  if (online.dataLogging == true)
  {
    unsigned long pauseUntil = millis() + 550UL; //Wait > 500ms so we can be sure SD data is sync'd
    while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
    {
      storeData();
    }

    storeFinalData();
    gnssDataFile.sync();

    updateDataFileAccess(&gnssDataFile); //Update the file access time stamp

    gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098

    delay(sdPowerDownDelay); // Give the SD card time to finish writing ***** THIS IS CRITICAL *****
  }

  uint32_t msToSleep = (uint32_t)(settings.usSleepDuration / 1000ULL);

  //Check if we are using a power management task to put the module to sleep
  if (qwiicAvailable.uBlox && qwiicOnline.uBlox && (settings.sensor_uBlox.powerManagement == true))
  {
    powerManagementTask((msToSleep - 1000), 0); // Wake the module up 1s before the Artemis so it is ready to rock
    //UBX_RXM_PMREQ does not ACK so there is no point in checking the return value
    if (settings.printMajorDebugMessages == true)
    {
      Serial.println(F("goToSleep: powerManagementTask request sent")); 
    }             
  }

  //qwiic.end(); //DO NOT Power down I2C - causes badness with v2.1 of the core: https://github.com/sparkfun/Arduino_Apollo3/issues/412

  SPI.end(); //Power down SPI

  powerControlADC(false); //Power down ADC. It it started by default before setup().

  Serial.flush(); //Finish any prints
  Serial.end(); //Power down UART

  //Counter/Timer 6 will use the 32kHz clock
  //Calculate how many 32768Hz system ticks we need to sleep for:
  //sysTicksToSleep = msToSleep * 32768L / 1000
  //We need to be careful with the multiply as we will overflow uint32_t if msToSleep is > 131072
  uint32_t sysTicksToSleep;
  if (msToSleep < 131000)
  {
    sysTicksToSleep = msToSleep * 32768L; // Do the multiply first for short intervals
    sysTicksToSleep = sysTicksToSleep / 1000L; // Now do the divide
  }
  else
  {
    sysTicksToSleep = msToSleep / 1000L; // Do the division first for long intervals (to avoid an overflow)
    sysTicksToSleep = sysTicksToSleep * 32768L; // Now do the multiply
  }
  
  //Force the peripherals off

  //With v2.1 of the core, very bad things happen if the IOMs are disabled.
  //We must leave them enabled: https://github.com/sparkfun/Arduino_Apollo3/issues/412
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0); // SPI
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1); // qwiic I2C
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
  //if (settings.useTxRxPinsForTerminal == true)
  //  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

  //Disable as many pins as we can
  const int pinsToDisable[] = {0,1,2,10,14,17,12,24,25,28,36,38,39,40,41,42,43,45,21,22,16,31,35,-1};
  for (int x = 0; pinsToDisable[x] >= 0; x++)
  {
    am_hal_gpio_pinconfig(pinsToDisable[x], g_AM_HAL_GPIO_DISABLE);
  }

  //Do disable CIPO, COPI, SCLK and chip selects to minimise the current draw during deep sleep
  am_hal_gpio_pinconfig(PIN_SPI_CIPO , g_AM_HAL_GPIO_DISABLE); //ICM / microSD CIPO
  am_hal_gpio_pinconfig(PIN_SPI_COPI , g_AM_HAL_GPIO_DISABLE); //ICM / microSD COPI
  am_hal_gpio_pinconfig(PIN_SPI_SCK , g_AM_HAL_GPIO_DISABLE); //ICM / microSD SCK
  am_hal_gpio_pinconfig(PIN_IMU_CHIP_SELECT , g_AM_HAL_GPIO_DISABLE); //ICM CS
  am_hal_gpio_pinconfig(PIN_IMU_INT , g_AM_HAL_GPIO_DISABLE); //ICM INT
  am_hal_gpio_pinconfig(PIN_MICROSD_CHIP_SELECT , g_AM_HAL_GPIO_DISABLE); //microSD CS

  //Do disable qwiic SDA and SCL to minimise the current draw during deep sleep
  am_hal_gpio_pinconfig(PIN_QWIIC_SDA , g_AM_HAL_GPIO_DISABLE);
  am_hal_gpio_pinconfig(PIN_QWIIC_SCL , g_AM_HAL_GPIO_DISABLE);

  //Disable pins 48 and 49 (UART0) to stop them back-feeding the CH340
  am_hal_gpio_pinconfig(48 , g_AM_HAL_GPIO_DISABLE); //TX0
  am_hal_gpio_pinconfig(49 , g_AM_HAL_GPIO_DISABLE); //RX0

  //Make sure PIN_POWER_LOSS is configured as an input for the WDT
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();

  //Keep Qwiic bus powered on if user desires it
  if (settings.powerDownQwiicBusBetweenReads == true)
    qwiicPowerOff();
  else
    qwiicPowerOn(); //Make sure pins stays as output

  //Leave the power LED on if the user desires it
  if (settings.enablePwrLedDuringSleep == true)
    powerLEDOn();
  else
    powerLEDOff();

  //Power down cache, flash, SRAM
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); // Power down all flash and cache
  am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_384K); // Retain all SRAM

  //Use the lower power 32kHz clock. Use it to run CT6 as well.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Check that sysTicksToSleep is >> sysTicksAwake
  if (sysTicksToSleep > 3277) // Abort if we are trying to sleep for < 100ms
  {
    //Setup interrupt to trigger when the number of ms have elapsed
    am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

    //We use counter/timer 6 to cause us to wake up from sleep but 0 to 7 are available
    //CT 7 is used for Software Serial. All CTs are used for Servo.
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG);  //Clear CT6
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); //Enable C/T G=6

    //Enable the timer interrupt in the NVIC.
    NVIC_EnableIRQ(STIMER_CMPR6_IRQn);

    //Deep Sleep
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    //Turn off interrupt
    NVIC_DisableIRQ(STIMER_CMPR6_IRQn);
    am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREG); //Disable C/T G=6
  }

  //We're BACK!
  wakeFromSleep();
}

//Power everything up gracefully
void wakeFromSleep()
{
  //Go back to using the main clock
  //am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
  //NVIC_EnableIRQ(STIMER_IRQn);
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ);

  // Power up SRAM, turn on entire Flash
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_MAX);

  //Turn on ADC
  powerControlADC(true);

  //Run setup again

  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up
  pin_config(PinName(PIN_POWER_LOSS), g_AM_HAL_GPIO_INPUT); // Make sure the pin does actually get re-configured
  
  delay(1); // Let PIN_POWER_LOSS stabilize

  if (digitalRead(PIN_POWER_LOSS) == LOW) powerDown(); //Check PIN_POWER_LOSS just in case we missed the falling edge
  ignorePowerLossInterrupt = true; // Ignore the power loss interrupt - when attaching the interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);
  ignorePowerLossInterrupt = false;

  if (settings.useGPIO32ForStopLogging == true)
  {
    pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
    pin_config(PinName(PIN_STOP_LOGGING), g_AM_HAL_GPIO_INPUT_PULLUP); // Make sure the pin does actually get re-configured
    delay(1); // Let the pin stabilize
    attachInterrupt(PIN_STOP_LOGGING, stopLoggingISR, FALLING); // Enable the interrupt
    am_hal_gpio_pincfg_t intPinConfig = g_AM_HAL_GPIO_INPUT_PULLUP;
    intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
    pin_config(PinName(PIN_STOP_LOGGING), intPinConfig); // Make sure the pull-up does actually stay enabled
    stopLoggingSeen = false; // Make sure the flag is clear
  }

  pinMode(PIN_STAT_LED, OUTPUT);
  pin_config(PinName(PIN_STAT_LED), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_STAT_LED, LOW);

  powerLEDOn();

  if (PIN_LOGIC_DEBUG >= 0)
  {
    pinMode(PIN_LOGIC_DEBUG, OUTPUT); //Debug pin
    digitalWrite(PIN_LOGIC_DEBUG, HIGH); //Make this high, trigger debug on falling edge
  }

  //Re-enable pins 48 and 49 (UART0)
  pin_config(PinName(48), g_AM_BSP_GPIO_COM_UART_TX);    
  pin_config(PinName(49), g_AM_BSP_GPIO_COM_UART_RX);

  //Re-enable CIPO, COPI, SCK and the chip selects but may as well leave ICM_INT disabled
  pin_config(PinName(PIN_SPI_CIPO), g_AM_BSP_GPIO_IOM0_MISO);
  pin_config(PinName(PIN_SPI_COPI), g_AM_BSP_GPIO_IOM0_MOSI);
  pin_config(PinName(PIN_SPI_SCK), g_AM_BSP_GPIO_IOM0_SCK);
  pin_config(PinName(PIN_IMU_CHIP_SELECT), g_AM_HAL_GPIO_OUTPUT);
  pin_config(PinName(PIN_MICROSD_CHIP_SELECT) , g_AM_HAL_GPIO_OUTPUT);

  //Re-enable the SDA and SCL pins
  pin_config(PinName(PIN_QWIIC_SCL), g_AM_BSP_GPIO_IOM1_SCL);
  pin_config(PinName(PIN_QWIIC_SDA), g_AM_BSP_GPIO_IOM1_SDA);

  Serial.begin(settings.serialTerminalBaudRate);

  beginSerialOutput();

  beginQwiic(); //Power up Qwiic bus as early as possible

  SPI.begin(); //Needed if SD is disabled

  beginSD(); //285 - 293ms

  for (int i = 0; i < 250; i++) // Allow extra time for the qwiic sensors to power up
  {
    checkBattery(); // Check for low battery
    delay(1);
  }

  beginDataLogging(); //180ms

  disableIMU(); //Disable IMU

  if (qwiicOnline.uBlox == false) //Check if we powered down the module
  {
    beginSensors(); //Restart the module from scratch
  }
  else
  {
    //Module is still online so (re)enable the selected messages
    enableMessages(1100);
  }
}

void stopLogging(void)
{
  detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
  
  if (qwiicAvailable.uBlox && qwiicOnline.uBlox) //If the u-blox is available and logging
  {
    //Disable all messages
    disableMessages(1100);
    //Using a maxWait of zero means we don't wait for the ACK/NACK
    //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)
  }

  //Save files before going to sleep
  if (online.dataLogging == true)
  {
    unsigned long pauseUntil = millis() + 550UL; //Wait > 500ms so we can be sure SD data is sync'd
    while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
    {
      storeData();
    }

    storeFinalData();
    gnssDataFile.sync();

    updateDataFileAccess(&gnssDataFile); //Update the file access time stamp

    gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
  }

  Serial.print("Logging is stopped. Please reset OpenLog Artemis and open a terminal at ");
  Serial.print((String)settings.serialTerminalBaudRate);
  Serial.println("bps...");
  delay(sdPowerDownDelay); // Give the SD card time to shut down and for the serial message to send
  powerDown();
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 4)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#elif(HARDWARE_VERSION_MAJOR == 1 && HARDWARE_VERSION_MINOR == 0)
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#endif
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 4)
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#elif(HARDWARE_VERSION_MAJOR == 1 && HARDWARE_VERSION_MINOR == 0)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#endif
}

void microSDPowerOn()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, LOW);
}
void microSDPowerOff()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}

void imuPowerOn()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, HIGH);
}
void imuPowerOff()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, LOW);
}

void powerLEDOn()
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH); // Turn the Power LED on  
#endif  
}
void powerLEDOff()
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, LOW); // Turn the Power LED off
#endif  
}

//Returns the number of milliseconds according to the RTC
//(In increments of 10ms)
//Watch out for the year roll-over!
uint64_t rtcMillis()
{
  myRTC.getTime();
  uint64_t millisToday = 0;
  int dayOfYear = calculateDayOfYear(myRTC.dayOfMonth, myRTC.month, myRTC.year + 2000);
  millisToday += ((uint64_t)dayOfYear * 86400000ULL);
  millisToday += ((uint64_t)myRTC.hour * 3600000ULL);
  millisToday += ((uint64_t)myRTC.minute * 60000ULL);
  millisToday += ((uint64_t)myRTC.seconds * 1000ULL);
  millisToday += ((uint64_t)myRTC.hundredths * 10ULL);

  return(millisToday);  
}

//Returns the day of year
//https://gist.github.com/jrleeman/3b7c10712112e49d8607
int calculateDayOfYear(int day, int month, int year)
{  
  // Given a day, month, and year (4 digit), returns 
  // the day of year. Errors return 999.
  
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  
  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }
  
  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year%4  == 0) {
    if (year%100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year%400 == 0) {
        daysInMonth[1] = 29;
      }
    }
   }

  // Make sure we are on a valid day of the month
  if (day < 1) 
  {
    return 999;
  } else if (day > daysInMonth[month-1]) {
    return 999;
  }
  
  int doy = 0;
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }
  
  doy += day;
  return doy;
}
