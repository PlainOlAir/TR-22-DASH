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

//LED Strip setup
#include <Adafruit_NeoPixel.h>
const int PIN = 4;
const int NUMPIXELS = 14;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//Data values
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
const int rpm_step = (rpm_rng) / NUMPIXELS;

//Brake Pressure
const int brakep_max = 2500;
const int brakep_min = 800;

//Warning values
int t_high = 2120;  //F
int fp_low = 350;   //dpsi
int op_low = 100;   //dpsi
int batt_low = 120; //dV

//Warning logic
int warning_sleep = 30000;
int twarntime = 0;
int fpwarntime = 0;
int opwarntime = 0;
int battwarntime = 0;
bool warning = false;

//Button logic
int button_time = 0;
int button_depress = 500;

//General setup
String endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));
String page = "boot";
String lastpage = "";
String diag0 = "BATTERY VOLTAGE";
String diag1 = "GPS SIGNAL";
String diag2 = "ENGINE TEMPERATURE";
int diag = 0;
int i = 0;
int starttime = 0;
const int diagnostictime = 7000;
const int logotime = 10000;

void setup() {
  delay(2000);
  Serial1.begin(9600);
}

void loop() {
  delay(500);
  Serial1.print("n0.val=" + String(diag) + endChar);
  diag += 1;
}

// END FILE
