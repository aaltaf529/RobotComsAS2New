#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include "Zigbee.h"

#define BUFFSIZE        64

BufferedSerial pc(USBTX,USBRX, 115200);
Zigbee xbee(PA_9,PA_10); // Zigbee module configured with 9600 baud rate.

AnalogIn ldr(A5);
PwmOut servo(PB_3);

char buffer[BUFFSIZE]   = {0};
char msgBuff[BUFFSIZE]  = {0};
char rcvBuff[BUFFSIZE]  = {0};
int counter             = 0;
int len                 = 0;

Thread readThread;
Mutex serialMutex;

void sendValue(float value) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%.2f\r\n", value);
    }


void reader()
{
    while(1)
    {
        if(xbee.receiveMessage(rcvBuff))
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

    servo.period(0.020f);
    while (true) {
        float ldrVoltage = ldr.read() * 3.3;
        sendValue(ldrVoltage);
        if (ldrVoltage < 1.3) {
            float dutyCycle = 0.05 + ldrVoltage / 3.3 * 0.1;
            servo.write(dutyCycle);
        }
        ThisThread::sleep_for(100ms);

        len = snprintf(msgBuff, BUFFSIZE, "counter %d", counter);
        xbee.sendMessage(msgBuff);
        counter++;
        ThisThread::sleep_for(chrono::seconds(10));
    }
}