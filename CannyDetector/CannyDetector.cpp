/**
 * @file CannyDetector_Demo.cpp
 * @brief Sample code showing how to detect edges using the Canny Detector
 * @author OpenCV team
 */

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;

//![variables]
Mat src, src_gray;
Mat dst, detected_edges;

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int ratio = 3;
const int kernel_size = 3;
const char* window_name = "Edge Map";
//![variables]

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
static void CannyThreshold(int, void*){
    //![reduce_noise]
    /// Reduce noise with a kernel 3x3
    blur( src_gray, detected_edges, Size(3,3) );
    //![reduce_noise]

    //![canny]
    /// Canny detector
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
    //![canny]

    /// Using Canny's output as a mask, we display our result
    //![fill]
    dst = Scalar::all(0);
    //![fill]

    //![copyto]
    src.copyTo( dst, detected_edges);
    //![copyto]

    //![display]
    imshow( window_name, dst );
    //![display]
}


/**
 * @function main
 */
int main( int argc, char** argv ){
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API

    VideoCapture cap;
    cap.open(deviceID + apiID);
    cap.set(CAP_PROP_FPS, 60);

    cap >> src;

    //![create_mat]
    /// Create a matrix of the same type and size as src (for dst)
    dst.create( src.size(), src.type() );
    //![create_mat]

    //![convert_to_gray]
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    //![convert_to_gray]

    //![create_window]
    namedWindow( window_name, WINDOW_AUTOSIZE );
    //![create_window]

    //![create_trackbar]
    /// Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );
    //![create_trackbar]

    /// Show the image
    CannyThreshold(0, 0);

    /// Wait until user exit program by pressing a key
    waitKey(0);

    return 0;
}