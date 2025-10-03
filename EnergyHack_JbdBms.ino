#include "JbdBms.h"
#include "mcp_can.h"
#include "CanFrame.h"

#include <avr/sleep.h>
#include <avr/interrupt.h>

//--------------------------------------------------------------------------

#define SLEEP_PIN 3

// CAN-BUS setup
#define CAN_INT 2
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);              // Set CS pin
#define RS_TO_MCP2515 true            // Set this to false if Rs is connected to your Arduino
#define RS_OUTPUT MCP_RX0BF           // RX0BF is a pin of the MCP2515. You can also define an Arduino pin here

// Sleep CAN BUS
unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
int lastLen = -1;
unsigned char lastBuf[8];                                    
unsigned long lastMsgTime = 0;
unsigned long lastBusActivity = 0;

//--------------------------------------------------------------------------
// BMS data setup
#define MAX_CHARGE_VOLTAGE 41500
JbdBms myBms(7, 8); // RX, TX

struct BatteryStatus {
  uint8_t soc;            // State of charge [%]
  uint16_t cycle;
  uint16_t voltage;       // [mV]
  int current;            // [mA]
  uint16_t balanceCapacity;
  uint16_t rateCapacity;
  uint16_t socCapacity;
  uint8_t mosActual;
};
BatteryStatus battery; 

  /* mosActual
     0x03  chg - OFF dis OFF
     0x02  chg - ON dis OFF
     0x01  chg - OFF dis ON
     0x00  chg - ON dis ON */

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

void setup() {
  Serial.begin(9600);

  CAN.begin(CAN_500KBPS, MCP_8MHz);
  CAN.setMode(MODE_NORMAL);
  lastBusActivity = millis();

  pinMode(SLEEP_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SLEEP_PIN), wakeUp, FALLING);

  myBms.setMosfet(battery.mosActual);
}

void MCP2515_ISR() {
    flagRecv  = 1;
}

void loop() {
  if (digitalRead(SLEEP_PIN) == HIGH) {  // go to sleep if not connected
    delay(100);
    goToSleep();
    delay(500);
  }

  flagRecv = 0;                   // clear flag
  lastBusActivity = millis();
  actTime = millis();
  if (actTime - prevTime >= 1000UL)    // read battery data in every 1s
  {
    prevTime = actTime;
    updateBatteryFrames();
    copyFrames();
  }
  checkCanBus();
}

void goToSleep() {
  setupMosfet(0x03);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  noInterrupts(); 
  EIFR = bit(INTF1);
  interrupts(); 

  sleep_cpu();

  sleep_disable();
}

void wakeUp() {
  setupMosfet(0x01);
}

void setupMosfet(uint8_t mosSetup) {
  if (mosSetup != battery.mosActual){
    battery.mosActual = mosSetup;
    myBms.setMosfet(mosSetup);
  }
}

void updateBatteryFrames() {
  // odczyt danych z BMS
  myBms.readBmsData();
  battery.soc = myBms.getChargePercentage();
  battery.voltage = myBms.getVoltage();
  battery.current = (int)myBms.getCurrent();
  battery.cycle = myBms.getCycle();
  battery.balanceCapacity = myBms.getBalanceCapacity();
  battery.rateCapacity = myBms.getRateCapacity();
  battery.socCapacity = ((float)battery.balanceCapacity / 100) * battery.soc;

  // sterowanie MOSFETem ładowania
  if (battery.voltage < MAX_CHARGE_VOLTAGE){
    setupMosfet(0x00);  // charging allowed
  } else {
    setupMosfet(0x01);  // charging not allowed
  }

  // aktualizacja danych w ramkach CAN
  data781[0] = battery.soc;
  data781[6] = highByte(battery.rateCapacity);
  data781[5] = lowByte(battery.rateCapacity);
  data781[4] = highByte(battery.balanceCapacity);
  data781[3] = lowByte(battery.balanceCapacity);
  data781[2] = highByte(battery.socCapacity);
  data781[1] = lowByte(battery.socCapacity);

  data782[3] = highByte(battery.voltage);
  data782[2] = lowByte(battery.voltage);
  data782[1] = highByte(battery.current);
  data782[0] = lowByte(battery.current);

  data593[3] = highByte(battery.cycle);
  data593[2] = lowByte(battery.cycle);

  data591[0] = 0x00;
  data591[1] = 0x00;
  data591[2] = 0x00;
  data591[4] = highByte(battery.voltage);
  data591[3] = lowByte(battery.voltage);
}

void checkCanBus() {
  while (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    lastLen = len;
    memcpy(lastBuf, buf, sizeof(buf));
    lastMsgTime = millis();

    unsigned long id = CAN.getCanId();

    if (id == 0x020) {
      data59F[0] = 0x01;
    } else if (id == 0x120) {
      data59F[0] = 0x00;
    }

    for (uint8_t i = 0; i < mappingsCount; i++) {
      if (mappings[i].id == id) {
        for (uint8_t j = 0; j < mappings[i].count; j++) {
          mappings[i].frames[j]->sendCAN(CAN);
        }
        break;
      }
    }
  }
}

void copyFrames() {
  memcpy(data580, data780, 3);
  memcpy(data581, data781, 7);
  memcpy(data582, data782, 4);
  memcpy(data583, data783, 5);
  memcpy(data584, data784, 4);
}