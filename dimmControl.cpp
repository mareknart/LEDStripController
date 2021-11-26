class ChannelDimmer{
    public:
        ChannelDimmer(){
        }
        long startTime;
        long stopTime;
        long deltaTime;
        long dimmStart;
        long dimmStop;
        void dimming(){            
        }
}



/*

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

*/