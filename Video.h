#ifndef DEF_VIDEO
#define DEF_VIDEO

#include "opencv.h"

class Video {
    protected:
        string windowsName;
        VideoCapture cap;
        Mat frame;


    public:
        Mat cameraMatrix;
        Mat distanceCoefficients;

        bool running;

        Video();
        Video(VideoCapture &_cap, string _windowsName);
        ~Video();

        Mat& getFrame();
        void updateFrame();

        string getWindowsName();
        void setDefaultCalibration(double width, double height);
};
#endif
