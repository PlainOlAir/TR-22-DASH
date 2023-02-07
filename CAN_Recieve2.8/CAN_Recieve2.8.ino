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
const int logotime = 5000;

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

  delay(1000);
  Serial1.print("page diagnostics" + endChar);
  page = "diagnostics";
  delay(500);
  starttime = millis();
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
        killsw = (buf[1] * 256 + buf[0]);
        clutch = (buf[3] * 256 + buf[2]);
        break;
      case 0x51:
        t = (buf[1] * 256 + buf[0]);
        batt = (buf[3] * 256 + buf[2]);
        tc_lvl = (buf[5] * 256 + buf[4]);
        break;
      case 0x52:
        op = (buf[1] * 256 + buf[0]);
        fp = (buf[3] * 256 + buf[2]);
        pedpos = (buf[5] * 256 + buf[4]);
        brakep = (buf[7] * 256 + buf[6]);
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

  if (diag < 4) {
    if (diag == 0) {
      Serial1.print("c0.pco=0" + endChar);
      for(i = 0; i < diag0.length() + 1; i++) {
        Serial1.print("t0.txt=\"" + diag0.substring(0,i) + "\"" + endChar);
        delay(50);        
      }
      diag = 1;

      if (batt >= 120) {
        Serial1.print("c0.pco=2016" + endChar);
      } else {
        Serial1.print("c0.pco=63488" + endChar);
      }

    } else if (diag == 1) {
      delay(300);
      Serial1.print("c1.pco=0" + endChar);
      for(i = 0; i < diag1.length() + 1; i++) {
        Serial1.print("t1.txt=\"" + diag1.substring(0,i) + "\"" + endChar);
        delay(50);        
      }
      diag = 2;

      if (nsats >= 4) {
        Serial1.print("c1.pco=2016" + endChar);
      } else {
        Serial1.print("c1.pco=63488" + endChar);
      }
      
    } else if (diag == 2) {
      delay(300);
      Serial1.print("c2.pco=0" + endChar);
      for(i = 0; i < diag2.length() + 1; i++) {
        Serial1.print("t2.txt=\"" + diag2.substring(0,i) + "\"" + endChar);
        delay(50);        
      }
      diag = 3;

      if (t <= 2120) {
        Serial1.print("c2.pco=2016" + endChar);
      } else {
        Serial1.print("c2.pco=63488" + endChar);
      }

    } else if (diag == 3) {
      delay(2000);
      Serial1.print("page logo" + endChar);
      page = "logo";
      delay(logotime);
      Serial1.print("page important" + endChar);
      page = "important";
      diag = 4;
    }
    return;
  }

  //Update values in each page
  if (page == "important") {
    pedpos = round(pedpos/10.0);
    brakep = (brakep - brakep_min) / (brakep_max - brakep_min);   
    Serial1.print("ecut.val=" + String(t) + endChar);
    Serial1.print("ecubatt.val=" + String(batt) + endChar);
    Serial1.print("ecuop.val=" + String(op) + endChar);
    Serial1.print("ecufp.val=" + String(fp) + endChar);
    Serial1.print("rpm.val=" + String(rpm) + endChar);
    Serial1.print("pedpos.val=" + String(pedpos) + endChar);
    Serial1.print("brakepos.val=" + String(brakep) + endChar);

  } else if (page == "race") {
    tc_lvl = tc_lvl = 5 - round((tc_lvl - 347) / 16);
    Serial1.print("ecut.val=" + String(t) + endChar);
    Serial1.print("ecubatt.val=" + String(batt) + endChar);
    Serial1.print("ecuop.val=" + String(op) + endChar);
    Serial1.print("ecufp.val=" + String(fp) + endChar);
    Serial1.print("rpm.val=" + String(rpm) + endChar);
    Serial1.print("gpsspd.val=" + String(spd) + endChar);

    if (tc_lvl < -10) {
      Serial1.print("tclvl.txt=\"OFF\"" + endChar);
    } else if (tc_lvl <= 0) {
      Serial1.print("tclvl.txt=\"1\"" + endChar);
    } else {
      Serial1.print("tclvl.txt=\"" + String(tc_lvl) + "\"" + endChar);
    }

    if (tc != 0) {
      Serial1.print("tc.bco=8160" + endChar + "tc.pco=63515" + endChar);
    } else {
      Serial1.print("tc.bco=0" + endChar + "tc.pco=65535" + endChar);
    }

    if (clutch != 0) {
      Serial1.print("launch.bco=8160" + endChar + "launch.pco=63515" + endChar);
    } else {
      Serial1.print("launch.bco=0" + endChar + "launch.pco=65535" + endChar);
    }

  }

  //Warnings
  if (rpm > 750) {
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
    } else if (page == "important") {
      if (millis() % 1500 > 750) {
        if (t > t_high) {
          Serial1.print("ecut.bco=63488" + endChar);
        }        
        if (fp < fp_low) {
          Serial1.print("ecufp.bco=63488" + endChar);
        }
        if (op < op_low) {
          Serial1.print("ecuop.bco=63488" + endChar);
        }
        if (batt < batt_low) {
          Serial1.print("ecubatt.bco=63488" + endChar);
        }
      } else {
        Serial1.print("ecut.bco=0" + endChar);
        Serial1.print("ecufp.bco=0" + endChar);
        Serial1.print("ecuop.bco=0" + endChar);
        Serial1.print("ecubatt.bco=0" + endChar);
      }      
    }
    
  }

  //Screen switch/Ack
  if (analogRead(A2) >= 700 && millis() - button_time > button_depress) {
    button_time = millis();

    if (page == "important") {
      page = "race";
      Serial1.print("page race");

    } else if (page == "race") {
      page = "important";
      Serial1.print("page important");

    } else if (page == "hight" || t < t_high) {
      page = lastpage;
      twarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);

    } else if (page == "lowfp" || fp > fp_low) {
      page = lastpage;
      fpwarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);

    } else if (page == "lowop" || op > op_low) {
      page = lastpage;
      opwarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);

    } else if (page == "lowbatt" || batt > batt_low) {
      page = lastpage;
      battwarntime = millis();
      warning = false;
      Serial1.print("page " + lastpage);
    }
    Serial1.print(endChar);
    delay(100);
  }

  //Car state
  if (page == "important") {
    if (killsw != 0) {
      Serial1.print("carstate.txt=\"CAR ON\"" + endChar + "carstate.pco=2016" + endChar);
    } else {
      Serial1.print("carstate.txt=\"CAR OFF\"" + endChar + "carstate.pco=63488" + endChar);
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
