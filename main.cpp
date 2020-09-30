#include "Video.h"
#include "Calibration.h"
#include "main.h"
#include <thread>

Video cam;
Calibration calib;

mutex frameLock;

void display()
{
    while (true)
    {
        frameLock.lock(); // Thread Lock
        cam.updateFrame();
        cv::imshow(cam.getWindowsName(), cam.getFrame());
        frameLock.unlock(); // Thread unLock
        
        if (waitKey(5) >= 0)
            break; 
    }
}

void recordFrameForCalibration(int numberOfInput, int secondsBetweenFrame)
{
    int numberRecorded = 0;
    while (numberRecorded < numberOfInput)
    {
        sleep(secondsBetweenFrame);
        
        frameLock.lock(); // Thread Lock
        if (calib.addInput(cam.getFrame())){
            numberRecorded++;
            cout << numberRecorded << " input recorded" << endl;
        }
        frameLock.unlock(); // Thread unLock
    }
    cout << calib.inputSize() << " input recorded : DONE" << endl;
    
}

int main( int argc, char **argv ){
    int capIndex = 0;
    int boardSize_Width = 9; // 9 or 6 or 11
    int boardSize_Height = 6; // 6 or 6 or 4
    float square_Size = 0.025;
    Pattern calibrate_Pattern = CHESSBOARD; // CHESSBOARD or CIRCLES_GRID or ASYMMETRIC_CIRCLES_GRID
    int numberOfInput = 5;
    int secondsBetweenFrame = 1;





    VideoCapture cap(capIndex);
    cam = Video(cap, "Live");
    calib = Calibration(boardSize_Width, boardSize_Height, square_Size, calibrate_Pattern);

    cout << "start display" << endl;
    thread displayThread(display);
    cout << "start record" << endl;
    thread recordframeLockead(recordFrameForCalibration, numberOfInput, secondsBetweenFrame);

    recordframeLockead.join();
    cout << "stop record" << endl;
    calib.calibrate();

    displayThread.join();
    cout << "stop display" << endl;

    return 0;
}
