#ifndef CAN_FRAME_H
#define CAN_FRAME_H

#include <Arduino.h>
#include "mcp_can.h"

class CanFrame {
  
	public:
    CanFrame(unsigned long can_id, byte can_ext, byte can_dlc, byte *data);
    void sendCAN(MCP_CAN CAN);


    unsigned long can_id;
    byte can_ext;
    byte can_dlc;
    byte *data;
};

extern byte data580[];
extern byte data581[];
extern byte data582[];
extern byte data583[];
extern byte data584[];
extern byte data590[];
extern byte data591[];
extern byte data592[];
extern byte data593[];
extern byte data594[];
extern byte data595[];
extern byte data59A[];
extern byte data59B[];
extern byte data59F[];
extern byte data780[];
extern byte data781[];
extern byte data782[];
extern byte data783[];
extern byte data784[];

#endif