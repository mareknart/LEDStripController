//dimming method return level of PWM (0-255) based on startTime, stopTime
//and period, which is time from start(PWM=0) to full (PWM=255)
//Period must be gievn in percet (0%-100%)
//100% mean only increasing PWM from 0 to 255


#include "Arduino.h"
#include "ChannelDimmer.h"

ChannelDimmer::ChannelDimmer(int dimmingPeriod)
{
  _dimmingPeriod = dimmingPeriod;
}

byte ChannelDimmer::dimming(long actualTime)
{
  if (stopTime > startTime)
  {
    float dimmingTimePeriod = float(_dimmingPeriod) / 100; // percentage of dimming period (0-50)
    long deltaTime = (stopTime - startTime) * dimmingTimePeriod;
    long dimmingStop = startTime + deltaTime;
    long dimmingStart = stopTime - deltaTime;
    dimmingLevel = 0;

    if (actualTime >= startTime && actualTime <= dimmingStop)
    {
      return map(actualTime, startTime, dimmingStop, 0, 255);
    }
    if (actualTime >= dimmingStart && actualTime <= stopTime)
    {
      return map(actualTime, dimmingStart, stopTime, 255, 0);
    }
    if (actualTime > dimmingStop && actualTime < dimmingStart)
    {
      return 255;
    }
  }
  return dimmingLevel;
}
