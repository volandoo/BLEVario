#ifdef VARIO_BLE

#include "BLEController.h"

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <stdlib.h>

class ServerCallbacks : public BLEServerCallbacks
{
public:
	ServerCallbacks(BLEController *controller)
	{
		this->controller = controller;
	}
	void onConnect(BLEServer *pServer)
	{
		Serial.println("connected");
		controller->deviceConnected = true;
		BLEDevice::stopAdvertising();
	};

	void onDisconnect(BLEServer *pServer)
	{
		Serial.println("disconnected");
		controller->deviceConnected = false;
		BLEDevice::startAdvertising();
	}

private:
	BLEController *controller;
};

class RXCharacteristicCallbacks : public BLECharacteristicCallbacks
{
public:
	RXCharacteristicCallbacks(BLEController *controller)
	{
		this->controller = controller;
	}
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string value = pCharacteristic->getValue();
		if (value.length() > 0)
		{
			Serial.println("*********");
			Serial.println("RX new value: " + String(value[0]));
			Serial.println("*********");
		}
	}

private:
	BLEController *controller;
};

class TXCharacteristicCallbacks : public BLECharacteristicCallbacks
{
public:
	TXCharacteristicCallbacks(BLEController *controller)
	{
		this->controller = controller;
	}
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string value = pCharacteristic->getValue();
		if (value.length() > 0)
		{
			Serial.println("*********");
			Serial.println("TX new value: " + String(value[0]));
			Serial.println("*********");
		}
	}

private:
	BLEController *controller;
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
public:
	MyCharacteristicCallbacks(BLEController *controller)
	{
		this->controller = controller;
	}
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string value = pCharacteristic->getValue();
		if (value.length() > 0)
		{
			Serial.println("*********");
			Serial.println("New value: " + String(value[0]));
			Serial.println("*********");
		}
	}

private:
	BLEController *controller;
};

void BLEController::start(String name)
{
	BLEDevice::init(std::string(name.c_str()));
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new ServerCallbacks(this));

	BLEService *pService = pServer->createService(UART_SERVICE);

	rxCharacteristic = pService->createCharacteristic(
		UART_CHARACTERISTIC_RX,
		BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
	rxCharacteristic->addDescriptor(new BLE2902());
	rxCharacteristic->setCallbacks(new RXCharacteristicCallbacks(this));

	txCharacteristic = pService->createCharacteristic(
		UART_CHARACTERISTIC_TX,
		BLECharacteristic::PROPERTY_NOTIFY);
	txCharacteristic->addDescriptor(new BLE2902());
	txCharacteristic->setCallbacks(new TXCharacteristicCallbacks(this));

	pService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(UART_SERVICE);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);

	BLEDevice::startAdvertising();
}

void BLEController::sendMessage(String &message)
{
	unsigned int checksum, ai, bi; // Calculating checksum for data string
	for (checksum = 0, ai = 0; ai < message.length(); ai++)
	{
		bi = (unsigned char)message[ai];
		checksum ^= bi;
	}
	message = '$' + message + '*' + String(checksum, HEX) + "\r\n";
	txCharacteristic->setValue(message.c_str());
	txCharacteristic->notify();
	delay(5);
}

#endif