#include <Arduino.h>

#include <MS5611.h>
#include <Altimeter.h>
#include "BLEController.h"
#include <driver/rtc_io.h>

MS5611 ms5611;
Altimeter altimeter;
BLEController bleController;

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void setup()
{
  setCpuFrequencyMhz(80);
  pinMode(GPIO_NUM_16, OUTPUT);
  digitalWrite(GPIO_NUM_16, HIGH);

  rtc_gpio_pulldown_en(GPIO_NUM_33);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, RISING);

  Serial.begin(115200);

  delay(1000);
  print_wakeup_reason();
  Serial.println("Starting Device...");

  ms5611.begin(19, 23);
  bleController.start("Volandoo");
  Serial.println("Starting BLE work!");
}

static double prevTime = 0;
static int cycles = 0;
void loop()
{
  if (bleController.deviceConnected)
  {
    auto now = millis();
    auto time = 1 / (now - prevTime);
    if (prevTime == 0)
    {
      time = 0;
    }
    prevTime = now;

    auto pressure = ms5611.getPressure();
    altimeter.addPressure(pressure, time);

    auto varioVal = altimeter.varioValue();
    if (isnan(varioVal))
    {
      ESP.restart();
      return;
    }
    auto start = String("LK8EX1");
    auto pres = String(pressure);
    auto alt = String(altimeter.altitude());
    auto vario = String(int(varioVal * 100));
    auto temp = String(double(ms5611.getTemperature()) / 100.0, 1);
    auto battery = String("999");
    // auto volt = analogRead(35) / 4096.0 * 7.445;
    // auto battery = String(volt);

    bleController.sendMessage(start + "," + pres + "," + alt + "," + vario + "," + temp + "," + battery);
    return;
  }
  prevTime = 0;
  cycles++;
  if (cycles > 3)
  {
    digitalWrite(GPIO_NUM_16, LOW);
    esp_deep_sleep_start();
  }
  delay(2000);
}
