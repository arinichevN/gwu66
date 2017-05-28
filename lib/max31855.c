
#include "max31855.h"

int max31855_init(int sclk, int cs, int miso) {
    pinModeOut(cs);
    pinModeOut(sclk);
    pinModeIn(miso);
    pinHigh(cs);
    return 1;
}

/*
int max31855_read(float *result, int sclk, int cs, int miso) {
    uint32_t v;
    pinLow(cs);
    delayUsBusy(1000);
    {
        int i;
        for (i = 31; i >= 0; i--) {
            pinLow(sclk);
            delayUsBusy(1000);
            if (pinRead(miso)) {
                v |= (1 << i);
            }
            pinHigh(sclk);
            delayUsBusy(1000);
        }
    }
    pinHigh(cs);
    if (v & 0x4) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: thermocouple is short-circuited to VCC where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        return 0;
    }
    if (v & 0x2) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: thermocouple is short-circuited to GND where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        return 0;
    }
    if (v & 0x1) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: thermocouple input is open where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        return 0;
    }
    if (v & 0x8000) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: fault expected but not found where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        return 0;
    }
    if (v & 0x80000000) {
        v = 0xFFFFC000 | ((v >> 18) & 0x00003FFFF);
    } else {
        v >>= 18;
    }
    *result = *result * 0.25;
    return 1;
}
*/
static uint32_t spiread321(int sclk, int cs, int miso) { 
  int i;
  uint32_t d = 0;

  // backcompatibility!


  pinLow(cs);
  delayUsBusy(1);

    pinLow(sclk);
    delayUsBusy(1);

    for (i=31; i>=0; i--) {
      pinLow(sclk);
      delayUsBusy(1);
      d <<= 1;
      if (pinRead(miso)) {
	d |= 1;
      }
      
      pinHigh(sclk);
      delayUsBusy(1);
    }


  pinHigh(cs);
  //Serial.println(d, HEX);
  return d;
}
int max31855_read(float *result, int sclk, int cs, int miso) {

  int32_t v;

  v = spiread321(sclk, cs, miso);

  //Serial.print("0x"); Serial.println(v, HEX);

  /*
  float internal = (v >> 4) & 0x7FF;
  internal *= 0.0625;
  if ((v >> 4) & 0x800) 
    internal *= -1;
  Serial.print("\tInternal Temp: "); Serial.println(internal);
  */

  if (v & 0x7) {
      puts("error");
    return 0; 
  }

  if (v & 0x80000000) {
    // Negative value, drop the lower 18 bits and explicitly extend sign bits.
    v = 0xFFFFC000 | ((v >> 18) & 0x00003FFFF);
  }
  else {
    // Positive value, just drop the lower 18 bits.
    v >>= 18;
  }
  //Serial.println(v, HEX);
  
  double centigrade = v;

  // LSB = 0.25 degrees C
  centigrade *= 0.25;
  *result=centigrade;
  return 1;
}




