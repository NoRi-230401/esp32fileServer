#include <Arduino.h>
#include "esp32fileServer.h"

void setup(void)
{
  FLSV_setup();
}

void loop()
{
  FLSV_loop();
  vTaskDelay(1);
}

