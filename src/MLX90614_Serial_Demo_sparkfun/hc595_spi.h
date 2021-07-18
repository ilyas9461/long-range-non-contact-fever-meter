#ifndef hc595_spi_h_
    #define hc595_spi_h_

    #if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
    #else
    #include "WProgram.h"
    #endif

    #define LATCH_DIO   6
    #define CLK_DIO     5
    #define DATA_DIO    4

    /* Segment byte maps for numbers 0 to 9 */
    //const byte SEGMENT_MAP[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};//ortak katod
    
    const byte SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90}; //ortak anod
    /* Byte maps to select digit 1 to 4 */
    const byte SEGMENT_SELECT[] = {0x08, 0x04, 0x02, 0x01,0x00};

    /* değişkenler burada tanımlanınca hata verdi*/

    void seg_disp_kur();
    void update_disp();
    char int_to_dijit(unsigned int sayi, char basamak);

    void float_to_disp(double f_sayi);
    void int_to_disp(int val);

    void seg7_tara(char digit);
    void WriteNumberToSegment(byte Segment, byte Value);
    void shiftOut_595(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);

#endif
