/*
    0x50 - Kill Sw[01]  Clutch[23]    N/O[4567]
    0x51 - Temp[01]     Volt[23]      TC Lvl[45]    N/O[67]
    0x52 - Oil P[01]    Fuel P[23]    APP[45]       Brake P[67]
    0x53 - RPM[01]      GPS Spd[23]   TC[45]        N/O[67]
    0x54 - Lap Time[0123]             Lap Num[45]   Sats Num[67]
*/

//CANbus setup

String endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));

void setup() {
  delay(2000);
  Serial1.begin(9600);
}

void loop() {
  delay(2000);
  Serial1.print("page logo" + endChar);
}

// END FILE
