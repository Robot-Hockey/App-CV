#include <opencv2/opencv.hpp>
#include <opencv2/tracking/tracking.hpp>
#include <opencv2/core/ocl.hpp>
 
using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
 
int main(int argc, char **argv)
{
    // List of tracker types in OpenCV 3.4.1
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));
 
    // Create a tracker
    string trackerType = trackerTypes[1];
 
    Ptr<Tracker> tracker;
    Ptr<Tracker> tracker2;
 
    if (trackerType == "BOOSTING"){
        tracker = TrackerBoosting::create();
        tracker2 = TrackerBoosting::create();}
    if (trackerType == "MIL"){
        tracker = TrackerMIL::create();
        tracker2 = TrackerMIL::create();}
    if (trackerType == "KCF"){
        tracker = TrackerKCF::create();
        tracker2 = TrackerKCF::create();}
    if (trackerType == "TLD"){
        tracker = TrackerTLD::create();
        tracker2 = TrackerTLD::create();}
    if (trackerType == "MEDIANFLOW"){
        tracker = TrackerMedianFlow::create();
        tracker2 = TrackerMedianFlow::create();}
    if (trackerType == "GOTURN"){
        tracker = TrackerGOTURN::create();
        tracker2 = TrackerGOTURN::create();}
    if (trackerType == "MOSSE"){
        tracker = TrackerMOSSE::create();
        tracker2 = TrackerMOSSE::create();}
    if (trackerType == "CSRT"){
        tracker = TrackerCSRT::create();
        tracker2 = TrackerCSRT::create();}
    // Read video
    int deviceID = 2;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API

    VideoCapture video;

    video.open(deviceID + apiID);

    // int WIDTH = 360;
    // int HEIGHT = 360;

    // video.set(3, WIDTH);
    // video.set(4, HEIGHT);
     
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl; 
        return 1; 
    } 
 
    // Read first frame 
    Mat frame; 
    bool ok = video.read(frame); 
 
    // Define initial bounding box 
    Rect2d bbox, bbox2;
 
    // Uncomment the line below to select a different bounding box 
    bbox = selectROI(frame, false);
    // Display bounding box. 
    rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );

    bbox2 = selectROI(frame, false);
    // Display bounding box. 
    rectangle(frame, bbox2, Scalar( 0, 255, 0 ), 2, 1 );
 
    imshow("Tracking", frame); 
    tracker->init(frame, bbox);
    tracker2->init(frame, bbox2);
     
    while(video.read(frame))
    {     
        // Start timer
        double timer = (double)getTickCount();
         
        // Update the tracking result
        bool ok = tracker->update(frame, bbox);
        bool ok2 = tracker->update(frame, bbox2);
         
        // Calculate Frames per second (FPS)
        float fps = getTickFrequency() / ((double)getTickCount() - timer);
         
        if (ok && ok2)
        {
            // Tracking success : Draw the tracked object
            rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
            rectangle(frame, bbox2, Scalar( 0, 255, 0 ), 2, 1 );
        }
        else
        {
            // Tracking failure detected.
            putText(frame, "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
        }
         
        // Display tracker type on frame
        putText(frame, trackerType + " Tracker", Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50),2);
         
        // Display FPS on frame
        putText(frame, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
 
        // Display frame.
        imshow("Tracking", frame);
         
        // Exit if ESC pressed.
        int k = waitKey(1);
        if(k == 27)
        {
            break;
        }
 
    }
}