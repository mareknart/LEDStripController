#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include "RTClib.h"
#include "Arduino.h"
#include "EEPROM.h"
#include "EEPROMAnything.h"
#include "ChannelDimmer.h"

#define CHANNEL1_PIN 9 // pin do sterowania led white
#define CHANNEL2_PIN 6
#define CHANNEL3_PIN 5
#define MENU 2 // przyciski 1
#define UP 4   // 2
#define DOWN 7 // 3
#define OK 8   // 4

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
ChannelDimmer Channel1(10);
ChannelDimmer Channel2(10);
ChannelDimmer Channel3(10);

unsigned long times, timenow, previous_time = 0;
byte buttonLast = 0;     // przyciski
byte menuLevel = 0;      // oznaczenie poziomu menu
byte cursorPosition = 0; // znacznik pozycji menu
boolean screenSaverOn = false, buttonMask = false;
boolean znacznik = false;    // znacznik drukowania tekstu
int screenSaverTime = 30000; // czas wygaszacza w mili
byte button;
byte minutesStartTime, hoursStartTime, minutesStopTime, hoursStopTime; // czas wlaczania i wylaczania led ogolny
int yTime, xTime;
byte gwdim = 0, gbdim = 0, grdim = 0;
byte pressPad1, pressPad2, pressPad3, pressPad4;
byte displayColumns = 20;
byte displayRows = 4;

void setup()
{
  Serial.begin(9600);
  lcd.begin(displayColumns, displayRows); // ilosc kolumn i rzedow wyswietlacza
  Wire.begin();
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  timenow = millis() + screenSaverTime; // przedział czasu do wygaszacza
  pinMode(MENU, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(OK, INPUT_PULLUP);
  pinMode(CHANNEL1_PIN, OUTPUT); // biały
  pinMode(CHANNEL3_PIN, OUTPUT); // niebieski
  pinMode(CHANNEL2_PIN, OUTPUT); // czerwony
  analogWrite(CHANNEL1_PIN, 0);
  analogWrite(CHANNEL3_PIN, 0);
  analogWrite(CHANNEL2_PIN, 0);

  Channel1.startTime = EEPROMReadlong(0);
  Channel1.stopTime = EEPROMReadlong(4);
  Channel2.startTime = EEPROMReadlong(8);
  Channel2.stopTime = EEPROMReadlong(12);
  Channel3.startTime = EEPROMReadlong(16);
  Channel3.stopTime = EEPROMReadlong(20);
}

// główna pętla programu z odczytem przycisków
void loop()
{
  button = 0;
  times = millis();
  pressPad1 = digitalRead(MENU); // przycisk MENU
  pressPad2 = digitalRead(UP);   // przycisk +
  pressPad3 = digitalRead(DOWN); // przycisk -
  pressPad4 = digitalRead(OK);   // przycisk OK
  if (pressPad1 == LOW)
  {
    button = 1;
  }
  if (pressPad2 == LOW)
  {
    button = 2;
  }
  if (pressPad3 == LOW)
  {
    button = 3;
  }
  if (pressPad4 == LOW)
  {
    button = 4;
  }
  if (button != 0 && buttonMask == false)
  {
    buttonMask = true;
    buttonLast = button;
    delay(200);
  }

  flagaMenu(menuLevel);
  wygaszacz();
  ledOn();
  if (buttonLast == 1 && buttonMask)
  {
    if (menuLevel == 0)
    { // wejśćie do menu głównego
      mainMenu(0);
      delay(10);
      buttonMask = false;
    }
    timenow = millis() + screenSaverTime;
  }
  if (buttonLast > 1 && menuLevel == 0)
  {
    buttonLast == 0;
    buttonMask = false;
  }
  if (button == 1 && buttonMask)
  { // wyjście z menu glownego do ekranu głównego
    if (menuLevel == 1)
    { // w menu glownym
      // mainMenu(1);
      // delay(10);
      //  buttonMask = false;
      // znacznik = false;
    }
    if (menuLevel == 10)
    {
      menuLevel = 1;
    }
    if (menuLevel == 11 || menuLevel == 12 || menuLevel == 13)
    {
      TimeSave();
      menuLevel = 1;
    }
    znacznik = false;
    mainMenu(1);
    buttonMask = false;
    delay(10);
    znacznik = false;
    timenow = millis() + screenSaverTime;
  }
  if (button == 2 && buttonMask)
  { // przycisk + (2)
    if (menuLevel == 1)
    { // przesuwanie kursora do wybory menu
      cursorPosition++;
      if (cursorPosition > 3)
      {
        cursorPosition = 0;
      }
      mainMenu(0);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel == 10)
    { // minuta +
      setZegar(1);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel > 10 && menuLevel < 14)
    {
      SetChannelTimes(0, cursorPosition);
    }
    timenow = millis() + screenSaverTime;
  }
  if (button == 3 && buttonMask)
  { // przycisk - (3)
    if (menuLevel == 1)
    { // przesuwanie kursora do wybory menu
      cursorPosition--;
      if (cursorPosition == 255)
      {
        cursorPosition = 3;
      }
      mainMenu(0);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel == 10)
    { // minuta -
      setZegar(2);
      delay(10);
      buttonMask = false;
    }
    if (menuLevel > 10 && menuLevel < 14)
    {
      SetChannelTimes(1, cursorPosition);
    }
    timenow = millis() + screenSaverTime;
  }
  if (button == 4 && buttonMask)
  { // przycisk OK (4)
    if (menuLevel == 1)
    {
      mainMenu(2);
      delay(10);
      buttonMask = false;
      znacznik = false;
    }
    if (menuLevel == 10)
    {
    }
    if (menuLevel > 10 && menuLevel < 14)
    {
      if (cursorPosition == 1)
      {
        cursorPosition = 0;
      }
      else
      {
        cursorPosition = 1;
      }
      delay(300);
      timenow = millis() + screenSaverTime;
    }
  }
}
//==================== Funkcje

// funkcja menu głównego
void mainMenu(byte mpoz)
{
  if (mpoz == 0)
  { // zmiana kursora wyboru
    lcd.clear();
    buttonLast = 1;
    menuLevel = 1;
  }
  if (mpoz == 1)
  { // wyjscie do ekranu głównego prze MENU
    lcd.clear();
    menuLevel = 0;
    cursorPosition = 0;
    buttonLast = 0;
  }
  if (mpoz == 2)
  { // zatwierdzenie wyboru w menu głównym
    lcd.clear();
    menuLevel = 10 + cursorPosition;
    cursorPosition = 0;
    delay(10);
  }
  mpoz = 0;
}

// funkcja wyświetlania menu (rozne poziomy)
void flagaMenu(byte flag)
{
  if (flag == 0)
  { // ekran główny
    // if (!znacznik) {
    byte Wdim = map(gwdim, 0, 255, 0, 100);
    byte Bdim = map(gbdim, 0, 255, 0, 100);
    byte Rdim = map(grdim, 0, 255, 0, 100);
    char *info[] = {"  Ch1:  Ch2:  Ch3:", "%", "1 Menu|2 +|3 -|4 OK"};
    //centerDisplayText("Kontroler LED", 1);
    lcd.setCursor(0, 1);
    lcd.print(info[0]);
    lcd.setCursor(2,2);
    lcd.print(Wdim);
    lcd.print(info[1]);
    lcd.setCursor(8, 2);    
    lcd.print(Bdim);
    lcd.print(info[1]);
    lcd.setCursor(14, 2);    
    lcd.print(Rdim);
    lcd.print(info[1]);
    lcd.setCursor(0, 3);
    lcd.print(info[2]);
    displayCurrentTime(6, 0);
  }
  if (flag == 1)
  { // menu główne
    char *opcjeMenu[] = {"Czas", "LED1", "LED2", "LED3", "Menu"};
    lcd.setCursor(8, 0);
    lcd.print(opcjeMenu[4]);
    for (int row = 0; row < 2; row++)
    {
      lcd.setCursor(4, row + 2);
      lcd.print(opcjeMenu[row]);
    }
    for (int row = 0; row < 2; row++)
    {
      lcd.setCursor(12, row + 2);
      lcd.print(opcjeMenu[row + 2]);
    }
    if (cursorPosition < 2)
    {
      lcd.setCursor(2, cursorPosition + 2);
      lcd.print(">");
    }
    else
    {
      lcd.setCursor(10, cursorPosition);
      lcd.print(">");
    }
    delay(10);
  }
  if (flag == 10)
  { // ustawianie zegarka
    centerDisplayText("Ustawianie czasu", 0);
    displayCurrentTime(6, 2);
  }
  char *opcjeLed[] = {"Set Channel 1", "Set Channel 2", "Set Channel 3", "hh mm", "Czas", "Start:", "Stop:"};
  if (flag > 10 && flag < 14)
  { // ustawianie czasow led
    if (!znacznik)
    {
      if (flag == 11)
      { // ustawianie bialego
        lcd.setCursor(0, 0);
        lcd.print(opcjeLed[0]);
      }
      if (flag == 12)
      {
        lcd.setCursor(0, 0);
        lcd.print(opcjeLed[1]);
      }
      if (flag == 13)
      {
        lcd.setCursor(0, 0);
        lcd.print(opcjeLed[2]);
      }
      lcd.setCursor(4, 1);
      lcd.print(opcjeLed[4]);
      lcd.setCursor(11, 1);
      lcd.print(opcjeLed[3]);
      lcd.setCursor(3, 2);
      lcd.print(opcjeLed[5]);
      lcd.setCursor(4, 3);
      lcd.print(opcjeLed[6]);
      SetChannelTimes(2, cursorPosition);
      znacznik = true;
      delay(10);
    }
    if (cursorPosition == 0)
    {
      lcd.setCursor(19, 3);
      lcd.print(" ");
      lcd.setCursor(19, 2);
    }
    else
    {
      lcd.setCursor(19, 2);
      lcd.print(" ");
      lcd.setCursor(19, 3);
    }
    lcd.print("<");
  }
}

// funcja wygaszacza
void wygaszacz()
{
  if (times > timenow && screenSaverOn == false && button == 0)
  {
    lcd.setBacklight(LOW);
    screenSaverOn = true;
    //lcd.noDisplay();
    delay(5);
  }
  if (screenSaverOn == true && button == 4)
  {
    timenow = millis() + screenSaverTime;
    lcd.setBacklight(HIGH);
    screenSaverOn = false;
    lcd.clear();
    lcd.display();
    menuLevel = 0;
    buttonLast = 0;
    buttonMask = 0;
    znacznik = false;
    delay(5);
  }
}

// funkcja nastawiania czasu
void setZegar(byte inc)
{
  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();
  if (inc == 1)
  {
    m++;
    if (m > 59)
    {
      h++;
      if (h > 23)
      {
        h = 0;
      }
      m = 0;
    }
  }
  if (inc == 2)
  {
    m--;
    if (m < 0)
    {
      h--;
      if (h < 0)
      {
        h = 23;
      }
      m = 59;
    }
  }
  char zegar[8];
  sprintf(zegar, "%02d:%02d:%02d", h, m, 0);
  lcd.setCursor(6, 2);
  lcd.print(zegar);
  rtc.adjust(DateTime(0, 0, 0, h, m, 0));
}

// funkcja nastawy LED
void SetChannelTimes(byte inc, byte poz)
{
  odczytajCzasy();
  if (poz == 0)
  {
    xTime = minutesStartTime;
    yTime = hoursStartTime;
  }
  else
  {
    xTime = minutesStopTime;
    yTime = hoursStopTime;
  }
  if (inc == 0)
  {
    xTime++;
    if (xTime > 59)
    {
      yTime++;
      if (yTime > 23)
      {
        yTime = 0;
      }
      xTime = 0;
    }
    delay(100);
  }
  if (inc == 1)
  {
    xTime--;
    if (xTime < 0)
    {
      yTime--;
      if (yTime < 0)
      {
        yTime = 23;
      }
      xTime = 59;
    }
    delay(100);
  }
  if (poz == 0)
  {
    minutesStartTime = xTime;
    hoursStartTime = yTime;
  }
  else
  {
    minutesStopTime = xTime;
    hoursStopTime = yTime;
  }
  char StartTime[8];
  char StopTime[8];
  lcd.setCursor(11, 2);
  sprintf(StartTime, "%02d:%02d", hoursStartTime, minutesStartTime);
  lcd.print(StartTime);
  lcd.setCursor(11, 3);
  sprintf(StopTime, "%02d:%02d", hoursStopTime, minutesStopTime);
  lcd.print(StopTime);
  zapiszCzasy();
}

// funcka zapisu czasow do poszczegolnych kolorow
void zapiszCzasy()
{
  long tempStart = 0;
  long tempStop = 0;
  tempStart = timeConversionToSeconds(hoursStartTime, minutesStartTime);
  tempStop = timeConversionToSeconds(hoursStopTime, minutesStopTime);
  Serial.println(tempStop);
  if (menuLevel == 11)
  {
    Channel1.startTime = tempStart;
    Channel1.stopTime = tempStop;
  }
  if (menuLevel == 12)
  {
    Channel2.startTime = tempStart;
    Channel2.stopTime = tempStop;
  }
  if (menuLevel == 13)
  {
    Channel3.startTime = tempStart;
    Channel3.stopTime = tempStop;
  }
}

// funkcja odczytu czasow z poszczegolnych kolorow
void odczytajCzasy()
{
  long tempStart = 0;
  long tempStop = 0;
  if (menuLevel == 11)
  {
    tempStart = timeConversionToHoursMinutes(Channel1.startTime);
    tempStop = timeConversionToHoursMinutes(Channel1.stopTime);
  }
  if (menuLevel == 12)
  {
    tempStart = timeConversionToHoursMinutes(Channel2.startTime);
    tempStop = timeConversionToHoursMinutes(Channel2.stopTime);
  }
  if (menuLevel == 13)
  {
    tempStart = timeConversionToHoursMinutes(Channel3.startTime);
    tempStop = timeConversionToHoursMinutes(Channel3.stopTime);
  }
  hoursStartTime = tempStart / 100;
  minutesStartTime = tempStart - (hoursStartTime * 100);
  hoursStopTime = tempStop / 100;
  minutesStopTime = tempStop - (hoursStopTime * 100);
}

// funkja włączania i wyłączania led
void ledOn()
{
  DateTime now = rtc.now();
  long hourNow = now.hour();
  int minuteNow = now.minute();
  dimm(timeConversionToSeconds(hourNow, minuteNow));
}

// funcja rozjasniania i zciemniania
void dimm(long dimmingActualTime)
{
  gwdim = Channel1.dimming(dimmingActualTime);
  gbdim = Channel2.dimming(dimmingActualTime);
  grdim = Channel3.dimming(dimmingActualTime);
  analogWrite(CHANNEL1_PIN, gwdim);
  analogWrite(CHANNEL2_PIN, gbdim);
  analogWrite(CHANNEL3_PIN, grdim);
}

// zapisywanie czasów do pamięci EPROM

void EEPROMWritelong(int address, long value)
{
  // Decomposition from a long to 4 bytes by using bitshift.
  // One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  // Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address)
{
  // Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  // Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void TimeSave()
{
  EEPROMWritelong(0, Channel1.startTime);
  EEPROMWritelong(4, Channel1.stopTime);
  EEPROMWritelong(8, Channel2.startTime);
  EEPROMWritelong(12, Channel2.stopTime);
  EEPROMWritelong(16, Channel3.startTime);
  EEPROMWritelong(20, Channel3.stopTime);
}

void displayCurrentTime(byte row, byte col)
{
  DateTime now = rtc.now();
  char zegar[14];
  sprintf(zegar, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  lcd.setCursor(row, col);
  lcd.print(zegar);
}

void centerDisplayText(String text, byte row)
{
  byte startColumn = displayColumns / 2 - (text.length() / 2);
  lcd.setCursor(startColumn, row);
  lcd.print(text);
}

long timeConversionToSeconds(long hours, int minutes)
{
  return hours * 3600 + minutes * 60;
}

long timeConversionToHoursMinutes(long timeToConvert)
{
  long hoursTemp = timeToConvert / 3600;
  long minutesTemp = (timeToConvert - (hoursTemp * 3600)) / 60;
  return (hoursTemp * 100) + minutesTemp;
}