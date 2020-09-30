#include "Video.h"

Video::Video(VideoCapture &_cap, string _windowsName){
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