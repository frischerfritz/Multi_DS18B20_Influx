// Speichert mehrere DS18B20 in InfluxDB2
// 
// Der Sketch ist noch in einem "Work in Progress" - Zustand und gehört gründlich entrümpelt.
/* 
Wiring mit meinen geloeteten Temperatursensoren: 
-----------------------------------------------
gelb=GND
rot= Daten -> Pin D2 oder D4
schwarz=5V
 */
// Schaltung auf dem Breadboard: Siehe Grafik Fritzing-1-Wire-Temperatursensor-Projekt_Steckplatine.png
//https://forum-raspberrypi.de/forum/thread/38060-mehrere-sensoren-an-1-wire-machen-probleme/

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2 // Sensor DS18B20 am digitalen Pin 2
#define DS18B20_Aufloesung 12
OneWire oneWire(ONE_WIRE_BUS); //

//Übergabe der OnewWire Referenz zum kommunizieren mit dem Sensor.
DallasTemperature sensors(&oneWire);
int sensorCount;
// arrays to hold device address
DeviceAddress sensor1, sensor2, sensor3, sensor4, sensor5, sensor6, sensor7;
DeviceAddress sensorAddresses;

String sensorstring1, sensorstring2, sensorstring3, sensorstring4, sensorstring5, sensorstring6, sensorstring7;

// Network Settings
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xF0};
IPAddress influxIP(192, 168, 178, 47);

// the port that the InfluxDB UDP plugin is listening on
unsigned int influxPort = 8086;
String line;
EthernetClient client;
// function to print a device address
String printSensorAddress(DeviceAddress deviceAddress)
{
  String oneWireAdress;    // Variable für den ID-String
  String oneWireAdressHex; // Variable für den ID-String
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16)
      Serial.print("0");
    if (deviceAddress[i] < 16)
      oneWireAdress += "0";
    Serial.print(deviceAddress[i], HEX);
    oneWireAdress += deviceAddress[i];
  }
  return oneWireAdress;
}

String SensorAddressString(int index)
{
  DeviceAddress sensoraddress;
  sensors.getAddress(sensoraddress, index);
  String oneWireAdress;    // Variable für den ID-String
  String oneWireAdressHex; // Variable für den ID-String
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    // if (sensoraddress[i] < 16) Serial.print("0");
    if (sensoraddress[i] < 16)
      oneWireAdress += "0";
    // Serial.print(sensoraddress[i], HEX);
    oneWireAdress += sensoraddress[i];
  }
  oneWireAdressHex = String(oneWireAdress.toInt(), HEX);
  return oneWireAdressHex;
}

void setup(void)
{
  Serial.begin(9600); // Starten der seriellen Kommunikation mit 9600 baud
  Serial.println("Temperatursensor - DS18B20");
  sensors.begin(); // Starten der Kommunikation mit dem Sensor

  sensorCount = sensors.getDeviceCount();             // Lesen der Anzahl der angeschlossenen Temperatursensoren.
  for (byte i = 0; i < sensors.getDeviceCount(); i++) // die selbe Resolution auf alle Sensoren speichern.
  {
    if (sensors.getAddress(sensorAddresses, i))
    {
      sensors.setResolution(sensorAddresses, DS18B20_Aufloesung);
    }
  }

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // get DHCP
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
  }
  Serial.println(Ethernet.localIP());

  if (!sensors.getAddress(sensor1, 0))
    Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(sensor2, 1))
    Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(sensor3, 2))
    Serial.println("Unable to find address for Device 2");
  if (!sensors.getAddress(sensor4, 3))
    Serial.println("Unable to find address for Device 3");
  if (!sensors.getAddress(sensor5, 4))
    Serial.println("Unable to find address for Device 3");
  if (!sensors.getAddress(sensor6, 5))
    Serial.println("Unable to find address for Device 3");
  if (!sensors.getAddress(sensor7, 6))
    Serial.println("Unable to find address for Device 3");
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  sensorstring1 = printSensorAddress(sensor1);
  Serial.println();
  Serial.print("Device 1 Address: ");
  sensorstring2 = printSensorAddress(sensor2);
  Serial.println();
  Serial.print("Device 2 Address: ");
  sensorstring3 = printSensorAddress(sensor3);
  Serial.println();
  Serial.print("Device 3 Address: ");
  sensorstring4 = printSensorAddress(sensor4);
  Serial.println();
  Serial.print("Device 4 Address: ");
  sensorstring5 = printSensorAddress(sensor5);
  Serial.println();
  Serial.print("Device 5 Address: ");
  sensorstring6 = printSensorAddress(sensor6);
  Serial.println();
  Serial.print("Device 6 Address: ");
  sensorstring7 = printSensorAddress(sensor7);
  Serial.println();
}

void loop(void)
{
  if (sensorCount == 0)
  {
    Serial.println("Es wurde kein Temperatursensor gefunden!");
    Serial.println("Bitte überprüfe deine Schaltung!");
  }
  // Es können mehr als 1 Temperatursensor am Datenbus angeschlossen werden.
  // Anfordern der Temperaturwerte aller angeschlossenen Temperatursensoren.
  sensors.requestTemperatures();

  // Ausgabe aller Werte der angeschlossenen Temperatursensoren.
  for (int i = 0; i < sensorCount; i++)
  {
    // for debugging:
    // Serial.print(i);
    // Serial.print(". Temperatur: ");
    // Serial.print(sensors.getTempCByIndex(i));
    // Serial.print("\xC2\xB0"); // shows degree symbol
    // Serial.println("C");
    // sensors.getAddress(sensorAddresses, i);

    client.stop();
    if (client.connect(influxIP, influxPort))
    {
      Serial.println("connected");
      client.println(F("POST /api/v2/write?org=bergstr&bucket=temperatures&precision=s HTTP/1.1"));
      client.println(F("Host: 192.168.178.47:8086"));
      client.println(F("Accept: */*"));
      client.println(F("Authorization: Token 96G-hNvM8P3xkazi198DGoIterBIuuippZC4mWnRd_FGR-DbvKi76hc-Ol1MGO_6djbWQu28IZVTOUtuzf0N1w=="));
      client.println(F("User-Agent: arduino-ethernet-puffer"));
      client.println(F("Connection: close"));
      client.print("Content-Length: ");
      client.println(String("temp,sensorid=" + SensorAddressString(i) + " temp=" + String(sensors.getTempCByIndex(i))).length());
      client.println(F("Content-Type: application/x-www-form-urlencoded"));
      // for debugging:
      // Serial.println(line.length());
      client.println();
      client.println(String("temp,sensorid=" + SensorAddressString(i) + " temp=" + String(sensors.getTempCByIndex(i))));
      // for debugging:
      //Serial.println(String("temp,sensorid=" + SensorAddressString(i) + " temperature=" + String(sensors.getTempCByIndex(i))).length());
      //Serial.println(String("temp,sensorid=" + SensorAddressString(i) + " temperature=" + String(sensors.getTempCByIndex(i))));

    }
    else
    {
      // kf you didn't get a connection to the server:
      Serial.println("connection failed");
    }
  }

  delay(15 * 60000); // Pause von minute * 60000
}
