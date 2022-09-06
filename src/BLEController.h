#ifndef BLE_CONTROLLER_
#define BLE_CONTROLLER_

#include <Arduino.h>

#define UART_SERVICE "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class BLEServer;
class BLECharacteristic;

class BLEController
{
public:
	BLEController(){};
	void start(String name);
	void sendMessage(String &message);

	bool rxDeviceConnected = false;
	bool deviceConnected = false;

private:
	BLEServer *pServer = NULL;
	BLECharacteristic *rxCharacteristic = NULL;
	BLECharacteristic *txCharacteristic = NULL;
};
#endif
