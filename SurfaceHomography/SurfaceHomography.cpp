#include "opencv2/opencv.hpp" 
 
using namespace cv;
using namespace std;
 
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

vector<Point2f> pts_src;
vector<Point2f> pts_dst;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
     if  ( event == EVENT_LBUTTONDOWN )
     {
          cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
          pts_src.push_back(Point2f(x, y));
     }
     else if  ( event == EVENT_RBUTTONDOWN )
     {
          cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
          pts_dst.push_back(Point2f(x, y));
     }
     else if  ( event == EVENT_MBUTTONDOWN )
     {
        //   cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if ( event == EVENT_MOUSEMOVE )
     {
        //   cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

     }
}

int main( int argc, char** argv)
{
    // Read source image.
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API

    VideoCapture cap;

    cap.open(deviceID + apiID);
    cap.set(CAP_PROP_FPS, 120);
    // cap.set(CAP_PROP_FRAME_WIDTH,320);
    // cap.set(CAP_PROP_FRAME_HEIGHT,240);
 
    Mat im_src;

    namedWindow("Surface", 1);


    Rect r = Rect(100, 30, 200, 400);

    setMouseCallback("Surface", CallBackFunc, NULL);

    while(1){
        cap.read(im_src);
        rectangle(im_src,r,Scalar(255,0,0),3,8,0);
        imshow("Surface", im_src);
        if (waitKey(5) == 27)
            break;

        if(pts_src.size() >= 4 && pts_dst.size() >= 4){
            cap.read(im_src);
            break;
        }
    }
 
    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);
 
    // Output image
    Mat im_out;
    // Warp source image to destination based on homography
    warpPerspective(im_src, im_out, h, im_src.size());
 
    // Display images
    imshow("Source Image", im_src);
    // imshow("Destination Image", im_dst);
    imshow("Warped Source Image", im_out);
 
    waitKey(0);
}