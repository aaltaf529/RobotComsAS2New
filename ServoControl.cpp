#include "mbed.h"
#include <string>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "Zigbee.h"

#define BUFFSIZE        64

BufferedSerial pc(USBTX,USBRX, 9600);
Zigbee ZigBeeB(PA_5,PA_7); // Zigbee module configured with 115200 baud rate - Change in Zigbee.cpp if required.
PwmOut MyServo(PB_3);

#define MIN 1000
#define MID 1500
#define MAX 2000
#define STEP 50

char buffer[BUFFSIZE]   = {0};
char msgBuff[BUFFSIZE]  = {0};
char rcvBuff[BUFFSIZE]  = {0};
int counter             = 0;
int len                 = 0;

Thread readThread;
Mutex serialMutex;

void reader()
{
    while(1)
    {
        if(ZigBeeB.receiveMessage(rcvBuff))
        {
            printf("LDR value received\n"); //prints messsage to pc to let it know that LDR value has been received.
            if(serialMutex.trylock_for(chrono::milliseconds(500)))
            {
                if (strcmp(rcvBuff, "0")==0)
                {
                    MyServo.pulsewidth_us(MIN);
                }
                len = snprintf(msgBuff, BUFFSIZE, "\r\n%s", rcvBuff);
                //MyServo.write(msgBuff, len);
                serialMutex.unlock();

                if(rcvBuff, "0")
                {
                    MyServo.pulsewidth(MIN);
                    printf("Servo at min position");

                }

                if (rcvBuff, "1")
                {
                    MyServo.pulsewidth(MAX);
                    printf("Servo at max position");
                }
            }
        }
        ThisThread::sleep_for(chrono::milliseconds(10));
    }
}

// main() runs in its own thread in the OS
int main()
{
    for(float offset = 0.0; offset < 0.001; offset+=0.0001) {
        MyServo.pulsewidth(0.001 + offset);
        chrono::milliseconds(50);
    }
    pc.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1
    );
    readThread.start(reader);
    
    if(serialMutex.trylock_for(chrono::milliseconds(500)))
    {
        len = snprintf(buffer, BUFFSIZE, "\r\nHello, I have started :)\r\n");
        pc.write(buffer, len);
        serialMutex.unlock();
    }

    //not required because only receiving.
    //while (true) {
        //len = snprintf(msgBuff, BUFFSIZE, "counter %d", counter);
        //ZigBeeB.sendMessage(msgBuff);
        //counter++;
        //ThisThread::sleep_for(chrono::seconds(10));
    //}
}