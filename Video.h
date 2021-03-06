#ifndef DEF_VIDEO
#define DEF_VIDEO

#include "opencv.h"

class Video {
    protected:
        string windowsName;
        VideoCapture cap;
        Mat frame;


    public:
        bool calibrated;
        Mat cameraMatrix;
        Mat distanceCoefficients;
        
        int width;
        int height;

        bool running;

        Video();
        Video(VideoCapture &_cap, string _windowsName);
        ~Video();

        Mat& getFrame();
        void updateFrame();

        string getWindowsName();
        void setDefaultCalibration();
};
#endif
