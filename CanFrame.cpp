
#include "CanFrame.h"
#include <Arduino.h>


CanFrame::CanFrame(unsigned long canId, byte canExt, byte canDlc, byte *data) {
  this->can_id = canId;
  this->can_ext = canExt;
  this->can_dlc = canDlc;
  this->data = data;
}

void CanFrame::sendCAN(MCP_CAN CAN){
  CAN.sendMsgBuf(this->can_id,  this->can_ext,  this->can_dlc,  this->data);
  delay(1);
}


//Hardcoded frames
byte data580[] = {0x00, 
                  0xC0, 
                  0x00};
byte data581[] = {0x00, // Battery level - data[0] = 0x64 - 100%
                  0x95, 
                  0x37, 
                  0xE3, 
                  0x3D, 
                  0x04, 
                  0x42}; 
byte data582[] = {0x00, 
                  0x00, 
                  0x78, 
                  0x9E};
byte data583[] = {0xF4, 
                  0x01, 
                  0x48, 
                  0xA3, 
                  0x01};
byte data584[] = {0x00, 
                  0x00, 
                  0x00, 
                  0x00};
byte data590[] = {0x0B, 
                  0x14};
byte data591[] = {0x14, 
                  0x0A, 
                  0x13, 
                  0xB0, 
                  0x0C, 
                  0x00, 
                  0x00, 
                  0x00};
byte data592[] = {0x1D, 
                  0x10, 
                  0x1A, 
                  0x10, 
                  0x94, 
                  0x93};
byte data593[] = {0x20, 
                  0x00, 
                  0x15, 
                  0x00, 
                  0x31, 
                  0x00, 
                  0x02, 
                  0x5E};
byte data594[] = {0x28, 
                  0x27, 
                  0x08, 
                  0xF4};
byte data595[] = {0x61, 
                  0x10, 
                  0xB0, 
                  0x0A, 
                  0xAC, 
                  0x79};
byte data59A[] = {0x9A, 
                  0x00, 
                  0x13, 
                  0x00, 
                  0x02, 
                  0x00};
byte data59B[] = {0x02, 
                  0x03};
byte data59F[] = {0x01}; // data[0] = 0x00 or 0x01
byte data780[] = {0x00, 
                  0xC0, 
                  0x00};
byte data781[] = {0x00,  // Battery level - data[0] = 0x64 - 100%
                  0x95, 
                  0x37, 
                  0xE3, 
                  0x3D, 
                  0x04, 
                  0x42};
byte data782[] = {0x21, 
                  0x00, 
                  0x7A, 
                  0x9E};
byte data783[] = {0xA0, 
                  0x0F, 
                  0x48, 
                  0xA3, 
                  0x01};
byte data784[] = {0x00, 
                  0x0C, 
                  0x00};                              
