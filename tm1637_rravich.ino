#include "tm1637_rravich.h"
//- Global Variables ------------------------------------------------------------------------------
uint8 SCLK = 0u;  // The actual pin value will be assigned during run time.
uint8 SDAT = 0u;

volatile ErrorStatusType ErrSts = ALL_OK;

//- These are the starting time of 3 different countries.
uint8 CurrentCanadaTime[4]  = {0,5,3,0};
uint8 CurrentGermanTime[4]  = {1,1,3,0};
uint8 CurrentIndiaTime[4]   = {0,4,0,0};

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

  // while entering this function, status of SDAT = HIGH ; SCLK = HIGH
  if( digitalRead(SCLK) != HIGH ||   \
      digitalRead(SDAT) != HIGH)
    {
      #if SERIAL_ENABLE
      Serial.println("ERROR: Cannot start communication due to incorrect level in SDAT, SCLK");
      #endif
      ErrSts = ERROR_DETECTED;
      return;
    }
    FALLING_EDGE(SDAT);  // Bring SDAT level LOW to "Start" communication.
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
  
  if( digitalRead(SCLK) != LOW || \
      digitalRead(SDAT) != LOW)
    {
      #if SERIAL_ENABLE
      Serial.println("ERROR: cannot stop communication because of SDAT, SCLK level.");
      #endif
      ErrSts = ERROR_DETECTED;
      return;
    }

  RISING_EDGE(SCLK);
  bitdelay(1u);
  
  RISING_EDGE(SDAT); // communication stopped
  bitdelay(1u);
}

void SendTo1637(uint8 TxData, PayloadType payload)
{
  uint8 SegmentedData = 0u;
  
  /* - If this function is called after a "Start", SCLK=HIGH and SDAT=LOW
   *  - If this function is called after a "SendTo1637, SCLK=LOW and SDAT=LOW */
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
      SegmentedData = SegmentedData | (DotBit << 7);
    }
    TxData = SegmentedData;
  }

  for(uint8 idx = 0; idx < 8; idx++)
  {
    FALLING_EDGE(SCLK);
    if(bitRead(TxData, idx))
    {
      // val = 1
      RISING_EDGE(SDAT);
    }
    else
    {
      FALLING_EDGE(SDAT);
    }
    RISING_EDGE(SCLK);
  } // 8 rising edges of SCLK will be over after the for loop

  FALLING_EDGE(SCLK);    // Falling edge of 8th clock
  pinMode(SDAT, OUTPUT); // Prepare for the ACK

  RISING_EDGE(SCLK);  // 9th rising edge
  
  if(digitalRead(SDAT) != LOW)
  {
    #if SERIAL_ENABLE
    Serial.println("ERROR: during ack.");
    #endif

    ErrSts = ERROR_DETECTED;
    return;
  }

  FALLING_EDGE(SCLK); // 9th clock is finished
  // During the exit of this function SDAT, SCLK are LOW
}

//- Global function defitions ---------------------------------------------------------------------
void TransferData(uint8 Addr, AddressingType AddrTyp, uint8* TimeInfo)
{
  // while entering this function, SCLK=HIGH, SDAT=LOW

  uint8 DataCmdSet    = (SET_DATA_CMD | (AddrTyp << 2u));
  uint8 ResolvedAddr  = (SET_ADDR_CMD | Addr);

  if(ErrSts != ALL_OK)
  {
    #if SERIAL_ENABLE
    Serial.println("ERROR: Cannot initiate data transfer.");
    #endif
    return;
  }
  
  if( digitalRead(SCLK) != HIGH || \
      digitalRead(SDAT) != HIGH)
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
      SendTo1637(TimeInfo[idx], DATA);
      Data_Cntr++;
      bitdelay(1);
  }
  StopComm();   //- ---------------------------------------

  #if SERIAL_ENABLE
  Serial.println("Transmit Brightness...");
  #endif

  StartComm();  //- BLOCK 3--------------------------------
  SendTo1637(SET_DISP_CMD | DISPLAY_ON_MASK | BRIGHTNESS_LVL_LOW, CMD);
  StopComm();   //- ---------------------------------------

  #if SERIAL_ENABLE
  if (ErrSts != ERROR_DETECTED)
    Serial.println("Brightness successfully transmitted.");
  #endif
}

/*- -----------------------------------------------------------------------------------------------
* Brief: Function to print to "Console" the computed time of different countries
*------------------------------------------------------------------------------------------------*/
void Print_CurrentTimes(void)
{
  uint8* CurrTimList[3] = {CurrentCanadaTime, CurrentGermanTime, CurrentIndiaTime};

  Serial.println("Time is displayed in the order: Canada, Germany, India:");
  for(uint8 idx=0; idx<3; idx++)
  {
    Serial.print(CurrTimList[idx][0]);
    Serial.print(CurrTimList[idx][1]);
    Serial.print(":");
    Serial.print(CurrTimList[idx][2]);
    Serial.println(CurrTimList[idx][3]);
  }
}

void compute_time(uint8* Curr_Time)
{
  boolean ChangeHour = false;
  uint8 m_lsb = Curr_Time[3];
  uint8 m_msb = Curr_Time[2];

  uint8 h_lsb = Curr_Time[1];
  uint8 h_msb = Curr_Time[0];

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
  Curr_Time[0] = h_msb;
  Curr_Time[1] = h_lsb;
  Curr_Time[2] = m_msb;
  Curr_Time[3] = m_lsb;
}

//- Setup -----------------------------------------------------------------------------------------
void setup() {

#if SERIAL_ENABLE
  Serial.begin(9600u);
#endif
  
  pinMode(CAN_SCLK, INPUT);
  pinMode(CAN_SDAT, INPUT);
  
  pinMode(GER_SCLK, INPUT);
  pinMode(GER_SDAT, INPUT);
  
  pinMode(IND_SCLK, INPUT);
  pinMode(IND_SDAT, INPUT);
}

//- LOOP function ---------------------------------------------------------------------------------
void loop() 
{
  if(Expired_Seconds == 59u)
  {
    compute_time(CurrentCanadaTime);  // Compute the time every minute.
    compute_time(CurrentGermanTime);
    compute_time(CurrentIndiaTime);
    Expired_Seconds = 0;
  }

#if SERIAL_ENABLE
  Print_CurrentTimes();
#endif

  #if SERIAL_ENABLE
    Serial.println("Starting New transmission...");
  #endif
  DotBit = 1; //!DotBit; use this code to blink the middle dot
  //- Transfer CANADA time --------------------------------
  SCLK = CAN_SCLK;
  SDAT = CAN_SDAT;
  TransferData(C0H, AUTO_ADDR, CurrentIndiaTime);
  delay(4);
  //- Transfer GERMAN time --------------------------------
  SCLK = GER_SCLK;
  SDAT = GER_SDAT;
  TransferData(C0H, AUTO_ADDR, CurrentCanadaTime);
  delay(4);
  //- Transfer INDIAN time --------------------------------
  SCLK = IND_SCLK;
  SDAT = IND_SDAT;
  TransferData(C0H, AUTO_ADDR, CurrentGermanTime);

  delay(1000);  // approximately 434.784 ms to compute the loop function
  Expired_Seconds++;

}
