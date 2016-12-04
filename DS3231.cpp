/*
* DS3231 I2C RTC Arduino class
*
* triton-dev
* 2016.11.27.
*
*/

#include <DS3231.h>
#include <Wire.h>

// Cím nélküli konstruktor, alapértelmezett I2C címmel.
DS3231::DS3231() {
  RTC_addr = 0x68;
}

// Címzett konstruktor, hardveresen megváltoztatott I2C címmel.
DS3231::DS3231(byte addr) {
  RTC_addr = addr;
}

DS3231::~DS3231() {
  // Nincs mit tenni.
}

byte DS3231::getRTC_addr() {
    return RTC_addr;
}

// Decimális BCD átalakítás
byte DS3231::dec2bcd(byte val) {
  return ( (val / 10 * 16) + (val % 10));
}

// BCD decimális átalakítás
byte DS3231::bcd2dec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

// RTC beállítása
void DS3231::setRTC(byte second, byte minute, byte hour, byte dow, \
            byte day, byte month, byte year) {
  Wire.beginTransmission(RTC_addr);
  Wire.write(0); // Másodperc regiszter címe
  Wire.write(dec2bcd(second)); // másodperc
  Wire.write(dec2bcd(minute)); // perc
  Wire.write(dec2bcd(hour)); // óra
  Wire.write(dec2bcd(dow)); // nap száma (1=Vasárnap, 7=Szombat)
  Wire.write(dec2bcd(day)); // nap (1 to 31)
  Wire.write(dec2bcd(month)); // hónap
  Wire.write(dec2bcd(year)); // év (0 to 99)
  Wire.endTransmission();
}

// RTC olvasása
void DS3231::getRTC() {
  Wire.beginTransmission(RTC_addr);
  Wire.write(0); // DS3231 másodperc címe
  Wire.endTransmission();
  Wire.requestFrom(RTC_addr, 7);
  // 7 bájt olvasása DS3231-ről 00h címtől kezdve
  second = bcd2dec(Wire.read() & 0x7f);
  minute = bcd2dec(Wire.read());
  hour = bcd2dec(Wire.read() & 0x3f);
  dow = bcd2dec(Wire.read());
  day = bcd2dec(Wire.read());
  month = bcd2dec(Wire.read());
  year = bcd2dec(Wire.read());
}

void DS3231::getRTCtime() {
  byte sec,min,h;
  
  Wire.beginTransmission(RTC_addr);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_addr,3);
  sec = bcd2dec(Wire.read() &0x7f);
  min = bcd2dec(Wire.read());
  h = bcd2dec(Wire.read() &0x3f);
  if (h < 10) {
    RTCtime[0] = '0';
    RTCtime[1] = h + 48;
  }
  else {
  RTCtime[0] = h / 10 + 48;
  RTCtime[1] = h % 10 + 48 ;
  }
  RTCtime[2] = ':';
  if (min < 10) {
    RTCtime[3] = '0';
    RTCtime[4] = min + 48;
  }
  else {
    RTCtime[3] = min / 10 + 48;
    RTCtime[4] = min % 10 + 48 ;
  }
  RTCtime[5] = ':';
  if (sec < 10) {
    RTCtime[6] = '0';
    RTCtime[7] = sec + 48;
  }
  else {
    RTCtime[6] = sec / 10 + 48;
    RTCtime[7] = sec % 10 + 48;
  }
  RTCtime[8] = '\0';
}

void DS3231::getRTCdate() {
  byte D,y,m,d;
  
  Wire.beginTransmission(RTC_addr);
  Wire.write(3);
  Wire.endTransmission();
  Wire.requestFrom(RTC_addr,4);
  D = bcd2dec(Wire.read());
  d = bcd2dec(Wire.read());
  m = bcd2dec(Wire.read());
  y = bcd2dec(Wire.read());
  RTCdate[0] = '2';
  RTCdate[1] = '0';
  if ( y < 10) {
    RTCdate[2] = '0';
    RTCdate[3] = y + 48;
  }
  else {
  RTCdate[2] = y / 10 + 48;
  RTCdate[3] = y % 10 + 48;
  }
  RTCdate[4] = '.';
  if (m < 10) {
    RTCdate[5] = '0';
    RTCdate[6] = m + 48;
  }
  else {
    RTCdate[5] = m / 10 + 48;
    RTCdate[6] = m % 10 + 48;
  }
  RTCdate[7] = '.';
  if (d < 10) {
    RTCdate[8] = '0';
    RTCdate[9] = d + 48;
  }
  else {
    RTCdate[8] = d / 10 + 48;
    RTCdate[9] = d % 10 + 48;
  }
  RTCdate[10] = '.';
  RTCdate[11] = ' ';
  switch (D) {
    case 1:
      RTCdate[12] = 'V';
      RTCdate[13] = 'a';
      RTCdate[14] = 's';
    break;
    case 2:
      RTCdate[12] = 'H';
      RTCdate[13] = 'e';
      RTCdate[14] = 't';
    break;
    case 3:
      RTCdate[12] = 'K';
      RTCdate[13] = 'e';
      RTCdate[14] = 'd';
    break;
    case 4:
      RTCdate[12] = 'S';
      RTCdate[13] = 'z';
      RTCdate[14] = 'e';
    break;
    case 5:
      RTCdate[12] = 'C';
      RTCdate[13] = 's';
      RTCdate[14] = 'u';
    break;
    case 6:
      RTCdate[12] = 'P';
      RTCdate[13] = 'e';
      RTCdate[14] = 'n';
    break;
    case 7:
      RTCdate[12] = 'S';
      RTCdate[13] = 'z';
      RTCdate[14] = 'o';
    break;
  }
  RTCdate[15] = '\0';
}

void DS3231::getRTCdatetime() {
  // sec,min,hour,DOW,day,Month,year
  byte s,m,h,D,d,M,y;
  Wire.beginTransmission(RTC_addr);
  Wire.write(0); // DS3231 másodperc címe
  Wire.endTransmission();
  Wire.requestFrom(RTC_addr, 7);
  // 7 bájt olvasása DS3231-ről 00h címtől kezdve
  s = bcd2dec(Wire.read() & 0x7f);
  m = bcd2dec(Wire.read());
  h = bcd2dec(Wire.read() & 0x3f);
  D = bcd2dec(Wire.read());
  d = bcd2dec(Wire.read());
  M = bcd2dec(Wire.read());
  y = bcd2dec(Wire.read());
  // Óra
  if (h < 10) {
    RTCdatetime[0] = '0';
    RTCdatetime[1] = h + 48;
  }
  else {
  RTCdatetime[0] = h / 10 + 48;
  RTCdatetime[1] = h % 10 + 48 ;
  }
  // Perc
  if (m < 10) {
    RTCdatetime[2] = '0';
    RTCdatetime[3] = m + 48;
  }
  else {
    RTCdatetime[2] = m / 10 + 48;
    RTCdatetime[3] = m % 10 + 48 ;
  }
  // Másodperc
  if (s < 10) {
    RTCdatetime[4] = '0';
    RTCdatetime[5] = s + 48;
  }
  else {
    RTCdatetime[4] = s / 10 + 48;
    RTCdatetime[5] = s % 10 + 48;
  }
  // Elválasztó
  RTCdatetime[6] = ' ';
  // Év
  if ( y < 10) {
    RTCdatetime[7] = '0';
    RTCdatetime[8] = y + 48;
  }
  else {
  RTCdatetime[7] = y / 10 + 48;
  RTCdatetime[8] = y % 10 + 48;
  }
  // Hónap
  if (M < 10) {
    RTCdatetime[9] = '0';
    RTCdatetime[10] = M + 48;
  }
  else {
    RTCdatetime[9] = M / 10 + 48;
    RTCdatetime[10] = M % 10 + 48;
  }
  // Nap
  if (d < 10) {
    RTCdatetime[11] = '0';
    RTCdatetime[12] = d + 48;
  }
  else {
    RTCdatetime[11] = d / 10 + 48;
    RTCdatetime[12] = d % 10 + 48;
  }
  // Nap neve
  switch (D) {
    case 1:
      RTCdatetime[13] = 'V';
      RTCdatetime[14] = 'a';
      RTCdatetime[15] = 's';
    break;
    case 2:
      RTCdatetime[13] = 'H';
      RTCdatetime[14] = 'e';
      RTCdatetime[15] = 't';
    break;
    case 3:
      RTCdatetime[13] = 'K';
      RTCdatetime[14] = 'e';
      RTCdatetime[15] = 'd';
    break;
    case 4:
      RTCdatetime[13] = 'S';
      RTCdatetime[14] = 'z';
      RTCdatetime[15] = 'e';
    break;
    case 5:
      RTCdatetime[13] = 'C';
      RTCdatetime[14] = 's';
      RTCdatetime[15] = 'u';
    break;
    case 6:
      RTCdatetime[13] = 'P';
      RTCdatetime[14] = 'e';
      RTCdatetime[15] = 'n';
    break;
    case 7:
      RTCdatetime[13] = 'S';
      RTCdatetime[14] = 'z';
      RTCdatetime[15] = 'o';
    break;
  }
  RTCdatetime[16] = '\0';
}
  
