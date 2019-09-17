#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
using namespace cv;
using namespace std;
int main(int, char**)
{
    Mat frame;
    VideoCapture cap;
    // open the default camera using default API
    // cap.open(0);
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    cap.open(deviceID + apiID);
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    cout << "Start grabbing" << endl
        << "Press any key to terminate" << endl;
    for (;;)
    {
        cap.read(frame);
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        imshow("Live", frame);
        if (waitKey(5) == 27)
            break;
    }
    return 0;
}