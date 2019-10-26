#include "mySerial.h"
#include <iostream>
#include "unistd.h"
using namespace std;



int  main(void)
{

    mySerial serial("/dev/ttyUSB0", 9600);

    // One Byte At the time
    cout << "Sending 1" << endl;
    while(1){
        serial.Send("1");

        usleep(50000);

        serial.Send("2");

        usleep(50000);
    }

    // cout << "Sending 0" << endl;
    // serial.Send(0);
    // serial.Send(132);

    // An array of byte
    // unsigned char  dataArray[] = { 142,0};
    // serial.Send(dataArray,sizeof(dataArray));

    // Or a string
    // serial.Send("this is it\r\n");

    return 0;
}