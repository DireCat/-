
#include "PlantController.h"
#include <LiquidCrystal.h>
#include <Streaming.h>
#include "Config.h"

/* Настройки см. вкладку Config.h */

void setup() 
{
  /* Распиновка входа датчиков */
  const uint8_t pinout_sensor_in [] = {A0, A1, A2, A3, A4, A5, A6, A7, A8};
  
  /* Распиновка питания + датчиков */
  const uint8_t pinout_sensor_enable [] = {13, 13, 13, 13, 13, 13, 13, 13, 13};

  /* Распиновка реле */
  const uint8_t pinout_relay [] = {3, 5, 6, 7, 8, 9, 10, 11, 12};

  Serial.begin(115200);
  Serial << "Загружено.\r\n";
  PlantController * pc = new PlantController (pinout_sensor_in, pinout_sensor_enable, pinout_relay, PLANT_COUNT);

  // TODO: прикрутить дисплей
  /*
  LiquidCrystal lcd (A9, A10, A11, A12, A13, A14);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("hello!");
  */

  for(;;)
  {
    pc -> loop();
  }
  
}

void loop() {}
