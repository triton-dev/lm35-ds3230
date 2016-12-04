


/*
* Arduino Pro Micro
* Board: Arduino Micro
* Port: /dev/ttyACMx
* Programmer: USBasp
*
*
*/

#include <DS3231.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// rtc DS3231 példány létrehozása
DS3231 rtc;

// Beállított idő jelzője EEPROM 0x00h címen
// 0 nincs beállítva, 0xffh beállítva
const byte RTC_ok = 0;

// RTC modul címe
const byte RTC_addr = 0x68;
//RTC modul EEPROM címe
const byte RTC_EEPROM_addr = 0x57;


// AD átalakítás feszültsége. Nem 5 Volt, mert a védő diódán esik pár tized volt,
const int VDD = 469;

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// LCD LED kapcsoló gombja.
const int LCD_BE_GOMB = 4;

// Egygombos ki-be kapcsoló állapotjelzője.
boolean LCD_be = false;

// Relé kimenet.
const int RELE = 5;

// Relé állapotjezője.
boolean RELE_be = false;

// LM35 analóg pinje.
const int lm35 = A0;

// Hőmérsékletek tömbje.
int meresek[10];

// Átlaghőmérséklet.
float atlaghomerseklet;

// Hiszterézis
float hist = 0.5;

// Hiszterézis DIP kapcsoló pinek
const int HISTPIN1 = 6;
const int HISTPIN2 = 7;

// Bekapcsoláai hőmérséklet
float be_homerseklet = 19.0;

// Mérések száma az átlaghőmérséklet számításához.
byte meresszam = 0;

// Hőmérséklet mérés közötti idő msec-ben. 6000 msec, percenként 10 mérés.
const unsigned long int mereskoz = 6000;

// Relé bekapcsolások közötti minimum idő msec-ben.
// Határesetben a relé ki-be kapcsolgatna, minimum időt kell biztosítani.
// Most ez próbából 1perc (60000msec).
const unsigned long int bekapcsolaskoz = 60000;


// A relé előző bekapcsolásának időpontja.
unsigned long int elozo_bekapcsolas = millis();

// Hiszterézis
byte Hyst;




// *********************************************************
// setup()
void setup() {
  pinMode(LCD_BE_GOMB, INPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("---Thermostat---");
  lcd.setCursor(0, 1);
  lcd.print("<> triton-dev <>");
  delay(2000);
  lcd.clear();
  lcd.print("Inicializalas...");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 10; i++) {
    lcd.backlight();
    meres();
    delay(600);
    lcd.print("**");
  }
  lcd.noBacklight();
  lcd.clear();
  // RTC beállítása
  if (EEPROM.read(0) == 0) {
    rtc.setRTC(00, 49, 21, 5, 24, 11, 16);
    EEPROM.write(0, 255);
  }

} // end setup
// *********************************************************


// *********************************************************
// loop()
void loop() {

  // Hiszterézis pinek olvasásása, hiszterézis beállítása
  Hyst = hister_olvas();
  Hyst = 0; // amíg nincs bekötve a DIP kapcsoló
  // LCD háttérfény bekapcsolása
  if (digitalRead(LCD_BE_GOMB) == HIGH && !LCD_be) {
    LCD_be = true;
    lcd.backlight();
    delay(250); // Várakozás, hogy ne kapcsolgasson ki-be az LCD LED-je.
  }

  // LCD háttérfény kikapcsolása
  if (digitalRead(LCD_BE_GOMB) == HIGH && LCD_be) {
    LCD_be = false;
    lcd.noBacklight();
    delay(250); // Várakozás, hogy ne kapcsolgasson ki-be az LCD LED-je.
  }

  rtc.getRTC();


  // Mérés, ha az előző mérés óta mereskoz idő eltelt.
  if (rtc.second % 6 == 0) {
    meres();
  }

  // Relé bekapcsolása, ha az átlaghőmérséklet hiszterézissel alacsonyabb,
  // mint a minimum hőmérséklet és még a relé nincs bekapcsolva.
  if ((int)atlaghomerseklet * 100 != (int)be_homerseklet * 100) {
    if ((atlaghomerseklet - hist < be_homerseklet) && !RELE_be) {
      RELE_be = true;
      digitalWrite(RELE, HIGH);
    }

    // Relé elengedése, ha az átlaghőmérséklet  eléri, vagy meghaladja
    //  a bekapcsolási hőmérsékletet.
    if ((atlaghomerseklet > be_homerseklet) ) {
      elozo_bekapcsolas = millis();
      RELE_be = false;
      digitalWrite(RELE, LOW);
    }
  }


  RTCido();

} //end loop
// *********************************************************


// *********************************************************
// Funkciók:

// Mérés
void meres() {
  // Mérés.
  int ertek = analogRead(lm35);
  meresek[meresszam] = ertek;
  //lcd.setCursor(0, 0);
  // lcd.print(meresszam);
  // lcd.write(byte(B01111110));
  // lcd.print(ertek * VDD / 1023.0, 1);
  meresszam++;
  // 10 mérés átlagának számítása.
  if (meresszam == 10) {
    meresszam = 0;
    int atlagol = 0;
    for (int i = 0; i < 10; i++) {
      atlagol += meresek[i];
    }
    atlaghomerseklet = atlagol / 10.0 * VDD / 1024;
    //   lcd.print(" ");
    //   lcd.print(atlaghomerseklet, 1);
    //  lcd.print(" ");
    //  lcd.print(be_homerseklet, 1);
  }
} // Mérés vége.

// Hiszterézis beolvasása a HISTPINx alapján
//
byte hister_olvas() {
  byte h = 0;
  if (digitalRead(HISTPIN1)) {
    h += 2;
  }
  if (digitalRead(HISTPIN2)) {
    h += 1;
  }
  return h;
} // Hiszterézis olvasás vége.

// RTC kezelése
void RTCido() {
  lcd.setCursor(0, 0);
  lcd.print(atlaghomerseklet, 1);
  lcd.print(" ");
  lcd.print(be_homerseklet, 1);
  lcd.print(" ");
  lcd.print(Hyst);
  if (RELE_be) {
    lcd.print("#");
  }
  else {
    lcd.print("-");
  }
  lcd.print("P");
  lcd.setCursor(0, 1);
  rtc.getRTCdatetime();
  lcd.print(rtc.RTCdatetime);

}
