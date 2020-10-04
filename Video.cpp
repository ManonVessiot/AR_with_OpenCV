#include "Video.h"

Video::Video(){
    calibrated = false;
}

Video::Video(VideoCapture &_cap, string _windowsName){
    calibrated = false;
    cap = _cap;
    windowsName = _windowsName;
    if(!cap.isOpened()){  // check if we succeeded
  	   cout << "erreur acces video" << endl << endl;
    }
}

Video::~Video(){
    cap.release();
}

Mat& Video::getFrame(){
    return frame;
}

void Video::updateFrame(){
    cap >> frame;
}

string Video::getWindowsName(){
    return windowsName;
}

void Video::setDefaultCalibration(int width, int height){
    cameraMatrix = Mat::zeros(3, 3, CV_64F);

    cameraMatrix.at<double>(0, 0) = width;
    cameraMatrix.at<double>(1, 1) = height;
    cameraMatrix.at<double>(2, 2) = 1;

    cameraMatrix.at<double>(0, 2) = width / 2.0;
    cameraMatrix.at<double>(1, 2) = height / 2.0;

    distanceCoefficients = Mat::zeros(8, 1, CV_64F);
}