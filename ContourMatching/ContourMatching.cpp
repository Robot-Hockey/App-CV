/**
 * @function findContours_Demo.cpp
 * @brief Demo code to find contours in an image
 * @author OpenCV team
 */

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include "mySerial.h"
#include "unistd.h"

using namespace cv;
using namespace std;

vector<Point2f> pts_src;
vector<Point2f> pts_dst;

string serial_code = "";

Mat src_gray, hsv, bin;
int thresh = 100;
RNG rng(12345);

int H_MIN = 4;
int H_MAX = 12;
int S_MIN = 221;
int S_MAX = 256;
int V_MIN = 71;
int V_MAX = 228;

int ATTACK_X = 500;
double MIN_DISTANCE_TO_POI = 100;
int MIN_X_DISTANCE_TO_POI = 30;
int MIN_Y_DISTANCE_TO_POI = 40;

bool should_move_goalie_x = false;
bool should_move_goalie_y = false;

bool y_clockwise = false;
bool x_clockwise = false;

Point previous = Point(0, 0);
Point infinite, reflection_point, infinite_reflection_point, point_of_interest, goalie;
double m, b, rm, rb;

double diff_m = 0;

double cx {0}, cy {0};
double ix {0}, iy {0};
double reb_x {0}, reb_y {0};

bool reflects;

float fps;

 // Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

double get_y(double m, double b, double x){
    return m*x + b;
}

double get_x(double m, double b, double y){
    return (y-b)/m;
}

string get_serial_code(){
    string s = "";

    if(should_move_goalie_x){
        if(x_clockwise) s+='1';
        else s+='2';
    }else{
        s+='0';
    }

    if(should_move_goalie_y){
        if(y_clockwise) s+='1';
        else s+='2';
    }else{
        s+='0';
    }

    return s;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata){
     if  ( event == EVENT_LBUTTONDOWN ){
          cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
          pts_src.push_back(Point2f(x, y));
     }
     else if  ( event == EVENT_RBUTTONDOWN ){
        //   cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        //   pts_dst.push_back(Point2f(x, y));
     }
     else if  ( event == EVENT_MBUTTONDOWN ){
        //   cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if ( event == EVENT_MOUSEMOVE ){
        //   cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

     }
}

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
    double max_area = 600;
    double intermediate_area = 270;

    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ ){
        double area = cv::contourArea(contours[i]);

        // cout << i << " " << area << endl;

        if(area >= intermediate_area && area <= max_area){
            Rect br = boundingRect(contours[i]);
            
            cx = br.x+br.width/2; 
            cy = br.y+br.height/2;

            Point curr = Point(cx, cy);

            double eps = 1.8;

            double den = (infinite.x - curr.x);
            if(fabs(den) < 1e-9){
                den = 0.001;
            }

            double temp_m = (infinite.y - curr.y)/den;

            diff_m = fabs(fabs(temp_m) - fabs(m));

            // cout << temp_m << " " << m << " " << diff_m << endl;

            if(diff_m > 0.008){
                // Makes line algorithm (y = mx + b)
                double den = (previous.x - curr.x);
                if(fabs(den) < 1e-9){
                    den = 0.001;
                }

                m = (previous.y - curr.y)/den;;
                b = (curr.y - m*curr.x);

                double infinite_x;

                if(previous.x < curr.x){
                    // Moving to the right of the screen
                    infinite_x = 640;
                }else{
                    // Moving to the left of the screen
                    infinite_x = 0;
                }

                
                double infinite_y = get_y(m, b, infinite_x);

                infinite = Point(infinite_x, infinite_y);
                ix = infinite.x;
                iy = infinite.y;
                // ...

                if(infinite.y <= 0 || infinite.y >= 480){
                    reflects = true;

                    double reflected_y;
                    if(infinite.y <= 0){
                        reflected_y = 0;
                    }else{
                        reflected_y = 480;
                    }
                    
                    reflection_point = Point(
                        get_x(m, b, reflected_y),
                        reflected_y
                    );

                    rm = -m;
                    rb = (reflection_point.y - rm*reflection_point.x);

                    infinite_reflection_point = Point(
                        infinite.x,
                        get_y(rm, rb, infinite.x)
                    );
                    
                }else{
                    reflects = false;
                }

                previous = curr;
            }

            line(drawing, curr, infinite, Scalar(0, 0, 256), 3); // First part of trajectory (Red)

            if(reflects){
                line(drawing, reflection_point, infinite_reflection_point, Scalar(256, 0, 0), 3); // Reflected trajectory (Blue)
            }

            line(drawing, Point(ATTACK_X, 0), Point(ATTACK_X, 480), Scalar(0, 256, 0), 2); // Attack line (Green)

            circle(drawing, curr, 1, Scalar(0, 0, 256), 5);

            drawContours( drawing, contours, (int)i, Scalar(0, 256, 0), 2, LINE_8, hierarchy, 0 );
        }else if(area >= min_area && area < intermediate_area){
            Rect br = boundingRect(contours[i]);
            
            reb_x = br.x+br.width/2; 
            reb_y = br.y+br.height/2;

            goalie = Point(reb_x, reb_y);

            circle(drawing, goalie, 1, Scalar(0, 256, 0), 60);

            // drawContours( drawing, contours, (int)i, Scalar(0, 0, 256), 2, LINE_8, hierarchy, 0 );
        }
    }

    // cout << "=============" << endl;


    // Point of interest
    double interest_m, interest_b;

    if(reflects && (reflection_point.x <= ATTACK_X)){
        interest_m = rm;
        interest_b = rb;
    }else{
        interest_m = m;
        interest_b = b;
    }

    point_of_interest = Point(
        ATTACK_X,
        get_y(interest_m, interest_b, ATTACK_X)
    );

    circle(drawing, point_of_interest, 1, Scalar(256, 0, 256), 20);

    line(drawing, point_of_interest, goalie, Scalar(256, 0, 256), 1);

    // Move goalie

    double curr_dist = hypot(goalie.x - point_of_interest.x, goalie.y - point_of_interest.y);
    int x_diff = goalie.x - point_of_interest.x;
    int y_diff = goalie.y - point_of_interest.y;

    // Logic to set serial string

    // positive x -> moves anti clockwise, negative x -> moves clockwise
    if(x_diff > 0) x_clockwise = true;
    else x_clockwise = false;

    // positive y -> moves anti clockwise, negative y -> moves clockwise
    if(y_diff > 0) y_clockwise = true;
    else y_clockwise = false;

    // if(curr_dist > MIN_DISTANCE_TO_POI){
    //     putText(drawing, "DIST: " + SSTR(double(curr_dist)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(256,50,256), 2);
    // }else{
    //     putText(drawing, "DIST: " + SSTR(double(curr_dist)) + " OK", Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
    // }

    if(x_diff > MIN_X_DISTANCE_TO_POI){
        should_move_goalie_x = true;
        putText(drawing, "X_DIST: " + SSTR(int(x_diff)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(256,50,256), 2);
    }else{
        should_move_goalie_x = false;
        putText(drawing, "X_DIST: " + SSTR(int(x_diff)) + " OK", Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
    }

    if(y_diff > MIN_Y_DISTANCE_TO_POI){
        should_move_goalie_y = true;
        putText(drawing, "Y_DIST: " + SSTR(int(y_diff)), Point(100,100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(256,50,256), 2);
    }else{
        should_move_goalie_y = false;
        putText(drawing, "Y_DIST: " + SSTR(int(y_diff)) + " OK", Point(100,100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
    }

    /// Show in a window
    imshow( "Contours", drawing );
}

int main( int argc, char** argv )
{
    pts_dst.push_back(Point2f(0, 0));
    pts_dst.push_back(Point2f(480, 0));
    pts_dst.push_back(Point2f(480, 640));
    pts_dst.push_back(Point2f(0, 640));

    /// Load source image
    Mat src;
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API

    // mySerial serial("/dev/stdout", 9600);

    VideoCapture cap;

    cap.open(deviceID + apiID);

    cap.set(CAP_PROP_FPS, 60);
    // cap.set(CAP_PROP_FPS, 120);
    // cap.set(CAP_PROP_FRAME_WIDTH,320);
    // cap.set(CAP_PROP_FRAME_HEIGHT,240);

    Mat im_src;

    namedWindow("Surface", 1);

    Rect r = Rect(100, 30, 200, 400);

    setMouseCallback("Surface", CallBackFunc, NULL);

    while(1){
        cap.read(im_src);
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
 
    Mat tmpsrc;

    while(1){
        double timer = (double)getTickCount();
        cap >> tmpsrc;

        warpPerspective(tmpsrc, src, h, tmpsrc.size());
        /// Convert image to gray and blur it
        cvtColor( src, src_gray, COLOR_BGR2GRAY );
        cvtColor( src, hsv, COLOR_BGR2HSV );

        /// Create Window
        const char* source_window = "Source";
        namedWindow( source_window );
        GaussianBlur(hsv, hsv, Size(31, 31), 0);
        // imshow( "HSV", hsv );

        inRange(hsv,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),bin);

        // imshow( "Binary", bin );

        const int max_thresh = 255;
        thresh_callback( 0, 0 );
        // Calculate Frames per second (FPS)
        fps = getTickFrequency() / ((double)getTickCount() - timer);
        putText(src, "FPS: " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        putText(src, "POINT: " + SSTR(int(cx)) + ", " + SSTR(int(cy)), Point(100,100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        putText(src, "DIFF M: " + SSTR(double(diff_m)), Point(100,200), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        imshow( source_window, src );

        // cout << get_serial_code() << endl;

        
        // string tmp_serial_code = get_serial_code();

        // if(tmp_serial_code != serial_code){
        //     serial_code = tmp_serial_code;
        //     serial.Send(serial_code);
        // }

        if(waitKey(1)==27)break;
    }

    return 0;
}