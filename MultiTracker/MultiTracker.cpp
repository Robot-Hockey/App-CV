/*----------------------------------------------
  * Usage:
  * example_tracking_multitracker <video_name> [algorithm]
  *
  * example:
  * example_tracking_multitracker Bolt/img/%04d.jpg
  * example_tracking_multitracker faceocc2.webm KCF
  *--------------------------------------------------*/
 
 #include <opencv2/core/utility.hpp>
 #include <opencv2/tracking.hpp>
 #include <opencv2/videoio.hpp>
 #include <opencv2/highgui.hpp>
 #include <iostream>
 #include <cstring>
 #include <ctime>
 #include "samples_utility.hpp"
 
 using namespace std;
 using namespace cv;

int H_MIN = 83;
int H_MAX = 119;
int S_MIN = 126;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 83;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;
//names that will appear at the top of each window
const string trackbarWindowName = "Trackbars";

void on_trackbar( int, void* )
{//This function gets called whenever a
	// trackbar position is changed

}

void createTrackbars(){
	//create window for trackbars


    namedWindow(trackbarWindowName,0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN", H_MIN);
	sprintf( TrackbarName, "H_MAX", H_MAX);
	sprintf( TrackbarName, "S_MIN", S_MIN);
	sprintf( TrackbarName, "S_MAX", S_MAX);
	sprintf( TrackbarName, "V_MIN", V_MIN);
	sprintf( TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );


}

 // Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
 
 int main( int argc, char** argv ){
   // show help
  //  if(argc<2){
  //    cout<<
  //      " Usage: example_tracking_multitracker <video_name> [algorithm]\n"
  //      " examples:\n"
  //      " example_tracking_multitracker Bolt/img/%04d.jpg\n"
  //      " example_tracking_multitracker faceocc2.webm MEDIANFLOW\n"
  //      << endl;
  //    return 0;
  //  }
 
   // set the default tracking algorithm
   std::string trackingAlg = "KCF";
 
   // set the tracking algorithm from parameter
  //  if(argc>2)
  //    trackingAlg = argv[2];
 
   // create the tracker
   MultiTracker trackers;
 
   // container of the tracked objects
   vector<Rect2d> objects;
 
   // set input video
  //  std::string video = argv[1];
  int deviceID = 2;             // 0 = open default camera
  int apiID = cv::CAP_ANY;      // 0 = autodetect default API

  VideoCapture cap;

  cap.open(deviceID + apiID);
  cap.set(CAP_PROP_FPS, 120);
  cap.set(CAP_PROP_FRAME_WIDTH,320);
  cap.set(CAP_PROP_FRAME_HEIGHT,240);
 
   Mat frame, threshold;

   createTrackbars();

  cout << "SALVE" << endl;

   // get bounding box
   cap >> frame;
   cvtColor(frame, frame, COLOR_BGR2HSV);
   inRange(frame,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
   vector<Rect> ROIs;
   selectROIs("tracker",threshold,ROIs);
 
   //quit when the tracked object(s) is not provided
   if(ROIs.size()<1)
     return 0;
 
   // initialize the tracker
   std::vector<Ptr<Tracker> > algorithms;
   for (size_t i = 0; i < ROIs.size(); i++)
   {
       algorithms.push_back(createTrackerByName(trackingAlg));
       objects.push_back(ROIs[i]);
     }
 
   trackers.add(algorithms,threshold,objects);
 
   // do the tracking
   printf("Start the tracking process, press ESC to quit.\n");
   for ( ;; ){

     // Start timer
    double timer = (double)getTickCount();

     // get frame from the video
     cap >> frame;

     cvtColor(frame, frame, COLOR_BGR2HSV);
     inRange(frame,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
 
     // stop the program if no more images
     if(frame.rows==0 || frame.cols==0)
       break;
 
     //update the tracking result
     trackers.update(threshold);

     // Calculate Frames per second (FPS)
      float fps = getTickFrequency() / ((double)getTickCount() - timer);
 
     // draw the tracked object
     for(unsigned i=0;i<trackers.getObjects().size();i++)
       rectangle( threshold, trackers.getObjects()[i], Scalar( 255, 0, 0 ), 2, 1 );
 
    putText(frame, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);

     // show image with the tracked object
     imshow("tracker",frame);
     imshow("binary",threshold);
 
     //quit on ESC button
     if(waitKey(1)==27)break;
   }
 
 }