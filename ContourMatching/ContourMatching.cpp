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

Mat src_gray, hsv, puck_bin, goalie_bin;
Mat src;

int thresh = 100;
int max_canny_thresh = 100;
RNG rng(12345);

int H_MIN_PUCK = 126;
int H_MAX_PUCK = 196;
int S_MIN_PUCK = 0;
int S_MAX_PUCK = 229;
int V_MIN_PUCK = 0;
int V_MAX_PUCK = 256;

int H_MIN_GOALIE = 85;
int H_MAX_GOALIE = 128;
int S_MIN_GOALIE = 143;
int S_MAX_GOALIE = 256;
int V_MIN_GOALIE = 103;
int V_MAX_GOALIE = 168;

const string puckTrackbarWindowName = "Puck Trackbars";
const string goalieTrackbarWindowName = "Goalie Trackbars";

int ATTACK_X = 500;
double MIN_DISTANCE_TO_POI = 100;
int MIN_X_DISTANCE_TO_POI = 25;
int MIN_Y_DISTANCE_TO_POI = 25;

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

    // if(should_move_goalie_x){
    //     if(x_clockwise) s+='1';
    //     else s+='2';
    // }else{
    //     s+='0';
    // }

    if(should_move_goalie_y){
        if(y_clockwise) s+='1';
        else s+='2';
    }else{
        s+='0';
    }

    // cout << s << endl;

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

void createPuckTrackbars(){
	//create window for trackbars
    namedWindow(puckTrackbarWindowName,0);
	//create memory to store trackbar name on window
	char PuckTrackbarName[50];
	sprintf( PuckTrackbarName, "H_MIN_PUCK", H_MIN_PUCK);
	sprintf( PuckTrackbarName, "H_MAX_PUCK", H_MAX_PUCK);
	sprintf( PuckTrackbarName, "S_MIN_PUCK", S_MIN_PUCK);
	sprintf( PuckTrackbarName, "S_MAX_PUCK", S_MAX_PUCK);
	sprintf( PuckTrackbarName, "V_MIN_PUCK", V_MIN_PUCK);
	sprintf( PuckTrackbarName, "V_MAX_PUCK", V_MAX_PUCK);
	  
    createTrackbar( "H_MIN_PUCK", puckTrackbarWindowName, &H_MIN_PUCK, 256 );
    createTrackbar( "H_MAX_PUCK", puckTrackbarWindowName, &H_MAX_PUCK, 256 );
    createTrackbar( "S_MIN_PUCK", puckTrackbarWindowName, &S_MIN_PUCK, 256 );
    createTrackbar( "S_MAX_PUCK", puckTrackbarWindowName, &S_MAX_PUCK, 256 );
    createTrackbar( "V_MIN_PUCK", puckTrackbarWindowName, &V_MIN_PUCK, 256 );
    createTrackbar( "V_MAX_PUCK", puckTrackbarWindowName, &V_MAX_PUCK, 256 );
}

void createGoalieTrackbars(){
	//create window for trackbars
    namedWindow(goalieTrackbarWindowName,0);
	//create memory to store trackbar name on window
	char GoalieTrackbarName[50];
	sprintf( GoalieTrackbarName, "H_MIN_GOALIE", H_MIN_GOALIE);
	sprintf( GoalieTrackbarName, "H_MAX_GOALIE", H_MAX_GOALIE);
	sprintf( GoalieTrackbarName, "S_MIN_GOALIE", S_MIN_GOALIE);
	sprintf( GoalieTrackbarName, "S_MAX_GOALIE", S_MAX_GOALIE);
	sprintf( GoalieTrackbarName, "V_MIN_GOALIE", V_MIN_GOALIE);
	sprintf( GoalieTrackbarName, "V_MAX_GOALIE", V_MAX_GOALIE);
	  
    createTrackbar( "H_MIN_GOALIE", goalieTrackbarWindowName, &H_MIN_GOALIE, 256 );
    createTrackbar( "H_MAX_GOALIE", goalieTrackbarWindowName, &H_MAX_GOALIE, 256 );
    createTrackbar( "S_MIN_GOALIE", goalieTrackbarWindowName, &S_MIN_GOALIE, 256 );
    createTrackbar( "S_MAX_GOALIE", goalieTrackbarWindowName, &S_MAX_GOALIE, 256 );
    createTrackbar( "V_MIN_GOALIE", goalieTrackbarWindowName, &V_MIN_GOALIE, 256 );
    createTrackbar( "V_MAX_GOALIE", goalieTrackbarWindowName, &V_MAX_GOALIE, 256 );
}

void findPuck(){
    /// Detect edges using Canny
    Mat canny_output;
    // cout << thresh << endl;
    Canny( puck_bin, canny_output, thresh, thresh*2 );

    /// Find contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );

    /// Draw contours

    double min_puck_area = 500;
    double max_puck_area = 2000;

    bool found = false;

    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size() && !found; i++ ){
        double area = cv::contourArea(contours[i]);
        drawContours( puck_bin, contours, (int)i, Scalar(0, 256, 0), 2, LINE_8, hierarchy, 0 );
        // cout << i << "Puck " << area << endl;

        if(area >= min_puck_area && area <= max_puck_area){
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

            line(src, curr, infinite, Scalar(0, 0, 256), 3);
            line(drawing, curr, infinite, Scalar(0, 0, 256), 3); // First part of trajectory (Red)

            if(reflects){
                line(src, reflection_point, infinite_reflection_point, Scalar(256, 0, 0), 3);
                line(drawing, reflection_point, infinite_reflection_point, Scalar(256, 0, 0), 3); // Reflected trajectory (Blue)
            }


            circle(src, curr, 1, Scalar(0, 0, 256), 5);
            circle(drawing, curr, 1, Scalar(0, 0, 256), 5);

            found = true;

            // drawContours( drawing, contours, (int)i, Scalar(0, 256, 0), 2, LINE_8, hierarchy, 0 );
        }
    }

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

    if(point_of_interest.y <= 0 || point_of_interest.y >= 480 || goalie.x < cx || infinite.x == 0){
        point_of_interest.y = 240;
        point_of_interest.x = ATTACK_X + 40;
    }

    circle(src, point_of_interest, 1, Scalar(256, 0, 256), 20);
    circle(drawing, point_of_interest, 1, Scalar(256, 0, 256), 20);

    // imshow( "Puck Contours", drawing );
}

void findGoalie(){
    Mat canny_output;
    // cout << thresh << endl;
    Canny( goalie_bin, canny_output, thresh, thresh*2 );

    /// Find contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );

    /// Draw contours

    double min_goalie_area = 200;
    double max_goalie_area = 3000;

    bool found = false;

    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size() && !found; i++ ){
        double area = cv::contourArea(contours[i]);
        drawContours( goalie_bin, contours, (int)i, Scalar(0, 256, 0), 2, LINE_8, hierarchy, 0 );
        // cout << i << "Goalie " << area << endl;
        
        if(area >= min_goalie_area && area <= max_goalie_area){
            Rect br = boundingRect(contours[i]);
            
            reb_x = br.x+br.width/2; 
            reb_y = br.y+br.height/2;

            goalie = Point(reb_x, reb_y);

            found = true;

            // drawContours( drawing, contours, (int)i, Scalar(0, 0, 256), 2, LINE_8, hierarchy, 0 );
        }
    }

    // cout << "=============" << endl;

    line(src, Point(ATTACK_X, 0), Point(ATTACK_X, 480), Scalar(0, 256, 0), 2);
    line(drawing, Point(ATTACK_X, 0), Point(ATTACK_X, 480), Scalar(0, 256, 0), 2); // Attack line (Green)

    line(src, point_of_interest, goalie, Scalar(256, 0, 256), 1);
    line(drawing, point_of_interest, goalie, Scalar(256, 0, 256), 1);

    circle(src, goalie, 1, Scalar(0, 256, 0), 60);
    circle(drawing, goalie, 1, Scalar(0, 256, 0), 60);

    // Move goalie
    double curr_dist = hypot(goalie.x - point_of_interest.x, goalie.y - point_of_interest.y);
    int x_diff = goalie.x - point_of_interest.x;
    int y_diff = goalie.y - point_of_interest.y;

    // Logic to set serial string

    // positive x -> moves anti clockwise, negative x -> moves clockwise
    if(x_diff > 0) x_clockwise = true;
    else x_clockwise = false;

    // positive y -> moves anti clockwise, negative y -> moves clockwise
    if(y_diff > 0) y_clockwise = false;
    else y_clockwise = true;

    // if(curr_dist > MIN_DISTANCE_TO_POI){
    //     putText(drawing, "DIST: " + SSTR(double(curr_dist)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(256,50,256), 2);
    // }else{
    //     putText(drawing, "DIST: " + SSTR(double(curr_dist)) + " OK", Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
    // }

    if(abs(x_diff) > MIN_X_DISTANCE_TO_POI){
        should_move_goalie_x = true;
        putText(drawing, "X_DIST: " + SSTR(int(x_diff)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(256,50,256), 2);
    }else{
        should_move_goalie_x = false;
        putText(drawing, "X_DIST: " + SSTR(int(x_diff)) + " OK", Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
    }

    if(abs(y_diff) > MIN_Y_DISTANCE_TO_POI){
        should_move_goalie_y = true;
        putText(drawing, "Y_DIST: " + SSTR(int(y_diff)), Point(100,100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(256,50,256), 2);
    }else{
        should_move_goalie_y = false;
        putText(drawing, "Y_DIST: " + SSTR(int(y_diff)) + " OK", Point(100,100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
    }

    /// Show in a window
    // imshow( "Goalie Contours", drawing );
}

int main( int argc, char** argv )
{
    pts_dst.push_back(Point2f(640, 480));
    pts_dst.push_back(Point2f(0, 480));
    pts_dst.push_back(Point2f(0, 0));
    pts_dst.push_back(Point2f(640, 0));

    pts_src.push_back(Point2f(12, 460));
    pts_src.push_back(Point2f(147, 28));
    pts_src.push_back(Point2f(412, 19));
    pts_src.push_back(Point2f(584, 441));

    /// Load source image
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API

    mySerial serial("/dev/ttyUSB0", 9600);

    VideoCapture cap;

    cap.open(deviceID + apiID);

    cap.set(CAP_PROP_FPS, 60);
    // cap.set(CAP_PROP_FPS, 120);
    // cap.set(CAP_PROP_FRAME_WIDTH,320);
    // cap.set(CAP_PROP_FRAME_HEIGHT,240);

    Mat im_src;

    // namedWindow("Surface", 1);

    // Rect r = Rect(100, 30, 200, 400);

    // setMouseCallback("Surface", CallBackFunc, NULL);

    // while(1){
    //     cap.read(im_src);
    //     imshow("Surface", im_src);
    //     if (waitKey(5) == 27)
    //         break;

    //     if(pts_src.size() >= 4 && pts_dst.size() >= 4){
    //         cap.read(im_src);
    //         break;
    //     }
    // }
 
    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);
 
    Mat tmpsrc;

    const char* source_window = "Source";
    namedWindow( source_window );

    createTrackbar( "Min Threshold:", source_window, &ATTACK_X, 640 );

    createPuckTrackbars();
    // createGoalieTrackbars();

    while(1){
        double timer = (double)getTickCount();
        cap >> tmpsrc;
        // cap >> src;

        // resize(tmpsrc, tmpsrc, Size(640, 480));

        warpPerspective(tmpsrc, src, h, tmpsrc.size());
        /// Convert image to gray and blur it
        cvtColor( src, src_gray, COLOR_BGR2GRAY );
        cvtColor( src, hsv, COLOR_BGR2HSV );

        GaussianBlur(hsv, hsv, Size(31, 31), 0);
        // GaussianBlur(hsv, hsv, Size(21, 21), 0);
        // imshow( "HSV", hsv );

        inRange(hsv, Scalar(H_MIN_PUCK, S_MIN_PUCK, V_MIN_PUCK), Scalar(H_MAX_PUCK, S_MAX_PUCK, V_MAX_PUCK), puck_bin);
        inRange(hsv, Scalar(H_MIN_GOALIE, S_MIN_GOALIE, V_MIN_GOALIE), Scalar(H_MAX_GOALIE, S_MAX_GOALIE, V_MAX_GOALIE), goalie_bin);

        imshow( "Puck Binary", puck_bin );
        // imshow( "Goalie Binary", goalie_bin );

        const int max_thresh = 255;
        findPuck();
        findGoalie();
        // Calculate Frames per second (FPS)
        fps = getTickFrequency() / ((double)getTickCount() - timer);
        putText(src, "FPS: " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        putText(src, "POINT: " + SSTR(int(cx)) + ", " + SSTR(int(cy)), Point(100,100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        putText(src, "DIFF M: " + SSTR(double(diff_m)), Point(100,200), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        imshow( source_window, src );

        // cout << get_serial_code() << endl;

        
        string tmp_serial_code = get_serial_code();

        if(tmp_serial_code != serial_code){
            serial_code = tmp_serial_code;
            serial.Send(serial_code);
        }

        if(waitKey(1)==27)break;
    }

    return 0;
}