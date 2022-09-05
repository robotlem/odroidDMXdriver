#include "OdroidDMXdriver.h"
#include <stdio.h>

int main(){

    printf("Start\n");

    struct odroidDMX * dmx1;
    struct odroidDMX * dmx2;

    dmx1 = initDMX("/dev/ttyS0", 3);
    dmx2 = initDMX("/dev/ttyS1", 16);

    setMaxDMXchannel(dmx1, 10);

    setMaxDMXchannel(dmx2, 10);


    startOdroidDMXserver(dmx1);
    startOdroidDMXserver(dmx2);

    for(int i = 0; i <=255; i++){
        setDMXchannel(dmx1, 1, i);
        setDMXchannel(dmx2, 3, i);
        usleep(10000);
    }

    for(int i = 0; i <=255; i++){
        setDMXchannel(dmx1, 2, i);
        setDMXchannel(dmx2, 1, i);
        usleep(10000);
    }

    for(int i = 255; i >= 0; i--){
        setDMXchannel(dmx1, 1, i);
        setDMXchannel(dmx2, 3, i);
        usleep(10000);
    }



    sleep(2);

    stopOdroidDMXserver(dmx1);
    stopOdroidDMXserver(dmx2);

    printf("End\n");

    sleep(2);

    return 0;
}
