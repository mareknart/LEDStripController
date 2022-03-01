#ifndef ChannelDimmer_h
#define ChannelDimmer_h

class ChannelDimmer
{
public:
    long startTime;
    long stopTime;
    ChannelDimmer(int dimmingPeriod);
    byte dimming(long actualTime);
    

private:
    byte dimmingLevel;
    byte previousDimmingLevel = 0;
    int _dimmingPeriod; //in percent (1-100)
};

#endif
