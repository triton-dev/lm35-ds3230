/*
* DS3231 I2C RTC
* 
* triton-dev
* 2016.11.27.
*
*/

#ifndef DS3231_H_
#define DS3231_H_
#include <Arduino.h>

class DS3231 {
  private:
    byte RTC_addr;
  public:
    byte second;
    byte minute;
    byte hour;
    byte dow;
    byte day;
    byte month;
    byte year;
    byte dec2bcd(byte);
    byte bcd2dec(byte);
    char RTCtime[9];
    char RTCdate[16];
    char RTCdatetime[17];
    DS3231();
    DS3231(byte);
    ~DS3231();
    
    void setRTC(byte, byte, byte, byte, byte, byte, byte);
    void getRTC();
    void getRTCtime();
    void getRTCdate();
    void getRTCdatetime();
    byte getRTC_addr();
};
#endif
