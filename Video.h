#ifndef DEF_VIDEO
#define DEF_VIDEO

#include "opencv.h"

class Video {
    private:
        string windowsName;
        VideoCapture cap;
        Mat frame;

    public:
        Video(){};
        Video(VideoCapture &_cap, string _windowsName);
        ~Video();

        Mat& getFrame();
        void updateFrame();

        string getWindowsName();
};
#endif
