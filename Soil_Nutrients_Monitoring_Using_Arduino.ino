#include <OneWire.h>
#include <SoftwareSerial.h>
#include <DallasTemperature.h>

/*
-: Temperature Sensor Connction :-
`````````````````````````````````````
  Arduino       Temperature Sensor
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    A0             Data
    GND            GND
    VCC            VCC

    // Add one 4.2K pullup resistor with Data; else output vaule will be -127
*/
#define TEMPARETURE_SENSOR A0


/*
-: Moisture Sensor Connction :-
`````````````````````````````````````
  Arduino       Moisture Sensor
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    A1             Data
    GND            GND
    VCC            VCC
*/
#define MOISTURE_SENSOR    A1


/*
-: NPK Sensor Connction :-
`````````````````````````````
  Arduino       Modbus
  ~~~~~~~~~~~~~~~~~~~~~
    2             RO  // Receiver Output
    3             DI  // Driver Input
    4             DE  // Driver Enable
    4             RE  // Receiver Enable

    // Both Enable pins are shorted and conected to pin 4 of arduino
*/
#define MOD_BUS_RO         2
#define MOD_BUS_DI         3
#define MOD_BUS_ENABLE     4



#define MOD_BUS_DATA_SIZE  11


byte npkValues[MOD_BUS_DATA_SIZE] = { 0 };
byte npkByteCode[]                = { 0x01, 0x03, 0x00, 0x1E, 0x00, 0x03, 0x65, 0xCD };
// Index        :  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21
// Dummy data 1 :  1   3   6   0   0   0   0   0   0  33 117   1   3   6   0   0   0   0   0   0  33 117
// Dummy data 2 :  1   3   6   0   1   0   1   0   3  13 116   1   3   6   0   1   0   1   0   3  13 116

OneWire           oneWire(TEMPARETURE_SENSOR);
DallasTemperature temparetureSensor(&oneWire);
SoftwareSerial    modBus(MOD_BUS_RO, MOD_BUS_DI);



void readNpkSensorData()
{
  // Make ENABLE pin High to setup as Master to write NPK bytecode to sensor
  digitalWrite(MOD_BUS_ENABLE, HIGH);
  modBus.write(npkByteCode, sizeof(npkByteCode));


  // Make ENABLE pin Low to setup as Slave to read from NPK sensor
  digitalWrite(MOD_BUS_ENABLE, LOW);
  while (modBus.available())  // While have data at modbus port this loop executes
  {
    for (int index = 0; index < MOD_BUS_DATA_SIZE; index++)
    {
      npkValues[index] = modBus.read();  // Receive INTEGER value from NPK sensor throught RS-485
      delay(10);
    }
  }
}


int readMoistureSensorData()
{
  int soilMoistureValue   = 0;
  int soilMoisturePercent = 0;

  // Read Moisture from sensor
  soilMoistureValue   = analogRead(MOISTURE_SENSOR);
  // Mapping the moisture value (0-1024) to percent (0-100) value
  soilMoisturePercent = map(soilMoistureValue, 0, 1023, 0, 100);

  return soilMoisturePercent;
}


int readTemperatureSensor()
{
  int tempareture = 0;

  // Read temperature sensor data
  temparetureSensor.requestTemperatures();
  tempareture = temparetureSensor.getTempCByIndex(0);

  return tempareture;
}


void setup()
{
  Serial.begin(9600);         // initialize serial at baudrate 9600
  temparetureSensor.begin();  // Start up the Tempareture Sensor library
  modBus.begin(9600);         // Setup the modbus as 9600 baudrate
  pinMode(MOD_BUS_ENABLE,  OUTPUT);
  pinMode(MOISTURE_SENSOR, INPUT);
  delay(10);
  Serial.println("\n~~~~~: Soil Nutrients Monitoring Using Arduino :~~~~~");
}


void loop()
{
  int temperature = 0;
  int moisture    = 0;
  int nitrogen    = 0;
  int phosphorous = 0;
  int potassium   = 0;

  // Reading all the sensor data
  temperature = readTemperatureSensor();
  moisture    = readMoistureSensorData();

  readNpkSensorData();
  nitrogen    = (npkValues[3] << 8) + npkValues[4];
  phosphorous = (npkValues[5] << 8) + npkValues[6];
  potassium   = (npkValues[7] << 8) + npkValues[8];

  // Print All the collcted data
  Serial.println("");
  Serial.println("######### Soil Nutrients Data From Sample Soil #########");
  Serial.println("````````````````````````````````````````````````````````");

  // Moisture Data
  Serial.print("        Moisture    : ");
  Serial.print(moisture);
  Serial.println(" %");

  // Temperature Data
  Serial.print("        Temperature : ");
  Serial.print(temperature);
  Serial.println(" C");

  // NPK Data
  Serial.print("        Nitrogen    : ");
  Serial.print(nitrogen, DEC);
  Serial.println(" mg/kg");
  Serial.print("        Phosphorous : ");
  Serial.print(phosphorous, DEC);
  Serial.println(" mg/kg");
  Serial.print("        Potassium   : ");
  Serial.print(potassium, DEC);
  Serial.println(" mg/kg");
  Serial.println("########################################################");
  delay(1000);
}
