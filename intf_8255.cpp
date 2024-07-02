/*
   2    D2    PD2   INT0
   3    D3    PD3   INT1

   8255
   Port A D0-7 <--> PC0-3,PD4-7

   8255 to Redboard
   PC7  /OBF   output to 2 (INT0), falling edge
   PC6  /ACK   input from PC5
   read data

   Redboard to 8255
   write data
   PC4  /STB   input from PC4
   PC5  IBF    output to 3 (INT1), falling edge
*/
#define CIRCULAR_BUFFER_INT_SAFE

#include "intf_8255.h"

const byte OBF_int = 2;
const byte IBF_int = 3;
const byte ACK_out = A5;   //PC5 A5/D19
const byte STB_out = A4;   //PC4 A4/D18

#ifdef USE_CODE_INTERRUPTS
CircularBuffer<byte, 256> Tx_8255;
CircularBuffer<byte, 256> Rx_8255;

volatile bool Wrote1stByte;
#endif

bool In8255Avail()
{
  return digitalRead(OBF_int) == LOW;
}

void WriteData(byte value)
{
  // Setup bus as output
  //DDRD = DDRD | 0xF0;   // set bits 7-4
  DDRD = 0xF2;
  
  //DDRC = DDRC | 0x0F;   // set bits 3-0
  DDRC = 0x3F;

  PORTC = (PORTC & 0x30) | (value & 0x0F);
  PORTD = (PORTD & 0x02) | (value & 0xF0);
  
  //PC4  /STB   input from PC4
  digitalWrite(STB_out, LOW);
  digitalWrite(STB_out, HIGH);

  // Setup bus as input
  //DDRD = DDRD & 0x0F;   // clear bits 7-4
  DDRD = 0x02;
  
  //DDRC = DDRD & 0xF0;   // clear bits 3-0
  DDRC = 0x30;
}

byte ReadData()
{
  byte value;
  
  //PC6  /ACK   input from PC5
  //read data
  digitalWrite(ACK_out, LOW);
  
  // Setup bus as input
  //DDRD = DDRD & 0x0F;   // clear bits 7-4
  DDRD = 0x02;
  
  //DDRC = DDRD & 0xF0;   // clear bits 3-0
  DDRC = 0x30;
  
  value = (PINC & 0x0F) | (PIND & 0xF0);
  
  digitalWrite(ACK_out, HIGH);

  return value;
}

#ifdef USE_CODE_INTERRUPTS
//8255 to Redboard
void OBF_Isr(void) {
byte value;

  value = ReadData();

  //put in circular buffer  
  if (Rx_8255.isFull()) {
    // volatile RxState = full
  }
  else
  {
    Rx_8255.push(value);
  }
}

//Redboard to 8255
void IBF_Isr(void) {
byte value;

  //get from circular buffer
  if (Tx_8255.isEmpty())
  {
    Wrote1stByte = false;
    return;
  }

  //write data (if not empty)
  value = Tx_8255.pop();
  
  WriteData(value);  
}
#endif

void intf_8255_init() {  
  // Setup bus as input
  //DDRD = DDRD & 0x0F;   // clear bits 7-4
  DDRD = 0x02;
  
  //DDRC = DDRD & 0xF0;   // clear bits 3-0
  DDRC = 0x30;

  pinMode(OBF_int, INPUT_PULLUP);
  pinMode(IBF_int, INPUT_PULLUP);
  pinMode(ACK_out, OUTPUT);
  pinMode(STB_out, OUTPUT);
  digitalWrite(ACK_out, HIGH);
  digitalWrite(STB_out, HIGH);

#ifdef USE_CODE_INTERRUPTS
  attachInterrupt(digitalPinToInterrupt(OBF_int), OBF_Isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(IBF_int), IBF_Isr, FALLING);

  Wrote1stByte = false;
#endif
}

#ifdef USE_CODE_INTERRUPTS
// blocking read
byte ReadFrom8255()
{
  while (Rx_8255.isEmpty()) /* wait */;

  return Rx_8255.pop();
}

void WriteTo8255(byte value)
{
  while (Tx_8255.isFull()) /* wait */;

  /* if empty send 1st byte directly, else push following bytes */
  if (Wrote1stByte)
  {
    Tx_8255.push(value);
  }
  else // 1st byte after buffer is emptied
  {
    WriteData(value);
    Wrote1stByte = true;
  }
}
#else // No interrupts
byte ReadFrom8255()
{
  while (digitalRead(OBF_int) == HIGH) /* wait */;

  return ReadData();
}

void WriteTo8255(byte value)
{
  while (digitalRead(IBF_int) == HIGH) /* wait */;

  WriteData(value);
}
#endif
