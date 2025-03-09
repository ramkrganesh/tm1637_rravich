#include "Arduino.h"
//- MACRO DEFINITIONS------------------------------------------------------------------------------
#define SERIAL_ENABLE 0u // Macro to enable\disable Serial print

#define CAN_SCLK  7u
#define CAN_SDAT  6u

#define GER_SCLK  5u
#define GER_SDAT  4u

#define IND_SCLK  3u
#define IND_SDAT  2u

#define RISING_EDGE(pin)  pinMode((pin), INPUT);bitdelay(2u);
#define FALLING_EDGE(pin) pinMode((pin), OUTPUT);bitdelay(2u);

//- Macros related to Chapter 1.Data command setting
#define SET_DATA_CMD    0x40u //- includes setting for B1, B0
#define ADDR_MODE_MASK  0x04u

//- Macros related to Chapter 2.Address command setting
#define SET_ADDR_CMD  0xC0u
#define C0H 0x00u
#define C1H 0x01u
#define C2H 0x02u
#define C3H 0x03u
#define C4H 0x04u
#define C5H 0x05u

//- Macros related to chapter 3.Display control
#define SET_DISP_CMD        0x80u
#define BRIGHTNESS_LVL_MAX  0x07u
#define BRIGHTNESS_LVL_MED  0x03u
#define BRIGHTNESS_LVL_LOW  0x00u
#define DISPLAY_ON_MASK     0x08u
#define DISPLAY_OFF_MASK    0x00u


//- Type Definitions ------------------------------------------------------------------------------
typedef enum {
  AUTO_ADDR = 0u,
  FIX_ADDR,
}AddressingType;

typedef enum {
  ALL_OK,
  ERROR_DETECTED,
}ErrorStatusType;

typedef enum {
  FIRST,
  SUBSEQUENT,
}StartTxType;

typedef byte uint8;

typedef enum {
  DATA,
  CMD,
}PayloadType;