### Volandoo BLE Vario

#### Notes:

A 500mAh battery will give you about 7 hours of flight time when fully charged. I recommend using a 1000mAh battery instead

The vario will try to connect to another device but shut down automatically if it can't connect within 6 seconds. Once connected, it will shut down when disconnected.

Once shut down it will wake up again when the button is pressed. The battery will last a long time when the vario is shut down. But just in case, charge it before going flying.

#### Requirements:

- Lolin32 Lite (ESP32 Chip with battery charger)
  ![Lolin32 Lite](/images/lolin32.jpg)

- MS5611 Barometer
  ![MS5611](/images/ms5611.jpg)

- Lipo Battery
  ![Battery](/images/battery.jpg)

- Button
  ![Button](/images/button.jpg)

#### Instructions:

```
Wire connections:

MS5611 ... Lolin32:
   GND  -  GNG
   VIN  -  3V
    19  -  SDA
    23  -  SCL

Button to Lolin32:
  16 and 33

Battery: plug it in.
```

#### Acknowledgements:

All the code here is written by me with the exception of the MS5611 library. The altimeter algorithm was inspired by the XCSoar and BlueFly open source projects.

[MIT License](/LICENSE.md)
