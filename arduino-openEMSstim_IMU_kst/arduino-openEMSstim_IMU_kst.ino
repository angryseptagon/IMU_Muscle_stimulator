/**
   arduino-openEMSstim: https://github.com/PedroLopes/openEMSstim
   a mod of the original [1] by Pedro Lopes, see
   [1] <https://bitbucket.org/MaxPfeiffer/letyourbodymove/wiki/Home/License>
   @license "The MIT License (MIT) – military use of this product is forbidden – V 0.2"
*/

// Necessary files (AltSoftSerial.h, AD5252.h, Rn4020BTLe.h, EMSSystem.h, EMSChannel.h) and dependencies (Wire.h, Arduino.h)
//#include "Arduino_Software.h"
#include "Arduino.h"
#include "AltSoftSerial.h"
#include "Wire.h"
#include "AD5252.h"
#include "Rn4020BTLe.h"
#include "EMSSystem.h"
#include "EMSChannel.h"
#include "avr/pgmspace.h"

//BT: the string below is how your EMS module will show up for other BLE devices
#define EMS_BLUETOOTH_ID "TEST1"

//DEBUG: setup for verbose mode (prints debug messages if DEBUG_ON is 1)
#define DEBUG_ON 1

//USB: allows commands using the full protocol (refer to https://github.com/PedroLopes/openEMSstim) (by default this is active)
#define USB_FULL_COMMANDS_ACTIVE 1

//USB: allows to send simplified test commands (one char each, refer to https://github.com/PedroLopes/openEMSstim) to the board via USB (by default this is inactive)
#define USB_TEST_COMMANDS_ACTIVE 0

//helper print function that handles the DEBUG_ON flag automatically
void printer(String msg, boolean force = false) {
  if (DEBUG_ON || force) {
    Serial.println(msg);
  }
}

//Initialization of control objects
AltSoftSerial softSerial;
AD5252 digitalPot(0);
Rn4020BTLe bluetoothModule(2, &softSerial);
EMSChannel emsChannel1(5, 4, A2, &digitalPot, 1);
EMSChannel emsChannel2(6, 7, A3, &digitalPot, 3);
EMSSystem emsSystem(2);

//begin IMU code

#define MAX_PACKET_LEN          (128)
uint8_t Dataout[20];
uint8_t ID;
int16_t AccRaw[3];
int16_t GyoRaw[3];
int16_t MagRaw[3];
float Eular[3];
float AV[3];   /* Euler Angle */
float AA[3]; /*aacceleration*/
int32_t Pressure;
int intensity;
static uint8_t status = 0; /* state machine */
/* define a packet struct */
typedef struct
{
  uint32_t offsetindex;
  uint8_t buf[MAX_PACKET_LEN];
  uint16_t payload_len;
} Packet_t;

void DispData(Packet_t *pkt);
static Packet_t RxPkt;
/* packet decode */
uint32_t Packet_Decode(uint8_t c)
{
  switch (status)
  {
    case 0: //Read 1st Byte
      if (c == 0x5A)
        status = 1;
      break;
    case 1: //Read 2nd Byte
      if (c == 0xA5)
        status = 2;
      break;
    case 2: //Read 3rd Byte
      status = 3;
      break;
    case 3: //Read 4th Byte
      status = 4;
      break;
    case 4: //Read 5th Byte
      status = 5;
      break;
    case 5: //Read 6th Byte
      status = 6;
      break;
    case 6: //Read 7th~41th Byte Total:35Bytes Data
      RxPkt.buf[RxPkt.offsetindex++] = c;
      if (RxPkt.offsetindex >= RxPkt.payload_len)
      {
        DispData(&RxPkt);
        status = 0;
        RxPkt.offsetindex = 0;
      }
      break;
    default:
      status = 0;
      break;
  }
}

void DispData(Packet_t *pkt) {

  if (pkt->buf[9] == 0xB0) /* Angular Velocity raw value from buf[10] to buf[15] */
  {
    AV[0] = ((float)(int16_t)(pkt->buf[10] + (pkt->buf[11] << 8))) / 10;
    AV[1] = ((float)(int16_t)(pkt->buf[12] + (pkt->buf[13] << 8))) / 10;
    AV[2] = ((float)(int16_t)(pkt->buf[14] + (pkt->buf[15] << 8))) / 10;
  }

  if ((AV[1] > 15) && (AV[1] < 100)) {
    intensity = 100;
  }
  else {
    intensity = 0;
  }

  Serial.print(AV[1]);
  Serial.print(" ");
  Serial.print(String(intensity) + " ");
  Serial.print("\r\n");

}


void setup() {
  Serial.begin(115200);
  softSerial.setTimeout(100);
  Serial.setTimeout(50);
  printer("\nSETUP:");
  Serial.flush();

  //Reset and Initialize the Bluetooth module
  printer("\tBT: RESETTING");
  bluetoothModule.reset();
  printer("\tBT: RESET DONE");
  printer("\tBT: INITIALIZING");
  bluetoothModule.init(EMS_BLUETOOTH_ID);
  printer("\tBT: INITIALIZED");

  //Add the EMS channels and start the control
  printer("\tEMS: INITIALIZING CHANNELS");
  emsSystem.addChannelToSystem(&emsChannel1);
  emsSystem.addChannelToSystem(&emsChannel2);
  EMSSystem::start();
  printer("\tEMS: INITIALIZED");
  printer("\tEMS: STARTED");
  printer("SETUP DONE (LED 13 WILL BE ON)");
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  RxPkt.payload_len = 35;//ID(2Bytes)+Accelerate(7Bytes)+AngularVelocity(7Bytes)+Mag(7Bytes)+EulerAngle(7Bytes)+Press(5Bytes)=35Bytes
}

String command = "";
String hexCommandString;
const String BTLE_DISCONNECT = "Connection End";

void loop() {
  String timing = "10";
  if (Serial.available())
  {
    uint8_t ch = Serial.read();
    //Serial.println(ch,HEX); If you want to watch the raw data uncomment this code.
    Packet_Decode(ch);
  }
  command = "C0I" + String(intensity) + "T" + timing + "G";
  printer(command);
  emsSystem.doCommand(&command);
}
