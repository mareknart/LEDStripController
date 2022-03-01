# LEDStripController
This is three channels LED controller based on Arduino Uno. Each channel controlled by PWM level from 0 to 255.
Project using ChannelDimmer library which simulate sunset and sunrise. 
Dimming time is calculated from `(Stop time - Start time)* dimming period`, where *dimming perid* is given during instance creation.
