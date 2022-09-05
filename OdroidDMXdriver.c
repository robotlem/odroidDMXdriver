#include "OdroidDMXdriver.h"

void initSerialDMXPort(int serial_port)
{

    struct termios2 tty;

    ioctl(serial_port, TCGETS2, &tty);

    // Config für DMX:
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= CBAUDEX;
    tty.c_ispeed = 250000; // 250000 baud for DMX
    tty.c_ospeed = 250000;

    ioctl(serial_port, TCSETS2, &tty);
}

int initGPIOpin(struct odroidDMX * dmx){
    int fd;
    if ((fd = open("/dev/gpiomem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        printf("Unable to open /dev/gpiomem\n");
        return -1;
    }
    dmx->gpio = mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_REG_MAP);
    if (dmx->gpio < 0) {
        printf("Mmap failed.\n");
        return -1;
    }

    // Set direction of GPIOX.n register to out
    *(dmx->gpio + (GPIOX_FSEL_REG_OFFSET)) &= ~(1 << dmx->breakPin);
    // Set GPIOX.n to high
    *(dmx->gpio + (GPIOX_OUTP_REG_OFFSET)) |= (1 << dmx->breakPin);

    return 0;
}

struct odroidDMX * initDMX(char port[], int breakPin){
    struct odroidDMX * dmx;                                             // Generate new odroidDMX pointer
    key_t shmKey = ftok(port,breakPin);                                 // Generate Key for SHM
    //int shmid = shmget(shmKey,sizeof(dmx), 0644|IPC_CREAT);             // Generate ID for SHM
    int shmid = shmget(shmKey,sizeof(dmx), 0644|IPC_CREAT);
    if (shmid == -1){                                                   // Prevent eventual Errors
        printf("Shared memory create error\n");
        return NULL;
    }
    dmx = shmat(shmid, NULL, 0);                                        // Set the odroidDMX pointer to the SHM pointer
    if (dmx == (void *) -1) {                                           // Prevent eventual Errors
        printf("Shared memory attach error\n");
        return NULL;
    }
    dmx->serialPort = open(port, O_RDWR);                               // Open Serial Port
    initSerialDMXPort(dmx->serialPort);                                 // Configure Serial Port


    dmx->maxOutput = 512;                                               // Set max Output to 512
    dmx->activeOutput = 0;                                              // Set the actual Output state to false
    for (int z = 0; z <sizeof(dmx->output);z++){
        dmx->output[z] = 0;                                             // Set the whole output to 0
    }

    // Break Pin Section
    dmx->breakPin = breakPin;                                           // Store breakPin
    initGPIOpin(dmx);



    return dmx;

}

void setMaxDMXchannel(struct odroidDMX * dmx, int maxChannel){
    if(maxChannel >= 0 && maxChannel <= 512){
        dmx->maxOutput = maxChannel+1;
    }
}

void setDMXchannel(struct odroidDMX * dmx, int channel, unsigned char value){
    if(channel >= 1 && channel <= 512){
        dmx->output[channel] = value;
    }
}

unsigned char getDMXchannel(struct odroidDMX * dmx, int channel){
    if(channel >=1 && channel <= 512){
        return dmx->output[channel];
    }
    return 0;
}

unsigned char * getDMXoutputArray(struct odroidDMX * dmx){
    return (dmx->output+1);         // +1 becuse first byte has to be 0
}

int startOdroidDMXserver(struct odroidDMX * dmx){
    if(dmx->activeOutput == 1){                 // Prevent multiple Startings
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0){
        printf("Error by forking");
        return -2;
    }
    else if (pid == 0){
        odroidDMXserver(dmx);
    }
    return 0;
}

int stopOdroidDMXserver(struct odroidDMX * dmx){
    if(dmx->activeOutput != 1){
        return -1;
    }
    dmx->activeOutput = 0;
    return 0;
}

int odroidDMXserver(struct odroidDMX * dmx){
    printf("DMX Server Started\n");

    /*
    struct odroidDMX * dmx;                                             // Generate new odroidDMX pointer

    key_t shmKey = ftok(port,breakPin);                                 // Generate Key for SHM

    int shmid = shmget(shmKey,sizeof(dmx), 0644|IPC_CREAT);             // Generate ID for SHM

    if (shmid == -1){                                                   // Prevent eventual Errors
        printf("Shared memory create error\n");
        return -1;
    }

    dmx = shmat(shmid, NULL, 0);                                        // Set the odroidDMX pointer to the SHM pointer

    if (dmx == (void *) -1) {                                           // Prevent eventual Errors
        printf("Shared memory attach error\n");
        return -1;
    }
    */

    dmx->activeOutput = 1;

    while(dmx->activeOutput == 1){

        *(dmx->gpio + (GPIOX_OUTP_REG_OFFSET)) &= ~(1 << dmx->breakPin);// Set breakPin to LOW
        usleep(120);
        *(dmx->gpio + (GPIOX_OUTP_REG_OFFSET)) |= (1 << dmx->breakPin); // Set breakPin to HIGH
        usleep(12);
        write(dmx->serialPort, dmx->output, dmx->maxOutput+1);
        //tcdrain(dmx->serialPort);             // Wait until buffer is send or
        usleep(dmx->maxOutput*44 + 300);        // Wait for Calculated output Time
    }
    dmx->activeOutput = 99;

    printf("DMX Server ShutDown\n");

    exit(0);
}
