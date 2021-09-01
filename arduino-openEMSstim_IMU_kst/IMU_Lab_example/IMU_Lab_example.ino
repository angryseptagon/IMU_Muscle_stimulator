
#define MAX_PACKET_LEN          (128)
uint8_t Dataout[20];
uint8_t ID;
int16_t AccRaw[3];
int16_t GyoRaw[3];
int16_t MagRaw[3];
float Eular[3];
float AV[3];   /* Euler Angle */
int32_t Pressure;
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
uint32_t Packet_ Decode(uint8_t c)
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

void DispData(Packet_t *pkt)
{

  if (pkt->buf[0] == 0x90) /* user ID */
  {
    ID = pkt->buf[1];
  }
  if (pkt->buf[2] == 0xA0) /* Acceleration raw value from buf[3] to buf[8] */
  {
    //try to write some code read acceleration
  }

  if (pkt->buf[9] == 0xB0) /* Angular Velocity raw value from buf[10] to buf[15] */
  {
    AV[0] = ((float)(int16_t)(pkt->buf[10] + (pkt->buf[11] << 8))) / 10;
    AV[1] = ((float)(int16_t)(pkt->buf[12] + (pkt->buf[13] << 8))) / 10;
    AV[2] = ((float)(int16_t)(pkt->buf[14] + (pkt->buf[15] << 8))) / 10;
  }

    /* Print the Angular Velocity */ 
    Serial.print("Angular Velocity Pitch:");
    Serial.print(AV[0]);
    Serial.print("  ");
    Serial.print("Roll:");
    Serial.print(AV[1]);
    Serial.print("  ");
    Serial.print("Yaw:");
    Serial.print(AV[2]);
    Serial.print("  ");
    Serial.print("\r\n");
}


void setup()
{
  Serial.begin(115200);
  RxPkt.payload_len = 35;//ID(2Bytes)+Accelerate(7Bytes)+AngularVelocity(7Bytes)+Mag(7Bytes)+EulerAngle(7Bytes)+Press(5Bytes)=35Bytes
  //if you change the data format the payload_len will need to change
}

void loop()
{
  if (Serial.available())
  {
    uint8_t ch = Serial.read();
    //Serial.println(ch,HEX); If you want to watch the raw data uncomment this code.
    Packet_Decode(ch);
  }
}
