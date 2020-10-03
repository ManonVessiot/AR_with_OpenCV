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
            cout << numberRecorded << " input recorded / " << numberOfInput << endl;
        }
        
        runningLock.lock(); // Thread Lock
        bool stop = !cam.running;
        runningLock.unlock(); // Thread unLock

        if (stop){
            break;
        }
    }
    cout << calib.inputSize() << " input recorded / " << numberOfInput << " : DONE" << endl;
    
}

int main( int argc, char **argv ){
    // ./exc 0 9 6 0.025 0 30 1 calibration.txt Webcam
    if (argc < 9){
        cout << "./exc capIndex boardSize_Width boardSize_Height square_Size calibrate_Pattern numberOfInput secondsBetweenFrame calibrationFileName (windowsName)" << endl;
        cout << " --> capIndex identify your webcam" << endl;
        cout << " --> boardSize_Width, boardSize_Height is the size of your pattern" << endl;
        cout << " --> square_Size is the size of your pattern's square" << endl;
        cout << " --> calibrate_Pattern is the type of pattern" << endl;
        cout << "      -> CHESSBOARD = 0" << endl;
        cout << "      -> CIRCLES_GRID = 1" << endl;
        cout << "      -> ASYMMETRIC_CIRCLES_GRID = 2" << endl;
        cout << " --> numberOfInput is the number of frame to use for calibration" << endl;
        cout << " --> secondsBetweenFrame is the number of seconde between frame used" << endl;
        cout << " --> calibrationFileName is the name of the file, where the calibration is saved" << endl;
        cout << " --> windowsName is OPTIONNAL and is the name of the windows that shows the webcam frame" << endl;

        return -1;
    }

    int capIndex = atoi(argv[1]); // 0
    int boardSize_Width = atoi(argv[2]); // 9 or 6 or 11
    int boardSize_Height = atoi(argv[3]); // 6 or 6 or 4
    float square_Size = atof(argv[4]); // 0.025
    Pattern calibrate_Pattern = (Pattern) atoi(argv[5]); // CHESSBOARD = 0 or CIRCLES_GRID = 1 or ASYMMETRIC_CIRCLES_GRID = 2
    int numberOfInput = atoi(argv[6]); // 30
    int secondsBetweenFrame = atoi(argv[7]); // 1

    string calibrationFile = argv[8]; // "calibration.txt"

    string windowsName = "Live";
    if (argc >=10){
        windowsName = argv[9];
    }


    VideoCapture cap(capIndex);
    cam = Video(cap, windowsName);
    calib = CalibrationProcess(boardSize_Width, boardSize_Height, square_Size, calibrate_Pattern);

    // init for thread
    XInitThreads();

    cout << "start display" << endl;
    thread displayThread(display);
    cout << "start record" << endl;
    thread recordframeLockead(recordFrameForCalibrationProcess, numberOfInput, secondsBetweenFrame);

    recordframeLockead.join();
    cout << "stop record" << endl;

    if (calib.inputSize() == numberOfInput){
        calib.calibrate(cam.cameraMatrix, cam.distanceCoefficients);
        calib.saveCalibration(calibrationFile);
    }
    displayThread.join();
    cout << "stop display" << endl;

    return 0;
}
