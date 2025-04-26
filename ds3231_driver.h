#ifndef DS3231_DRIVER_H
#define DS3231_DRIVER_H

/* Macros for DS3231*/
#define REG_DS3231_SECONDS  0x00
#define REG_DS3231_MINUTES  0x01
#define REG_DS3231_HOUR     0x02
#define REG_DS3231_DAY      0x03
#define REG_DS3231_DATE     0x04
#define REG_DS3231_MONTH    0x05
#define REG_DS3231_YEAR     0x06

#define REG_DS3231_SECONDS  0x00
#define REG_DS3231_MINUTES  0x01
#define REG_DS3231_HOUR     0x02
#define REG_DS3231_DAY      0x03
#define REG_DS3231_DATE     0x04
#define REG_DS3231_MONTH    0x05
#define REG_DS3231_YEAR     0x06

#define DS3231_DEVICE_ADDR  0x68u
#define INITIAL_TIME_HR     0x62u //- 2PM
#define INITIAL_TIME_MIN    0x00u
#define INITIAL_TIME_SEC    0x00u

#define INITIAL_TIME_DATE   0x26u
#define INITIAL_TIME_DAY    0x07u //- Saturday
#define INITIAL_TIME_MONTH  0x04u
#define INITIAL_TIME_YEAR   0x25u

/**
* Brief Structure containing time details (Hr, Min, Sec)
*/
typedef struct {
  byte hr;
  byte min;
  byte sec;
}Time_t;

#endif /* DS3231_DRIVER_H */
