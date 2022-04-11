#include <Sodaq_RN2483.h>
#include <CayenneLPP.h>

#define debugSerial SerialUSB
#define loraSerial Serial2

#define ADC_AREF 3.3f
//#define BATVOLT_R1 2.0f // One v1
//#define BATVOLT_R2 2.0f // One v1
#define BATVOLT_R1 4.7f // One v2
#define BATVOLT_R2 10.0f // One v2
#define BATVOLT_PIN BAT_VOLT

bool OTAA = false;
CayenneLPP lpp(51); // the *maximum* payload size

// ABP
// USE YOUR OWN KEYS!
const uint8_t devAddr[4] =
{
    0x26, 0x0E, 0x9C, 0x3C
};

// USE YOUR OWN KEYS!
const uint8_t appSKey[16] =
{
   0x4D, 0x28, 0x72, 0xAC, 0xC6, 0x70, 0x2E, 0x8B, 0x78, 0xA1, 0x46, 0x3F, 0x8D, 0xE7, 0xBE, 0xA6
};

// USE YOUR OWN KEYS!
const uint8_t nwkSKey[16] =
{
   0x20, 0x1F, 0xFB, 0xEE, 0xE7, 0x27, 0x11, 0xE5, 0x33, 0xBE, 0x1A, 0x62, 0xAD, 0x97, 0xAA, 0x64
};

// OTAA
const uint8_t DevEUI[8] =
{
  0x00, 0x04, 0xA3, 0x0B, 0x00, 0xE8, 0xAB, 0x04
};

const uint8_t AppEUI[16] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t AppKey[16] =
{
  0x9E, 0x71, 0x9B, 0x5F, 0x51, 0x8B, 0x0E, 0x2E, 0xDB, 0x4D, 0xA8, 0x5D, 0x49, 0x08, 0x2C, 0xB3
};

void setup()
{
  while ((!debugSerial) && (millis() < 10000)){
    // Wait 10 seconds for debugSerial to open
  }

  debugSerial.begin(57600);
  loraSerial.begin(LoRaBee.getDefaultBaudRate());

  setupLoRa();
}

void setupLoRa(){
  if(!OTAA){
    // ABP
    setupLoRaABP();
  } else {
    //OTAA
    setupLoRaOTAA();
  }
}

void setupLoRaABP(){  
  if (LoRaBee.initABP(loraSerial, devAddr, appSKey, nwkSKey, false))
  {
    debugSerial.println("Communication to LoRaBEE successful.");
  }
  else
  {
    debugSerial.println("Communication to LoRaBEE failed!");
  }
}

void setupLoRaOTAA(){

  debugSerial.println(LoRaBee.initOTA(loraSerial, DevEUI, AppEUI, AppKey, false));

  if (LoRaBee.initOTA(loraSerial, DevEUI, AppEUI, AppKey, false))
  {
    debugSerial.println("Network connection successful.");
  }
  else
  {
    debugSerial.println("Network connection failed!");
  }
}

void loop()
{
  // Cayenne LPP example code from https://github.com/TheThingsNetwork/arduino-device-lib/blob/master/examples/CayenneLPP/CayenneLPP.ino
  // Cayenne LPP documentation from https://mydevices.com/cayenne/docs/lora/#lora-cayenne-low-power-payload
  
  lpp.reset();
  lpp.addTemperature(1, getTemperature() );  

    switch (LoRaBee.send(1, (uint8_t*)lpp.getBuffer(), lpp.getSize()))
    {
    case NoError:
      debugSerial.println("Successful transmission.");
      break;
    case NoResponse:
      debugSerial.println("There was no response from the device.");
      break;
    case Timeout:
      debugSerial.println("Connection timed-out. Check your serial connection to the device! Sleeping for 20sec.");
      delay(20000);
      break;
    case PayloadSizeError:
      debugSerial.println("The size of the payload is greater than allowed. Transmission failed!");
      break;
    case InternalError:
      debugSerial.println("Oh No! This shouldn't happen. Something is really wrong! The program will reset the RN module.");
      setupLoRa();
      break;
    case Busy:
      debugSerial.println("The device is busy. Sleeping for 10 extra seconds.");
      delay(10000);
      break;
    case NetworkFatalError:
      debugSerial.println("There is a non-recoverable error with the network connection. The program will reset the RN module.");
      setupLoRa();
      break;
    case NotConnected:
      debugSerial.println("The device is not connected to the network. The program will reset the RN module.");
      setupLoRa();
      break;
    case NoAcknowledgment:
      debugSerial.println("There was no acknowledgment sent back!");
      break;
    default:
      break;
    }
    // Delay between readings
    // 60 000 = 1 minute
    delay(60000); 
}

float getTemperature()
{
  //10mV per C, 0C is 500mV
  float mVolts = (float)analogRead(TEMP_SENSOR) * 3300.0 / 1023.0;
  float temp = (mVolts - 500.0) / 10.0;
  return (temp);
}
