/*
   Not: mlx90614 için ardu 3.3V besleme uygun degil...
*/
#include "hc595_spi.h"
#include "pin_mp3_tanimlar.h"

#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library
#include <Wire.h>             // I2C library, required for MLX90614
IRTherm therm; // Create an IRTherm object to interact with throughout

#include <DFPlayer_Mini_Mp3.h>
#include <SoftwareSerial.h>

const int votageInputPin = A2;
const float arduinoVoltage = 5.0; //Operating voltage of Arduino. either 3.3V or 5.0V
const int SISLEME_OUT_PIN= 13;

#define sislemeOn() digitalWrite(SISLEME_OUT_PIN, LOW);
#define sislemeOff() digitalWrite(SISLEME_OUT_PIN, HIGH);

SoftwareSerial dfSerial(DF_PLAYER_ARDU_RX_PIN,
                        DF_PLAYER_ARDU_TX_PIN); // RX, TX ARDU tarafın pinler

double mlx_deger = 0.0, ates_toplam = 0.0;

bool ates_soyle = false;
bool blink_led = false;
unsigned long time_df = 0L;
int ates_yuksek_olc_say = 0;
float randNumber = 0.0;

void setup() {

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(B_LED_PIN, OUTPUT);
  pinMode(SISLEME_OUT_PIN,OUTPUT);

  pinMode(UZ_SENS_PIN, INPUT);
  pinMode(DF_BUSSY_PIN, INPUT);

  seg_disp_kur();

  Serial.begin(9600); // Initialize Serial to log output
                      // Serial.println("Press any key to begin");
  // while (!Serial.available());

  therm.begin();         // Initialize thermal IR sensor
  therm.setUnit(TEMP_C); // Set the library's units to Farenheit

  therm.setEmissivity(0.98); // float newEmissivity = 0.98;
  Serial.println("Emissivity: " + String(therm.readEmissivity()));

  dfSerial.begin(9600);
  mp3_set_serial(dfSerial); // set softwareSerial for DFPlayer-mini mp3 module

  delay(100); // wait 1ms for mp3 module to set volume
  mp3_set_volume(25);
  delay(100); // wait 1ms for mp3 module to set volume
  mp3_stop();
  delay(100);
  mp3_play(MP3_ATES_OLC_SIS);
  delay(100);

  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
  float_to_disp(0.0);
  
  randomSeed(analogRead(0));
  sislemeOff();

}
void loop() {
  if (!digitalRead(UZ_SENS_PIN) && !ates_soyle) {
    int_to_disp(0);
    mp3_stop();
    delay(100);
    mp3_set_volume(30);
    delay(100);
    tone(BUZZER_PIN, 50, 100); // tone(BUZZER_PIN, 500, 300);Alarm
    mp3_play(MP3_OLCUM_YAPILIYOR);

    for (int i = 0; i < 3; i++) {
      if (therm.read()) // sensörün okuma işleminin doğruluğunu kontrol ediyor
      {
        // mlx_deger = (double)therm.object();
        // ates_toplam += mlx_deger;

        int_to_disp(i);
      } else {
        i--;
        if (i < 0)
          i = 0;
      }

      delay(500);
    }

    mlx_deger = 0.0;
    ates_toplam = 0.0;

    for (int i = 0; i < 5; i++) {
      if (therm.read()) // sensörün okuma işleminin doğruluğunu kontrol ediyor
      {
        mlx_deger = (double)therm.object();
        ates_toplam += mlx_deger;

        int_to_disp(i);
      } else {
        i--;
        if (i < 0)
          i = 0;
      }

      delay(200);
    }
    mlx_deger = ates_toplam / 5.0;

    //mlx_deger += 2.1; // 2.3; //2.3   //1.2 iyi
    mlx_deger=vcc_duzeltme(mlx_deger);
    mlx_deger=temp_kalibrasyon((double)mlx_deger);

    Serial.println(mlx_deger);
    float_to_disp(mlx_deger);

    mp3_stop();
    delay(100);

    if (mlx_deger > 34.0 && mlx_deger < 38.0) // 37.9
    {
      mp3_play(MP3_ATES_NORMAL);
      delay(100);
      ates_yuksek_olc_say = 0;
      ates_soyle = true;
    }

    if (mlx_deger >= 38.0 && mlx_deger <= 38.5) {
      // mp3_play(MP3_ATES_YUKSEK);
      mp3_play(MP3_A_YUK_TEKRARLA);
      delay(100);
      // digitalWrite(R_LED_PIN, HIGH);
      ates_yuksek_olc_say++;
      ates_soyle = true;
    }
    
    if (mlx_deger > 38.5) {
      // mp3_play(MP3_ATES_COK_YUKSEK);
      mp3_play(MP3_A_YUK_TEKRARLA);
      // digitalWrite(R_LED_PIN, HIGH);
      delay(100);
      ates_yuksek_olc_say++;
      ates_soyle = true;
    }

    if (ates_yuksek_olc_say >= 3) {
      ates_yuksek_olc_say = 0;
      mp3_play(MP3_ATES_COK_YUKSEK);
      delay(100);
    }

    time_df = millis();

    while (true) {
      if (!digitalRead(DF_BUSSY_PIN))
        break;
    }
    
    sislemeOn();
    delay(1000);

    noTone(BUZZER_PIN);
    // tone(BUZZER_PIN, 100, 100);

  } //
  else {
    digitalWrite(R_LED_PIN, LOW);
    digitalWrite(G_LED_PIN, LOW);
    ates_soyle = false;
    sislemeOff();
  }

  if (millis() - time_df > 1000) {
    if (therm.read()) // On success, read() will return 1, on fail 0.
    {
      mlx_deger = (double)therm.ambient();

      Serial.print("Ambient: " + String(therm.ambient(), 2));
      Serial.println("C");
    }

    float_to_disp(0.0);
    time_df = millis();
  }

  // delay(100);
}

double vcc_duzeltme(double temp)
{

    double ret_temp = 0.0;
    unsigned int adc_value = analogRead(votageInputPin);   // read the input
    double voltage = adc_value * (arduinoVoltage / 1023.0); //get the voltage from the value above
    ret_temp = temp - (voltage - 3.0) * 0.6;               // orj: obje_sic-(voltage -3.0) * 0.6;

    // Serial.print("VCC:");
    // Serial.println(voltage);
    // Serial.print("objeT_son:");
    // Serial.println(ret_temp);

    return ret_temp;
}
