//- MACRO DEFINITIONS------------------------------------------------------------------------------
#define SERIAL_ENABLE 1u // Macro to enable\disable Serial print

#define SCL 8u
#define SDA 9u

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

#define TEST_DATA 0xbFu //- !! EDIT HERE !!
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
//- Global Variables ------------------------------------------------------------------------------
volatile ErrorStatusType ErrSts = ALL_OK;
volatile uint8 Current_Time[4] = {1,0,3,9};  // {h h m m}

uint8 DecimalToSegment[10] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
};

volatile uint8 Data_Cntr = 0;
boolean DotBit = true;
volatile uint8 Expired_Seconds = 0;

//- Inline functions ------------------------------------------------------------------------------
static inline void bitdelay(unsigned int delay_us)
{
  delayMicroseconds(delay_us);
}

static inline void StartComm(void)
{
  if(ErrSts != ALL_OK)
  {
    #if SERIAL_ENABLE
    Serial.println("Cannot start communication...");
    #endif
    return;
  }

  // while entering this function, status of SDA = HIGH ; SCL = HIGH
  if( digitalRead(SCL) != HIGH ||   \
      digitalRead(SDA) != HIGH)
    {
      #if SERIAL_ENABLE
      Serial.println("ERROR: Cannot start communication due to incorrect level in SDA, SCL");
      #endif
      ErrSts = ERROR_DETECTED;
      return;
    }
    FALLING_EDGE(SDA);  // Bring SDA level LOW to "Start" communication.
    bitdelay(1);
}

static inline void StopComm(void)
{
  if(ErrSts != ALL_OK)
  {
    #if SERIAL_ENABLE
    Serial.println("ERROR: StopComm: cannot stop communication");
    #endif
    return;
  }
  
  if( digitalRead(SCL) != LOW || \
      digitalRead(SDA) != LOW)
    {
      #if SERIAL_ENABLE
      Serial.println("ERROR: cannot stop communication because of SDA, SCL level.");
      #endif
      ErrSts = ERROR_DETECTED;
      return;
    }

  RISING_EDGE(SCL);
  bitdelay(1u);
  
  RISING_EDGE(SDA); // communication stopped
  bitdelay(1u);
}

void SendTo1637(uint8 TxData, PayloadType payload)
{
  uint8 SegmentedData = 0u;
  
  /* - If this function is called after a "Start", SCL=HIGH and SDA=LOW
   *  - If this function is called after a "SendTo1637, SCL=LOW and SDA=LOW */
  if(ErrSts != ALL_OK)
  {
    #if SERIAL_ENABLE
    Serial.println("ERROR: SendTo1637: cannot send data");
    #endif
    return;
  }

  if(payload == DATA)
  {
    SegmentedData = DecimalToSegment[TxData];  // Use the look up table to convert for example 5 --> 0x6D
    
    if(Data_Cntr == 1)
    {
      DotBit = !DotBit;
      SegmentedData = SegmentedData | (DotBit << 7);
    }
    TxData = SegmentedData;
  }

  for(uint8 idx = 0; idx < 8; idx++)
  {
    FALLING_EDGE(SCL);
    if(bitRead(TxData, idx))
    {
      // val = 1
      RISING_EDGE(SDA);
    }
    else
    {
      FALLING_EDGE(SDA);
    }
    RISING_EDGE(SCL);
  } // 8 rising edges of SCL will be over after the for loop

  FALLING_EDGE(SCL);    // Falling edge of 8th clock
  pinMode(SDA, OUTPUT); // Prepare for the ACK

  RISING_EDGE(SCL);  // 9th rising edge
  
  if(digitalRead(SDA) != LOW)
  {
    #if SERIAL_ENABLE
    Serial.println("ERROR: during ack.");
    #endif

    ErrSts = ERROR_DETECTED;
    return;
  }

  FALLING_EDGE(SCL); // 9th clock is finished
  // During the exit of this function SDA, SCL are LOW
}

//- Global function defitions ---------------------------------------------------------------------
void TransferData(uint8 Addr, AddressingType AddrTyp)
{
  // while entering this function, SCL=HIGH, SDA=LOW

  uint8 DataCmdSet    = (SET_DATA_CMD | (AddrTyp << 2u));
  uint8 ResolvedAddr  = (SET_ADDR_CMD | Addr);

  if(ErrSts != ALL_OK)
  {
    #if SERIAL_ENABLE
    Serial.println("ERROR: Cannot initiate data transfer.");
    #endif
    return;
  }
  
  if( digitalRead(SCL) != HIGH || \
      digitalRead(SDA) != HIGH)
    {
      #if SERIAL_ENABLE
      Serial.println("ERROR: Cannot transfer data. Previous start comm was not OK");
      #endif
      ErrSts = ERROR_DETECTED;
      return;
    }
  
  StartComm();  //- BLOCK 1 -------------------------------

  #if SERIAL_ENABLE
  Serial.println("Transmit DataCmdSet...");
  #endif

  SendTo1637(DataCmdSet, CMD);
  StopComm();   //- ---------------------------------------

  #if SERIAL_ENABLE
  Serial.println("Transmit Address...");
  #endif
  StartComm();  //- BLOCK 2--------------------------------
  SendTo1637(ResolvedAddr, CMD);

  #if SERIAL_ENABLE
  Serial.println("Transmit Data...");
  #endif
  Data_Cntr = 0;
  for(uint8 idx=0; idx < 4; idx++)
  {
      SendTo1637(Current_Time[idx], DATA);
      Data_Cntr++;
      bitdelay(1);
  }
  StopComm();   //- ---------------------------------------

  #if SERIAL_ENABLE
  Serial.println("Transmit Brightness...");
  #endif

  StartComm();  //- BLOCK 3--------------------------------
  SendTo1637(SET_DISP_CMD | DISPLAY_ON_MASK | BRIGHTNESS_LVL_MAX, CMD);
  StopComm();   //- ---------------------------------------

  #if SERIAL_ENABLE
  if (ErrSts != ERROR_DETECTED)
    Serial.println("Brightness successfully transmitted.");
  #endif
}

//- Setup -----------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600u);
  pinMode(SCL, INPUT);
  pinMode(SDA, INPUT);
}

void compute_time(void)
{
  boolean ChangeHour = false;
  uint8 m_lsb = Current_Time[3];
  uint8 m_msb = Current_Time[2];

  uint8 h_lsb = Current_Time[1];
  uint8 h_msb = Current_Time[0];

  if(m_lsb == 9)
  { //- 10mins has passed
    m_lsb = 0;
    m_msb++;

    if(m_msb > 5)
    {
      m_msb = 0;
      ChangeHour = true;
    }
  }
  else
  {
    m_lsb++;
  }

  if(ChangeHour)
  {
    if((h_msb == 1) && (h_lsb == 2)) // Transition 12 --> 01
    {
      h_msb = 0;
      h_lsb = 1;
    }
    else if((h_msb == 0) && (h_lsb == 9))  // Transition 09 --> 10
    {
      h_msb = 1;
      h_lsb = 0;
    }
    else
    {
      h_lsb++; // Example Transitioon 07 --> 08
    }
  }
  Current_Time[0] = h_msb;
  Current_Time[1] = h_lsb;
  Current_Time[2] = m_msb;
  Current_Time[3] = m_lsb;
}

void loop() 
{
  if(Expired_Seconds == 59u)
  {
    compute_time();  // Compute the time every minute.
    Expired_Seconds = 0;
  }
#if SERIAL_ENABLE
  Serial.print(Current_Time[0]);
  Serial.print(Current_Time[1]);
  Serial.print(":");
  Serial.print(Current_Time[2]);
  Serial.println(Current_Time[3]);
#endif

  #if SERIAL_ENABLE
    Serial.println("Starting New transmission...");
  #endif
  TransferData(C0H, AUTO_ADDR);
  delay(1000);
  Expired_Seconds++;
}
