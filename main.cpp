#include "Video.h"
#include "CalibrationProcess.h"
#include "main.h"
#include <thread>
#include <X11/Xlib.h>   

Video cam;
CalibrationProcess calib;

mutex frameLock;
mutex runningLock;

void display()
{
    runningLock.lock(); // Thread Lock
    cam.running = true;
    runningLock.unlock(); // Thread unLock
    while (true)
    {
        frameLock.lock(); // Thread Lock
        cam.updateFrame();
        cv::imshow(cam.getWindowsName(), cam.getFrame());
        frameLock.unlock(); // Thread unLock
        
        if (waitKey(5) >= 0){
            break;
        }
    }
    runningLock.lock(); // Thread Lock
    cam.running = false;
    runningLock.unlock(); // Thread unLock
}

void recordFrameForCalibrationProcess(int numberOfInput, int secondsBetweenFrame)
{
    int numberRecorded = 0;
    while (numberRecorded < numberOfInput)
    {
        sleep(secondsBetweenFrame);
        
        frameLock.lock(); // Thread Lock
        bool frameAdded = calib.addInput(cam.getFrame());
        frameLock.unlock(); // Thread unLock
        if (frameAdded){
            numberRecorded++;
            cout << numberRecorded << " input recorded" << endl;
        }
        
        runningLock.lock(); // Thread Lock
        bool stop = !cam.running;
        runningLock.unlock(); // Thread unLock

        if (stop){
            break;
        }
    }
    cout << calib.inputSize() << " input recorded : DONE" << endl;
    
}

int main( int argc, char **argv ){
    int capIndex = 0;
    int boardSize_Width = 9; // 9 or 6 or 11
    int boardSize_Height = 6; // 6 or 6 or 4
    float square_Size = 0.025;
    Pattern calibrate_Pattern = CHESSBOARD; // CHESSBOARD or CIRCLES_GRID or ASYMMETRIC_CIRCLES_GRID
    int numberOfInput = 30;
    int secondsBetweenFrame = 1;

    string calibrationFile = "calibration.txt";


    VideoCapture cap(capIndex);
    cam = Video(cap, "Live");
    calib = CalibrationProcess(boardSize_Width, boardSize_Height, square_Size, calibrate_Pattern);

    // init for thread
    XInitThreads();

    cout << "start display" << endl;
    thread displayThread(display);
    cout << "start record" << endl;
    thread recordframeLockead(recordFrameForCalibrationProcess, numberOfInput, secondsBetweenFrame);

    recordframeLockead.join();
    cout << "stop record" << endl;
    calib.calibrate(cam.cameraMatrix, cam.distanceCoefficients);
    calib.saveCalibration(calibrationFile);

    displayThread.join();
    cout << "stop display" << endl;

    return 0;
}
