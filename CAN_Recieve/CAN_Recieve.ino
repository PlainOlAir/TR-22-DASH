/*
   0x50 - Temp[01]    Lap Num[23]   Lap Time[4567]
   0x51 - Kill Sw[01] Volt[23]      Clutch[45]      GPS Sats Num[67]
   0x52 - Oil P[01]   Fuel P[23]    TC Lvl[45]      N/O[67]
   0x53 - RPM[01]     GPS Spd[23]   TC[45]          N/O[67]
*/

//CANbus setup
#include <same51_can.h>
SAME51_CAN can;

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

int modeTC = 0; //#
int whlslp = 0; //km/h

int killsw = 0; //#

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
int i = 0;
int killsw_temp = 0;
int starttime = millis();
const int logotime = 7000;

//LED Strip setup
#include <Adafruit_NeoPixel.h>
const int PIN = 4;
const int NUMPIXELS = 14;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
const int rpm_max = 9000;
const int rpm_min = 1000;
const int rpm_rng = rpm_max - rpm_min;
const int rpm_step = (rpm_rng) / NUMPIXELS;

static void off_LED() {
  if (millis() % 1500 > 750) {
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
  } else {
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0));
    }
  }
  pixels.show();
}

static void tc_LED() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255));
  }
  pixels.show();
}

static void clutch_LED() {
  int current_step = floor( (millis() % 700) / (700 / 7));

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.setPixelColor(current_step, pixels.Color(255, 0, 0));
  pixels.setPixelColor(NUMPIXELS - (current_step + 1), pixels.Color(255, 0, 0));
  pixels.show();
}

static void tach_LED(int rev) {
  int current_step = (max(rpm_min, rev) - rpm_min) / rpm_step;

  //Above limit

  if (rev > rpm_max) {
    if (millis() % 100 > 50) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
      }
    } else {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 255));
      }
    }

    //Below limit
  } else {
    for (i = 0; i < NUMPIXELS; i++) {
      if (i < current_step) {
        if (i < 8) {
          pixels.setPixelColor(i, pixels.Color(0, 255, 0));
        } else if (i >= 8) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 255));
        }
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
    }
  }
  pixels.show();
}

void setup() {
  pixels.begin();

  Serial1.begin(115200);
  while (!Serial1);

  uint8_t ret;
  ret = can.begin(MCP_ANY, CAN_500KBPS, MCAN_MODE_CAN);

  if (ret != CAN_OK) {
    while (1);
  }

  while (millis() - starttime < logotime) {
    continue;
  }
  Serial1.print("page important" + endChar);
  delay(100);
  Serial1.print("hight.val=" + String(t_high) + endChar + "lowfp.val=" + String(fp_low) + endChar + "lowop.val=" + String(op_low) + endChar + "lowbatt.val=" + String(batt_low) + endChar);
  page = "important";
}

void loop() {

  uint8_t ret;
  uint32_t id;
  uint8_t len;
  uint8_t buf[8];
  uint8_t i;

  ret = can.readMsgBuf(&id, &len, buf);

  //Get CAN data
  if (ret == CAN_OK) {
    switch (id) {
      case 0x50:
        t = (buf[1] * 256 + buf[0]);
        break;
      case 0x51:
        killsw = (buf[1] * 256 + buf[0]);
        batt = (buf[3] * 256 + buf[2]);
        break;
      case 0x52:
        op = (buf[1] * 256 + buf[0]);
        fp = (buf[3] * 256 + buf[2]);
        tc_lvl = (buf[5] * 256 + buf[4]);
        tc_lvl = tc_lvl = 5 - round((tc_lvl - 347) / 16);
        break;
      case 0x53:
        rpm = (buf[1] * 256 + buf[0]);
        tc = (buf[5] * 256 + buf[4]);
        clutch = (buf[7] * 256 + buf[6]);
        break;
    }
  }

  //Update values in each page
  if (page == "important") {
    Serial1.print("ecut.val=" + String(t) + endChar);
    Serial1.print("ecubatt.val=" + String(batt) + endChar);
    Serial1.print("ecuop.val=" + String(op) + endChar);
    Serial1.print("ecufp.val=" + String(fp) + endChar);
    Serial1.print("rpm.val=" + String(rpm) + endChar);

  } else if (page == "race") {
    Serial1.print("ecut.val=" + String(t) + endChar);
    Serial1.print("ecubatt.val=" + String(batt) + endChar);
    Serial1.print("ecuop.val=" + String(op) + endChar);
    Serial1.print("ecufp.val=" + String(fp) + endChar);
    Serial1.print("rpm.val=" + String(rpm) + endChar);

    if (tc_lvl < -10) {
      Serial1.print("tc.txt=\"OFF\"" + endChar);
    } else if (tc_lvl <= 0) {
      Serial1.print("tc.txt=\"1\"" + endChar);
    } else {
      Serial1.print("tc.txt=\"" + String(tc_lvl) + "\"" + endChar);
    }

    if (tc != 0) {
      Serial1.print("t5.bco=8160" + endChar + "t5.pco=63515" + endChar);
    } else {
      Serial1.print("t5.bco=0" + endChar + "t5.pco=65535" + endChar);
    }

    if (clutch != 0) {
      Serial1.print("t6.bco=8160" + endChar + "t6.pco=63515" + endChar);
    } else {
      Serial1.print("t6.bco=0" + endChar + "t6.pco=65535" + endChar);
    }

  }

  //Warnings
  if (page == "race" && !warning && rpm > 750) {
    if (t > t_high && millis() - twarntime > warning_sleep) {
      warning = true;
      lastpage = page;
      page = "hight";
      Serial1.print("page hight");
    } else if (fp < fp_low && millis() - fpwarntime > warning_sleep) {
      warning = true;
      lastpage = page;
      page = "lowfp";
      Serial1.print("page lowfp");

    } else if (op < op_low && millis() - opwarntime > warning_sleep) {
      warning = true;
      lastpage = page;
      page = "lowop";
      Serial1.print("page lowop");

    } else if (batt < batt_low && millis() - battwarntime > warning_sleep) {
      warning = true;
      lastpage = page;
      page = "lowbatt";
      Serial1.print("page lowbatt");
    }
    if (warning) {
      Serial1.print(endChar);
      delay(100);
    }
  }

  //Screen switch/Ack
  if (analogRead(A2) >= 700 && millis() - button_time > button_depress) {
    button_time = millis();

    if (page == "important") {
      page = "race";
      Serial1.print("page race");

    } else if (page == "race") {
      killsw_temp = 0;
      page = "important";
      Serial1.print("page important");

    } else if (page == "hight") {
      page = lastpage;
      twarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);

    } else if (page == "lowfp") {
      page = lastpage;
      fpwarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);

    } else if (page == "lowop") {
      page = lastpage;
      opwarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);

    } else if (page == "lowbatt") {
      page = lastpage;
      battwarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);
    }
    Serial1.print(endChar);
    delay(100);
    if (page == "important") {
      Serial1.print("hight.val=" + String(t_high) + endChar + "lowfp.val=" + String(fp_low) + endChar + "lowop.val=" + String(op_low) + endChar + "lowbatt.val=" + String(batt_low) + endChar);
    }
  }

  //Car state
  if (page == "important") {
    if (killsw != 0) {
      Serial1.print("t9.txt=\"CAR ON\"" + endChar + "t9.pco=2016" + endChar);
    } else {
      Serial1.print("t9.txt=\"CAR OFF\"" + endChar + "t9.pco=63488" + endChar);
    }
  }

  //LEDs
  if (killsw == 0) {
    off_LED();
  } else if (killsw != 0) {
    if (tc != 0) {
      tc_LED();
    } else if (clutch != 0) {
      clutch_LED();
    } else {
      tach_LED(rpm);
    }
  }
}

// END FILE
