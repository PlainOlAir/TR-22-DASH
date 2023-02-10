/*
    0x50 - Kill Sw[01]  Clutch[23]    N/O[4567]
    0x51 - Temp[01]     Volt[23]      TC Lvl[45]    N/O[67]
    0x52 - Oil P[01]    Fuel P[23]    APP[45]       Brake P[67]
    0x53 - RPM[01]      GPS Spd[23]   TC[45]        N/O[67]
    0x54 - Lap Time[0123]             Lap Num[45]   Sats Num[67]
*/

//CANbus setup

#include <same51_can.h>
SAME51_CAN can;

String endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));

int m = 0;

int rpm = 0;    //#
int t = 0;      //F
int fp = 0;     //dpsi
int op = 0;     //dpsi
int batt = 0;   //dV
int spd = 0;    //hm/h
int tc_lvl = 0; //hm/h
int tc = 0;     //#
int clutch = 0; //#
int nsats = 0; //#
int killsw = 0; //#
int pedpos = 0; //0.1%
int brakep = 0; //psi

//RPM
const int rpm_max = 9000;
const int rpm_min = 1000;
const int rpm_rng = rpm_max - rpm_min;

//Brake Pressure
const float brakep_max = 2500;
const float brakep_min = 100;

void setup() {
  delay(2000);
  Serial.begin(115200);

  uint8_t ret;
  ret = can.begin(MCP_ANY, CAN_500KBPS, MCAN_MODE_CAN);
}

void loop() {
    uint8_t ret;
    uint32_t id;
    uint8_t len;
    uint8_t buf[8];
    uint8_t i;

    ret = can.readMsgBuf(&id, &len, buf);
  
    if (ret == CAN_OK) {
    switch (id) {
      case 0x50:
        killsw = (buf[1] * 256 + buf[0]);
        clutch = (buf[3] * 256 + buf[2]);
        break;
      case 0x51:
        t = (buf[1] * 256 + buf[0]);
        batt = (buf[3] * 256 + buf[2]);
        tc_lvl = (buf[5] * 256 + buf[4]);
        tc_lvl = 5 - round((tc_lvl - 347) / 16);
        Serial.println(batt);
        break;
      case 0x52:
        op = (buf[1] * 256 + buf[0]);
        fp = (buf[3] * 256 + buf[2]);
        pedpos = (buf[5] * 256 + buf[4]);
        brakep = (buf[7] * 256 + buf[6]);
        pedpos = round(pedpos/10.0);
        brakep = ((brakep - brakep_min) / (brakep_max - brakep_min)) * 100;  
        break;
      case 0x53:
        rpm = (buf[1] * 256 + buf[0]);
        spd = (buf[3] * 256 + buf[2]);
        tc = (buf[5] * 256 + buf[4]);
        break;
      case 0x54:
        nsats = (buf[7] * 256 + buf[6]);
      break;
    }
  }

  
}

// END FILE
