/**
 * @function findContours_Demo.cpp
 * @brief Demo code to find contours in an image
 * @author OpenCV team
 */

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

using namespace cv;
using namespace std;

Mat src_gray, hsv, bin;
int thresh = 100;
RNG rng(12345);

// int H_MIN = 83;
// int H_MAX = 119;
// int S_MIN = 126;
// int S_MAX = 256;
// int V_MIN = 0;
// int V_MAX = 83;


int H_MIN = 4;
int H_MAX = 12;
int S_MIN = 221;
int S_MAX = 256;
int V_MIN = 71;
int V_MAX = 228;

 // Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

/// Function header
void thresh_callback(int, void* );

/**
 * @function main
 */
int main( int argc, char** argv )
{
    /// Load source image
    Mat src;
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API

    VideoCapture cap;

    cap.open(deviceID + apiID);

    cap.set(CAP_PROP_FPS, 60);

    while(1){
        double timer = (double)getTickCount();
        cap >> src;
        /// Convert image to gray and blur it
        cvtColor( src, src_gray, COLOR_BGR2GRAY );
        cvtColor( src, hsv, COLOR_BGR2HSV );

        /// Create Window
        const char* source_window = "Source";
        namedWindow( source_window );
        GaussianBlur(hsv, hsv, Size(31, 31), 0);
        imshow( "HSV", hsv );

        inRange(hsv,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),bin);

        const int max_thresh = 255;
        thresh_callback( 0, 0 );
        // Calculate Frames per second (FPS)
        float fps = getTickFrequency() / ((double)getTickCount() - timer);
        putText(src, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        imshow( source_window, src );


        if(waitKey(1)==27)break;
    }

    return 0;
}

/**
 * @function thresh_callback
 */
void thresh_callback(int, void* )
{
    /// Detect edges using Canny
    Mat canny_output;
    Canny( bin, canny_output, thresh, thresh*2 );

    /// Find contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );

    /// Draw contours

    double min_area = 100;
    double max_area = 100;

    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ ){
        double area = cv::contourArea(contours[i]);
        if(area >= min_area){
            Rect br = boundingRect(contours[i]);
            
            double cx = br.x+br.width/2; 
            double cy = br.y+br.height/2; 

            circle(drawing, Point(cx, cy), 1, Scalar(0, 0, 256), 3);

            drawContours( drawing, contours, (int)i, Scalar(0, 256, 0), 2, LINE_8, hierarchy, 0 );
        }
    }

    /// Show in a window
    imshow( "Contours", drawing );
}