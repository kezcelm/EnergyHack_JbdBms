#include "JbdBms.h"
#include "mcp_can.h"
#include "CanFrame.h"

//--------------------------------------------------------------------------

#define SLEEP_PIN 3

// CAN-BUS setup
#define CAN_INT 2
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);              // Set CS pin
#define RS_TO_MCP2515 true            // Set this to false if Rs is connected to your Arduino
#define RS_OUTPUT MCP_RX0BF           // RX0BF is a pin of the MCP2515. You can also define an Arduino pin here

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
int lastLen = -1;
unsigned char lastBuf[8];                                    
unsigned long lastMsgTime = 0;
unsigned long lastBusActivity = millis();

//--------------------------------------------------------------------------
// BMS data setup
JbdBms myBms(7, 8); // RX, TX

unsigned char batterySOC = 0;
unsigned int batteryCycle = 0;
unsigned int batteryVoltage = 0;
int batteryCurrent = 0;
unsigned int balanceCapacity = myBms.getBalanceCapacity();
unsigned int ratedCapacity = myBms.getRateCapacity();
unsigned int socCapacity = 0;

//--------------------------------------------------------------------------
// Data for time checking
unsigned long actTime = 0;
unsigned long prevTime = 0;

//--------------------------------------------------------------------------
// Intiaize CAN frames to send
CanFrame frame580(0x580, 0, 3, data580);
CanFrame frame581(0x581, 0, 7, data581);
CanFrame frame582(0x582, 0, 4, data582);
CanFrame frame583(0x583, 0, 5, data583);
CanFrame frame584(0x584, 0, 4, data584);
CanFrame frame590(0x590, 0, 2, data590);
CanFrame frame591(0x591, 0, 8, data591);
CanFrame frame592(0x592, 0, 6, data592);
CanFrame frame593(0x593, 0, 8, data593);
CanFrame frame594(0x594, 0, 4, data594);
CanFrame frame595(0x595, 0, 6, data595);
CanFrame frame59A(0x59A, 0, 6, data59A);
CanFrame frame59B(0x59B, 0, 2, data59B);
CanFrame frame59F(0x59F, 0, 1, data59F);
CanFrame frame780(0x780, 0, 3, data780);
CanFrame frame781(0x781, 0, 7, data781);
CanFrame frame782(0x782, 0, 4, data782);
CanFrame frame783(0x783, 0, 5, data783);
CanFrame frame784(0x784, 0, 3, data784);


void setup()
{
  Serial.begin(9600);
  CAN.begin(CAN_500KBPS, MCP_8MHz);
  CAN.setMode(MODE_NORMAL);
  pinMode(SLEEP_PIN, INPUT_PULLUP);
}

void MCP2515_ISR()
{
    flagRecv  = 1;
}

void loop()
{
  flagRecv = 0;                   // clear flag
  lastBusActivity = millis();
  actTime = millis();
  if (actTime - prevTime >= 1000UL)    // read battery level in every 1s
 
  {
    prevTime = actTime;
    myBms.readBmsData();
    batterySOC = myBms.getChargePercentage();
    batteryVoltage = myBms.getVoltage();
    batteryCurrent = (int)myBms.getCurrent();
    batteryCycle = myBms.getCycle();
    balanceCapacity = myBms.getBalanceCapacity(); // 16150
    ratedCapacity = myBms.getRateCapacity(); //17250
    socCapacity = ((float)balanceCapacity / 100) * batterySOC;

    data781[0] = batterySOC;
    data781[6] = highByte(ratedCapacity);
    data781[5] = lowByte(ratedCapacity);
    data781[4] = highByte(balanceCapacity);
    data781[3] = lowByte(balanceCapacity);
    data781[2] = highByte(socCapacity);
    data781[1] = lowByte(socCapacity);

    data782[3] = highByte(batteryVoltage);
    data782[2] = lowByte(batteryVoltage);
    data782[1] = highByte(batteryCurrent);
    data782[0] = lowByte(batteryCurrent);
    data593[3] = highByte(batteryCycle);
    data593[2] = lowByte(batteryCycle);

    // if (batteryVoltage>=41000){ //stop chargingwhen
    //   myBms.stopCharging();
    // }

    // TMP just to check battery capacity
    // store Coulomb counter value in CAN 591 frame data
    data591[0] = 0x00;
    data591[1] = 0x00;
    data591[2] = 0x00;
    data591[4] = highByte(batteryVoltage);
    data591[3] = lowByte(batteryVoltage); //shov actual voltage

    copy_frames();
  }

  while (CAN_MSGAVAIL == CAN.checkReceive()) 
  {
    // read data,  len: data length, buf: data buf
    CAN.readMsgBuf(&len, buf);
    lastLen = len;
    memcpy(lastBuf, buf, sizeof(buf));
    lastMsgTime = millis();

    switch (CAN.getCanId())
    {
      case 0x020: //  0x40000020
        data59F[0] = 0x01;
        frame59F.sendCAN(CAN);
        break;
      case 0x120:
        data59F[0] = 0x00;
        frame59F.sendCAN(CAN);
        break;
      case 0x42C:
        frame590.sendCAN(CAN);
        frame591.sendCAN(CAN);
        frame592.sendCAN(CAN);
        break;
      case 0x22C:
        frame580.sendCAN(CAN);
        frame581.sendCAN(CAN);
        frame582.sendCAN(CAN);
        frame583.sendCAN(CAN);
        break;
      case 0x26C:
        frame580.sendCAN(CAN);
        frame581.sendCAN(CAN);
        frame583.sendCAN(CAN);
        break;
      case 0x52C:
        frame593.sendCAN(CAN);
        frame594.sendCAN(CAN);
        frame595.sendCAN(CAN);
        break;
      case 0x72C:
        frame59A.sendCAN(CAN);
        frame59B.sendCAN(CAN);
        break;
      case 0x641:
        frame780.sendCAN(CAN);
        frame781.sendCAN(CAN);
        frame782.sendCAN(CAN);
        frame784.sendCAN(CAN);
        break;
//charging
      case 0x7C0:
        frame780.sendCAN(CAN);
        break;
      case 0x7C1:
        frame781.sendCAN(CAN);
        break;
      case 0x7C2:
        frame782.sendCAN(CAN);
        frame783.sendCAN(CAN);
        frame784.sendCAN(CAN);
        break;
      case 0x2EC:
    Serial.println("Start 0x2EC");
        frame580.sendCAN(CAN);
        frame581.sendCAN(CAN);
        frame582.sendCAN(CAN);
        frame583.sendCAN(CAN);
        break;
      default:
        break;
    }
  }
}


void copy_frames(){
  *(data580+0) = *(data780+0);
  *(data580+1) = *(data780+1);
  *(data580+2) = *(data780+2);

  *(data581+0) = *(data781+0);
  *(data581+1) = *(data781+1);
  *(data581+2) = *(data781+2);
  *(data581+3) = *(data781+3);
  *(data581+4) = *(data781+4);
  *(data581+5) = *(data781+5);
  *(data581+6) = *(data781+6);

  *(data582+0) = *(data782+0);
  *(data582+1) = *(data782+1);
  *(data582+2) = *(data782+2);
  *(data582+3) = *(data782+3);

  *(data583+0) = *(data783+0);
  *(data583+1) = *(data783+1);
  *(data583+2) = *(data783+2);
  *(data583+3) = *(data783+3);
  *(data583+4) = *(data783+4);

  *(data584+0) = *(data784+0);
  *(data584+1) = *(data784+1);
  *(data584+2) = *(data784+2);
  *(data584+3) = *(data784+3);
}