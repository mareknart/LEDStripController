#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include "RTClib.h"
#include "Arduino.h"
#include "EEPROM.h"
#include "EEPROMAnything.h"

#define WHITE 9  //pin do sterowania led white
#define RED 6
#define BLUE 5
#define MENU 2    //przyciski
#define UP 4      //
#define DOWN 7  //
#define OK 8      //

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
unsigned long times, timenow, previous_time = 0;
byte buttonLast = 0 ; //przyciski
byte menuLevel = 0;     //oznaczenie poziomu menu
byte menuPosition = 0;   //znacznik pozycji menu
boolean screenSaver, buttonMask = false;
boolean znacznik = false; //znacznik drukowania tekstu
int screenSaverTime = 30000; //czas wygaszacza w mili
byte button;
byte minutesStartTime, hoursStartTime, minutesStopTime, hoursStopTime; //czas wlaczania i wylaczania led ogolny
long wStart, wStop, bStart, bStop, rStart, rStop; //czasy poszczegolnych kolorow
int yTime, xTime;
byte wtdim = 0, btdim = 0, rtdim = 0;
byte wdim = 0, bdim = 0, rdim = 0, gwdim = 0, gbdim = 0, grdim = 0;

void setup() {
  Serial.begin(9600);
  // ilosc kolumn i rzedow wyswietlacza
  lcd.begin(20, 4);
  Wire.begin();
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  timenow = millis() + screenSaverTime; //przedział czasu do wygaszacza
  pinMode(MENU, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(OK, INPUT_PULLUP);
  pinMode(WHITE, OUTPUT); //biały
  pinMode(BLUE, OUTPUT); //niebieski
  pinMode(RED, OUTPUT); //czerwony
  analogWrite(WHITE, 0);
  analogWrite(BLUE, 0);
  analogWrite(RED, 0);
  screenSaver = false;
  wStart = EEPROMReadlong(0); wStop = EEPROMReadlong(4);
  bStart = EEPROMReadlong(8); bStop = EEPROMReadlong(12);
  rStart = EEPROMReadlong(16); rStop = EEPROMReadlong(20);
}

//główna pętla programu z odczytem przycisków
void loop() {
  Serial.print(button); Serial.print(":"); Serial.println(menuLevel);
  button = 0;
  times = millis();
  //println(time);
  byte  p1, p2, p3, p4;
  p1 = digitalRead(MENU); //przycisk MENU
  p2 = digitalRead(UP); //przycisk +
  p3 = digitalRead(DOWN); //przycisk -
  p4 = digitalRead(OK); //przycisk OK
  if (p1 == LOW) {
    button = 1;
  }
  if (p2 == LOW) {
    button = 2;
  }
  if (p3 == LOW) {
    button = 3;
  }
  if (p4 == LOW) {
    button = 4;
  }
  if (button != 0 && buttonMask == false) {
    buttonMask = true;
    buttonLast = button;
    delay(200);
  }

  flagaMenu(menuLevel);
  wygaszacz();
  ledOn();
  if (buttonLast == 1 && buttonMask) {
    if (menuLevel == 0) {                          //wejśćie do menu głównego
      mainMenu(0);
      delay(10);
      buttonMask = false;
    }
    timenow = millis() + screenSaverTime;
  }
  if (buttonLast > 1 && menuLevel == 0) {
    buttonLast == 0;
    buttonMask = false;
  }
  if (button == 1 && buttonMask) {   // wyjście z menu glownego do ekranu głównego
    if (menuLevel == 1 ) {        //w menu glownym
      //mainMenu(1);
      //delay(10);
      // buttonMask = false;
      //znacznik = false;
    }
    if (menuLevel == 10) {
      menuLevel = 1;
    }
    if (menuLevel == 11 || menuLevel == 12 || menuLevel == 13) {
      TimeSave();
      //mainMenu(1);
      // delay(10);
      // znacznik = false;
      //buttonMask = false;
      menuLevel = 1;
    }
    znacznik = false;
    mainMenu(1);
    buttonMask = false;
    delay(10);
    znacznik = false;
    timenow = millis() + screenSaverTime;
  }
  if (button == 2 && buttonMask) {  //przycisk + (2)
    if (menuLevel == 1) {             //przesuwanie kursora do wybory menu
      menuPosition++;
      if (menuPosition > 3) {
        menuPosition = 0;
      }
      mainMenu(0);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel == 10) {      //minuta +
      setZegar(1);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel > 10 && menuLevel < 14) {
      LedSet(0, menuPosition);
    }
    timenow = millis() + screenSaverTime;
  }
  if (button == 3 && buttonMask) {  //przycisk - (3)
    if (menuLevel == 1) {             //przesuwanie kursora do wybory menu
      menuPosition--;
      if (menuPosition == 255) {
        menuPosition = 3;
      }
      mainMenu(0);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel == 10) {    //minuta -
      setZegar(2);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel > 10 && menuLevel < 14) {
      LedSet(1, menuPosition);
    }
    timenow = millis() + screenSaverTime;
  }
  if (button == 4 && buttonMask) {  //przycisk OK (4)
    if (menuLevel == 1) {
      mainMenu(2);
      delay(10);
      buttonMask = false;
      znacznik = false;
    }
    if (menuLevel == 10) {

    }
    if (menuLevel > 10 && menuLevel < 14) {
      if (menuPosition == 1) {
        menuPosition = 0;
      }
      else {
        menuPosition = 1;
      }
      delay(300);
      timenow = millis() + screenSaverTime;
    }
  }
}
//==================== Funkcje

//funkcja menu głównego
void mainMenu(byte mpoz) {
  if (mpoz == 0) {                //zmiana kursora wyboru
    lcd.clear();
    buttonLast = 1;
    menuLevel = 1;
  }
  if (mpoz == 1) {    //wyjscie do ekranu głównego prze MENU
    lcd.clear();
    menuLevel = 0;
    menuPosition = 0;
    buttonLast = 0;
  }
  if (mpoz == 2) {    //zatwierdzenie wyboru w menu głównym
    lcd.clear();
    menuLevel = 10 + menuPosition;
    menuPosition = 0;
    delay(10);
  }
  mpoz = 0;
}

//funkcja wyświetlania menu (rozne poziomy)
void flagaMenu(byte flag) {
  if (flag == 0) {     //ekran główny
    //if (!znacznik) {
    byte Wdim = map(gwdim, 0, 255, 0, 100);
    byte Bdim = map(gbdim, 0, 255, 0, 100);
    byte Rdim = map(grdim, 0, 255, 0, 100);
    lcd.setCursor(3, 0); lcd.print("Kontroler  LED");
    lcd.setCursor(0, 2); lcd.print("W:"); lcd.print(Wdim); lcd.print("%");
    lcd.setCursor(7, 2); lcd.print("B:"); lcd.print(Bdim); lcd.print("%");
    lcd.setCursor(14, 2); lcd.print("R:"); lcd.print(Rdim); lcd.print("%");
    lcd.setCursor(0, 3); lcd.print("1 Menu|2 +|3 -|4 OK");
    // znacznik = true;
    //}
    printCurrentTime(6, 1);
  }
  if (flag == 1) {  //menu główne
    char* opcjeMenu[] = {"Czas", "WLed", "BLed", "RLed", "Menu"};
    lcd.setCursor(8, 0); lcd.print(opcjeMenu[4]);
    for (int x = 0; x < 2; x++) {
      lcd.setCursor(4, x + 2);
      lcd.print(opcjeMenu[x]);
    }
    for (int x = 0; x < 2; x++) {
      lcd.setCursor(12, x + 2);
      lcd.print(opcjeMenu[x + 2]);
    }
    if (menuPosition < 2) {
      lcd.setCursor(2, menuPosition + 2);
      lcd.print(">");
    } else {
      lcd.setCursor(10, menuPosition); lcd.print(">");
    }
    delay(10);
  }
  if (flag == 10) {         //ustawianie zegarka
    lcd.setCursor(3, 0);
    lcd.print("Ustawianie czasu");
    printCurrentTime(6, 2);
  }
  char* opcjeLed[] = {"Set LED bialy", "Set LED niebieski", "Set LED czerwony", "hh mm", "Czas start:", "Czas stop :"};
  if (flag > 10 && flag < 14) {     //ustawianie czasow led
    if (!znacznik) {
      if (flag == 11) { //ustawianie bialego
        lcd.setCursor(0, 0); lcd.print(opcjeLed[0]);
      }
      if (flag == 12) {
        lcd.setCursor(0, 0); lcd.print(opcjeLed[1]);
      }
      if (flag == 13) {
        lcd.setCursor(0, 0); lcd.print(opcjeLed[2]);
      }
      lcd.setCursor(13, 1); lcd.print(opcjeLed[3]);
      lcd.setCursor(1, 2); lcd.print(opcjeLed[4]);
      lcd.setCursor(1, 3); lcd.print(opcjeLed[5]);
      LedSet(2, menuPosition);
      znacznik = true;
      delay(10);
    }
    if (menuPosition == 0) {
      lcd.setCursor(19, 3); lcd.print(" ");
      lcd.setCursor(19, 2);
    } else {
      lcd.setCursor(19, 2); lcd.print(" ");
      lcd.setCursor(19, 3);
    }
    lcd.print("<");
  }
}

//funcja wygaszacza
void wygaszacz() {
  if (times > timenow && screenSaver == false && button == 0) {
    lcd.setBacklight(LOW);
    screenSaver = true;
    lcd.noDisplay();
    delay(5);
  }
  if (screenSaver == true && button == 4) {
    timenow = millis() + screenSaverTime;
    lcd.setBacklight(HIGH);
    screenSaver = false;
    lcd.clear();
    lcd.display();
    menuLevel = 0;
    buttonLast = 0;
    buttonMask = 0;
    znacznik = false;
    delay(5);
  }
}

//funkcja nastawiania czasu
void setZegar(byte inc) {
  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();
  if (inc == 1) {
    m++;
    if (m > 59) {
      h++;
      if (h > 23) {
        h = 0;
      }
      m = 0;
    }
  }
  if (inc == 2) {
    m--;
    if (m < 0) {
      h--;
      if (h < 0) {
        h = 23;
      }
      m = 59;
    }
  }
  char zegar[8];
  sprintf(zegar, "%02d:%02d:%02d", h, m, 0);
  lcd.setCursor(6, 2);
  lcd.print(zegar);
  rtc.adjust(DateTime(0, 0 , 0 , h , m, 0));
}

//funkcja nastway LED
void LedSet(byte inc, byte poz) {
  odczytajCzasy();
  if (poz == 0) {
    xTime = minutesStartTime;
    yTime = hoursStartTime;
  } else {
    xTime = minutesStopTime;
    yTime = hoursStopTime;
  }
  if (inc == 0) {
    xTime ++;
    if (xTime > 59) {
      yTime++;
      if (yTime > 23) {
        yTime = 0;
      }
      xTime = 0;
    }
    delay(100);
  }
  if (inc == 1) {
    xTime --;
    if (xTime < 0) {
      yTime--;
      if (yTime < 0) {
        yTime = 23;
      }
      xTime = 59;
    }
    delay(100);
  }
  if (poz == 0) {
    minutesStartTime = xTime;
    hoursStartTime = yTime;
  } else {
    minutesStopTime = xTime;
    hoursStopTime = yTime;
  }
  char StartTime[8];
  char StopTime[8];
  lcd.setCursor(13, 2);
  sprintf(StartTime, "%02d:%02d", hoursStartTime, minutesStartTime);
  lcd.print(StartTime);
  lcd.setCursor(13, 3);
  sprintf(StopTime, "%02d:%02d", hoursStopTime, minutesStopTime);
  lcd.print(StopTime);
  zapiszCzasy();
}

//funcka zapisu czasow do poszczegolnych kolorow
void zapiszCzasy() {
  int tempStart = 0;
  int tempStop = 0;
  tempStart = hoursStartTime * 100 + minutesStartTime;
  tempStop = hoursStopTime * 100 + minutesStopTime;
  if (menuLevel == 11) {
    wStart = tempStart;
    wStop = tempStop;
  }
  if (menuLevel == 12) {
    bStart = tempStart;
    bStop = tempStop;
  }
  if (menuLevel == 13) {
    rStart = tempStart;
    rStop = tempStop;
  }
}

//funkcja odczytu czasow z poszczegolnych kolorow
void odczytajCzasy() {
  int tempStart = 0;
  int tempStop = 0;
  if (menuLevel == 11) {
    tempStart = wStart;
    tempStop = wStop;
  }
  if (menuLevel == 12) {
    tempStart = bStart;
    tempStop = bStop;
  }
  if (menuLevel == 13) {
    tempStart = rStart;
    tempStop = rStop;
  }
  hoursStartTime = tempStart / 100;
  minutesStartTime = tempStart - (hoursStartTime * 100);
  hoursStopTime = tempStop / 100;
  minutesStopTime = tempStop - (hoursStopTime * 100);
}

//funkja włączania i wyłączania led
void ledOn() {
  DateTime now = rtc.now();
  long h = now.hour();
  byte m = now.minute();
  byte s = now.second();
  int temporaryTimeMinutes = h * 100 + m;
  long temporaryTimeSeconds = ((h * 3600) + (m * 60) + s);
  if (temporaryTimeMinutes >= wStart && temporaryTimeMinutes < wStop || temporaryTimeMinutes >= bStart && temporaryTimeMinutes < bStop || temporaryTimeMinutes >= rStart && temporaryTimeMinutes < rStop) {
    dimm(temporaryTimeSeconds);
  } else {
    analogWrite(WHITE, 0);
    analogWrite(BLUE, 0);
    analogWrite(RED, 0);
  }
}

//funcja rozjasniania i zciemniania
void dimm(long dimmingActualTime) {
  if (wStop - wStart > 0 || bStop - bStart > 0 || rStop - rStart > 0) {
    //long dimmingActualTime = tptime;
    //kontrola dim bialego
    long wMStart = (((wStart / 100) * 60) + (wStart - (wStart / 100) * 100)) * 60; //bialy start na sekundy
    long wMStop = (((wStop / 100) * 60) + (wStop - (wStop / 100) * 100)) * 60; //bialy stop na sekundy
    long deltawM = (wMStop - wMStart) * 0.1; //10% roznicy czasu na rozjasnianie i zciemnianie
    long wdimStop = wMStart + deltawM;
    long wdimStart = wMStop - deltawM;
    if (dimmingActualTime >= wMStart && dimmingActualTime <= wdimStop) {
      byte wdim = map(dimmingActualTime, wMStart, wdimStop, 0, 255);
      if (wtdim != wdim) {
        analogWrite(WHITE, wdim);
        gwdim = wdim;
      }
    }
    if (dimmingActualTime >= wdimStart && dimmingActualTime <= wMStop) {
      wdim = map(dimmingActualTime, wdimStart, wMStop, 255, 0);
      if (wtdim != wdim) {
        analogWrite(WHITE, wdim);
        gwdim = wdim;
      }
    }
    if (dimmingActualTime > wdimStop && dimmingActualTime < wdimStart) {
      analogWrite(WHITE, 255);
      gwdim = 255;
    }
    //kontrola dim niebieskiego
    long bMStart = (((bStart / 100) * 60) + (bStart - (bStart / 100) * 100)) * 60; //niebieski start na sekundy
    long bMStop = (((bStop / 100) * 60) + (bStop - (bStop / 100) * 100)) * 60; //niebieski stop na sekundy
    long deltabM = (bMStop - bMStart) * 0.1;
    long bdimStop = bMStart + deltabM;
    long bdimStart = bMStop - deltabM;
    if (dimmingActualTime >= bMStart && dimmingActualTime <= bdimStop) {
      bdim = map(dimmingActualTime, bMStart, bdimStop, 0, 255);
      if (btdim != bdim) {
        analogWrite(BLUE, bdim);
        gbdim = bdim;
      }
    }
    if (dimmingActualTime >= bdimStart && dimmingActualTime <= bMStop) {
      bdim = map(dimmingActualTime, bdimStart, bMStop, 255, 0);
      if (btdim != bdim) {
        analogWrite(BLUE, bdim);
        gbdim = bdim;
      }
    }
    if (dimmingActualTime > bdimStop && dimmingActualTime < bdimStart) {
      analogWrite(BLUE, 255);
      gbdim = 255;
    }
    //kontrola dim czerwonego
    long rMStart = (((rStart / 100) * 60) + (rStart - (rStart / 100) * 100)) * 60; //czerwony start na sekundy
    long rMStop = (((rStop / 100) * 60) + (rStop - (rStop / 100) * 100)) * 60; //czerwony stop na sekundy
    long deltarM = (rMStop - rMStart) * 0.1;
    long rdimStop = rMStart + deltarM;
    long rdimStart = rMStop - deltarM;
    if (dimmingActualTime >= rMStart && dimmingActualTime <= rdimStop) {
      rdim = map(dimmingActualTime, rMStart, rdimStop, 0, 255);
      if (rtdim != rdim) {
        analogWrite(RED, rdim);
        grdim = rdim;
      }
    }
    if (dimmingActualTime >= rdimStart && dimmingActualTime <= rMStop) {
      rdim = map(dimmingActualTime, rdimStart, rMStop, 255, 0);
      if (rtdim != rdim) {
        analogWrite(RED, rdim);
        grdim = rdim;
      }
    }
    if (dimmingActualTime > rdimStop && dimmingActualTime < rdimStart) {
      analogWrite(RED, 255);
      grdim = 255;
    }

    rtdim = rdim;
    wtdim = wdim;
    btdim = bdim;
  }
}

// zapisywanie czasów do pamięci EPROM

void EEPROMWritelong(int address, long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


void TimeSave() {
  EEPROMWritelong(0, wStart); EEPROMWritelong(4, wStop);
  EEPROMWritelong(8, bStart); EEPROMWritelong(12, bStop);
  EEPROMWritelong(16, rStart); EEPROMWritelong(20, rStop);
}


void printCurrentTime(byte row, byte col) {
  DateTime now = rtc.now();
  char zegar[14];
  sprintf(zegar, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  lcd.setCursor(row, col);
  lcd.print(zegar);
}
