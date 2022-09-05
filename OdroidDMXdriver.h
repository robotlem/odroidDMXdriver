#ifndef ODROIDDMXDRIVER_H
#define ODROIDDMXDRIVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <asm/termbits.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h>

#include <sys/shm.h>

#include <sys/mman.h>
#include <stdint.h>

#define	GPIO_REG_MAP            0xFF634000
#define GPIOX_FSEL_REG_OFFSET   0x116
#define GPIOX_OUTP_REG_OFFSET   0x117
#define GPIOX_INP_REG_OFFSET    0x118
#define BLOCK_SIZE              (4*1024)


struct odroidDMX{
    volatile int serialPort;
    volatile int breakPin;
    unsigned char output[513];
    volatile int maxOutput;
    volatile int activeOutput;
    volatile uint32_t * gpio;
};

void initSerialDMXPort(int serial_port);

int initGPIOpin(struct odroidDMX * dmx);

struct odroidDMX * initDMX(char port[], int breakPin);

void setMaxDMXchannel(struct odroidDMX * dmx, int maxChannel);

void setDMXchannel(struct odroidDMX * dmx, int channel, unsigned char value);

unsigned char getDMXchannel(struct odroidDMX * dmx, int channel);

unsigned char * getDMXoutputArray(struct odroidDMX * dmx);

int startOdroidDMXserver(struct odroidDMX * dmx);

int stopOdroidDMXserver(struct odroidDMX * dmx);

int odroidDMXserver(struct odroidDMX * dmx);



#endif
