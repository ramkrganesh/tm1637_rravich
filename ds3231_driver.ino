#include "Wire.h"
#include "ds3231_driver.h"

/**
* Brief Function to initialize the time machine of DS3231.
* Call this function only once during the "Setup" stage.
*/
void SetTimeInDS3231(void)
{
  Wire.begin(); // join as a controller.
  Wire.beginTransmission(0x68);
  Wire.write(REG_DS3231_SECONDS); //- Point to address 0x00
  Wire.write(INITIAL_TIME_SEC);   //- Write the seconds
  Wire.write(INITIAL_TIME_MIN);   //- Minutes
  Wire.write(INITIAL_TIME_HR);    //- 2PM (hr)
  Wire.write(INITIAL_TIME_DAY);   //- Saturday
  Wire.write(INITIAL_TIME_DATE);  //- Date = 26th
  Wire.write(INITIAL_TIME_MONTH); //- April
  Wire.write(INITIAL_TIME_YEAR);  //- Year = 25
  Wire.endTransmission();
}

/**
* Brief Call this function periodically and get the current time.
*/
Time_t GetTimeFromDS3231(void)
{
  Time_t ret_time;

  Wire.beginTransmission((byte)DS3231_DEVICE_ADDR);
  Wire.write(REG_DS3231_SECONDS);         //- Perform a dummy start and point to the register address 0x00
  Wire.endTransmission();

  Wire.requestFrom(DS3231_DEVICE_ADDR,3); //- Only read the Sec,Min,Hr
  if(Wire.available())                    //- If data received, available = 3
  {
    ret_time.sec = Wire.read();
    ret_time.min = Wire.read();
    ret_time.hr  = Wire.read();
    
    ret_time.sec = (((ret_time.sec & 0xF0) >> 4) * 10) + (ret_time.sec & 0x0F);
    ret_time.min = (((ret_time.min & 0xF0) >> 4) * 10) + (ret_time.min & 0x0F);
    ret_time.hr  = (ret_time.hr & 0x10 == 0x10? 10u : 0u) + (ret_time.hr & 0xF);

#if defined (DEBUG_ENABLED)
    Serial.print("Time: ");
    Serial.print(ret_time.hr, DEC);
    Serial.print(":");
    Serial.print(ret_time.min, DEC);
    Serial.print(":");
    Serial.println(ret_time.sec, DEC);
#endif
  }
  return ret_time;
}