#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include "Zigbee.h"

#define BUFFSIZE        64

BufferedSerial pc(USBTX,USBRX, 9600);
Zigbee ZigBeeA(PA_9,PA_10); // Zigbee module configured with 115200 baud rate - Change in Zigbee.cpp if required.
AnalogIn LDR(A5);

int MinValue = 10000;
int MaxValue = 60000;

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
        if(ZigBeeA.receiveMessage(rcvBuff))
        {
            if(serialMutex.trylock_for(chrono::milliseconds(500)))
            {
                len = snprintf(msgBuff, BUFFSIZE, "\r\n%s", rcvBuff);
                pc.write(msgBuff, len);
                serialMutex.unlock();
            }
        }
        ThisThread::sleep_for(chrono::milliseconds(10));
    }
}

// main() runs in its own thread in the OS
int main()
{
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

    while (true) {
        len = snprintf(msgBuff, BUFFSIZE, "%d, %d\r\n", LDR.read_u16(), counter); //unsignes 16 bit integer read from the LDR.
        pc.write(msgBuff, len); //prints the LDR value to the pc.
        ZigBeeA.sendMessage(msgBuff);
        counter++;
        ThisThread::sleep_for(chrono::seconds(10));
    }
}