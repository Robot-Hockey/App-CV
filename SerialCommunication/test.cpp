#include "mySerial.h"
#include <iostream>
#include "unistd.h"
using namespace std;



int  main(void)
{

    mySerial serial("/dev/ttyUSB0", 9600);

    // One Byte At the time
    cout << "Sending 1" << endl;
    char one {1}, zero {0};
    char* str1[] = "1,1,1,1,1";
    serial.Send(str1);

    sleep(3);

    cout << "Sending 0" << endl;
    serial.Send("0,0,0,0,0");
    // serial.Send(132);

    // An array of byte
    // unsigned char  dataArray[] = { 142,0};
    // serial.Send(dataArray,sizeof(dataArray));

    // Or a string
    // serial.Send("this is it\r\n");

    return 0;
}