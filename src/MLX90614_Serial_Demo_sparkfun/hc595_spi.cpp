#include "hc595_spi.h"
#include <TimerOne.h>

byte buf_seg_data[4] = {0};
byte seg_digit = 0;

void seg_disp_kur()
{

  pinMode(LATCH_DIO, OUTPUT);
  pinMode(CLK_DIO, OUTPUT);
  pinMode(DATA_DIO, OUTPUT);

  Timer1.initialize(1000); //uS
  Timer1.attachInterrupt(update_disp);
}

void update_disp()
{
  seg7_tara(seg_digit++);
  if (seg_digit > 3)
    seg_digit = 0;
}

void int_to_disp(int val)
{
  buf_seg_data[0] = SEGMENT_MAP[int_to_dijit(val, 0)];
  buf_seg_data[1] = SEGMENT_MAP[int_to_dijit(val, 1)];
  buf_seg_data[2] = SEGMENT_MAP[int_to_dijit(val, 2)];
  buf_seg_data[3] = SEGMENT_MAP[int_to_dijit(val, 3)];
}

void float_to_disp(double f_sayi)
{
  byte tam_kisim = 0; //int 16 bit PCD de
  byte ondalik_kisim = 0;

  double f_ondalik, f_tam;

  f_ondalik = modf(f_sayi, &f_tam); //result= modf (value, & integral)

  tam_kisim = (unsigned int)f_tam;
  ondalik_kisim = (unsigned int)(f_ondalik * 10.0);

  buf_seg_data[0] = SEGMENT_MAP[ondalik_kisim];                           //| 0x80;
  buf_seg_data[1] = SEGMENT_MAP[int_to_dijit(tam_kisim, 0)] & 0b01111111; //ortak anod
  buf_seg_data[2] = SEGMENT_MAP[int_to_dijit(tam_kisim, 1)];              // | 0x80;

  if(int_to_dijit(tam_kisim, 2)==0)
    buf_seg_data[3]=0xFF;
  else
    buf_seg_data[3] = SEGMENT_MAP[int_to_dijit(tam_kisim, 2)];              // | 0x80;
}

char int_to_dijit(unsigned int sayi, char basamak)
{
  char dig[5];
  // %10 10'a bölündüğünde kalan
  if (basamak == 0)
  {
    dig[basamak] = sayi % 10u;
    return dig[basamak];
  } //Birler
  if (basamak == 1)
  {
    dig[basamak] = (sayi / 10u) % 10u;
    return dig[basamak];
  } //Onlar
  if (basamak == 2)
  {
    dig[basamak] = (sayi / 100u) % 10u;
    return dig[basamak];
  } //yuzler
  if (basamak == 3)
  {
    dig[basamak] = (sayi / 1000u) % 10u;
    return dig[basamak];
  } //binler
  if (basamak == 4)
  {
    dig[basamak] = (sayi / 10000u) % 10u;
    return dig[basamak];
  } //onbinler
  return 0;
}

void seg7_tara(char digit)
{
  byte seg_veri = 0;

  switch (digit)
  {
  case 0:
    seg_veri = buf_seg_data[0];

    break;
  case 1:
    seg_veri = buf_seg_data[1];
    break;
  case 2:
    seg_veri = buf_seg_data[2];
    break;
  case 3:
    seg_veri = buf_seg_data[3];
    break;

  default:
    break;
  }

  WriteNumberToSegment(digit, seg_veri);
}

/* Write a decimal number between 0 and 9 to one of the 4 digits of the display */
void WriteNumberToSegment(byte Segment, byte Value)
{
  digitalWrite(LATCH_DIO, LOW);
  // devreye göre ilk 8bit segment bilgisi ikinci 8 bit 7seg display değeri.
  //       -  veri - -segment sec-
  // (MSB)(xxxx xxxx 0000c4c3c2c1)(LSB)
  shiftOut_595(DATA_DIO, CLK_DIO, MSBFIRST, Value);                   //MSB first olduğu iiçin önce veri gönderilir
  shiftOut_595(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[Segment]); //sonra segment bilgisi.

  digitalWrite(LATCH_DIO, HIGH);
}

void shiftOut_595(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    if (bitOrder == LSBFIRST)
    {
      digitalWrite(dataPin, val & 1);
      val >>= 1;
    }
    else
    {
      if ((val & 128) == 0)
        digitalWrite(dataPin, LOW); //128 -- > 0b1000 0000
      else
        digitalWrite(dataPin, HIGH); //128 -- > 0b1000 0000
      val <<= 1;
    }

    digitalWrite(clockPin, LOW);
    digitalWrite(clockPin, HIGH);
  }
}
