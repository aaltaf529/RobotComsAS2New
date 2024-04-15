#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include "Zigbee.h"

#define BUFFSIZE        64

BufferedSerial pc(USBTX,USBRX, 9600);
Zigbee ZigBeeB(PA_5,PA_7); // Zigbee module configured with 9600 baud rate.

AnalogIn ldr(A5); //LDR connected to analogue pin A5 on microcontroller A.
PwmOut servo(PB_3); //The servo motor is connected to the PB_3 pin on microcontroller B.

char buffer[BUFFSIZE]   = {0};
char msgBuff[BUFFSIZE]  = {0};
char rcvBuff[BUFFSIZE]  = {0};
int counter             = 0;
int len                 = 0;

Thread readThread;
Mutex serialMutex;

void sendValue(float value) { //converts the float value to a string.
    char buffer[32]; //buffer size of 32.
    int len = snprintf(buffer, sizeof(buffer), "%.2f\r\n", value);
    }


void reader()
{
    while(1) //while loop.
    {
        if(ZigBeeB.receiveMessage(rcvBuff)) //recieving the message.
        {
            if(serialMutex.trylock_for(chrono::milliseconds(500)))
            {
                len = snprintf(msgBuff, BUFFSIZE, "\r\n%s", rcvBuff); //prints the message buff size.
                pc.write(msgBuff, len);
                serialMutex.unlock();
            }
        }
        ThisThread::sleep_for(chrono::milliseconds(10)); //waits 10ms.
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

    servo.period(0.020f); //20ms used for the servo period.
    while (true) {
        float ldrVoltage = ldr.read() * 3.3; //reads the LDR voltage value.
        sendValue(ldrVoltage); //LDR value is sent through the ZigBee.
        if (ldrVoltage < 1.3) { //if the LDR value read is less than 1.3V.
            float dutyCycle = 0.05 + ldrVoltage / 3.3 * 0.1; //move the servo motor.
            servo.write(dutyCycle);
        }
        ThisThread::sleep_for(100ms); //sleeps for 100ms.

        len = snprintf(msgBuff, BUFFSIZE, "counter %d", counter); //prints the buff size.
        ZigBeeB.sendMessage(msgBuff); //sends the message.
        counter++;
        ThisThread::sleep_for(chrono::seconds(10)); //waits 10ms.
    }
}