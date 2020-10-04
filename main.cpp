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
    bool stop = false;
    while (!stop)
    {
        frameLock.lock(); // Thread Lock
        cam.updateFrame();
        cv::imshow(cam.getWindowsName(), cam.getFrame());
        frameLock.unlock(); // Thread unLock
        
        runningLock.lock(); // Thread Lock
        stop = !cam.running;
        runningLock.unlock(); // Thread unLock
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
    runningLock.lock(); // Thread Lock
    cam.running = false;
    runningLock.unlock(); // Thread unLock   
}

bool saveCalibration(int numberOfInput, int secondsBetweenFrame, string calibrationFile){
    recordFrameForCalibrationProcess(numberOfInput, secondsBetweenFrame);

    if (calib.inputSize() == numberOfInput){
        calib.calibrate(cam.cameraMatrix, cam.distanceCoefficients);
        calib.saveCalibration(calibrationFile);
        return true;
    }
    return false;
}

bool loadCalibration(string calibrationFile){
    ifstream inStream(calibrationFile);
    if (inStream){
        uint16_t rows;
        uint16_t columns;

        // cameraMatrix
        inStream >> rows;
        inStream >> columns;

        cam.cameraMatrix = Mat(Size(columns, rows), CV_64F);
        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < columns; c++)
            {
                double read = 0.0f;
                inStream >> read;
                cam.cameraMatrix.at<double>(r, c) = read;
                cout << cam.cameraMatrix.at<double>(r, c) << endl;
            }            
        }

        // distanceCoefficients
        inStream >> rows;
        inStream >> columns;

        cam.distanceCoefficients = Mat::zeros(columns, rows, CV_64F);
        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < columns; c++)
            {
                double read = 0.0f;
                inStream >> read;
                cam.distanceCoefficients.at<double>(r, c) = read;
                cout << cam.distanceCoefficients.at<double>(r, c) << endl;
            }            
        }
        inStream.close();

        return true;
    }
    return false;
}

bool detectionArucoMarker(float arucoSquareDimention){
    bool stop = false;

    vector<int> markerIds;
    Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
    vector<vector<Point2f>> markerCorners, rejectedCandidates;

    vector<Vec3d> rotationVectors, translationVectors;

    while (!stop){

        frameLock.lock(); // Thread Lock
        Mat frame = cam.getFrame();
        frameLock.unlock(); // Thread unLock

        aruco:: detectMarkers(frame, markerDictionary, markerCorners, markerIds);
        aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimention, cam.cameraMatrix, cam.distanceCoefficients,
                                        rotationVectors, translationVectors);
        
        cout << "markerIds.size() : " << markerIds.size() << endl;
        for (int i = 0; i < markerIds.size(); i++)
        {
            aruco::drawAxis(frame, cam.cameraMatrix, cam.distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);            
        }
        imshow("markerAxis", frame);

        runningLock.lock(); // Thread Lock
        stop = !cam.running;
        runningLock.unlock(); // Thread unLock
    }

    return true;
}

int main( int argc, char **argv ){
    // ./exc 0 0 9 6 0.025 0 30 1 calibration.txt Webcam
    if (argc < 10){
        cout << "./exc mode capIndex boardSize_Width boardSize_Height square_Size calibrate_Pattern numberOfInput secondsBetweenFrame calibrationFileName (windowsName)" << endl;
        cout << " --> mode :" << endl;
        cout << "      -> Load Calibration = 0" << endl;
        cout << "      -> Save Calibration = 1" << endl;
        cout << " --> capIndex identify your webcam" << endl;
        cout << " --> boardSize_Width, boardSize_Height is the size of your pattern" << endl;
        cout << " --> square_Size is the size of your pattern's square" << endl;
        cout << " --> calibrate_Pattern is the type of pattern :" << endl;
        cout << "      -> CHESSBOARD = 0" << endl;
        cout << "      -> CIRCLES_GRID = 1" << endl;
        cout << "      -> ASYMMETRIC_CIRCLES_GRID = 2" << endl;
        cout << " --> numberOfInput is the number of frame to use for calibration" << endl;
        cout << " --> secondsBetweenFrame is the number of seconde between frame used" << endl;
        cout << " --> calibrationFileName is the name of the file, where the calibration is saved" << endl;
        cout << " --> windowsName is OPTIONNAL and is the name of the windows that shows the webcam frame" << endl;

        return -1;
    }

    // variables
    int mode = atoi(argv[1]);
    int capIndex = atoi(argv[2]); // 0
    int boardSize_Width = atoi(argv[3]); // 9 or 6 or 11
    int boardSize_Height = atoi(argv[4]); // 6 or 6 or 4
    float square_Size = atof(argv[5]); // 0.025
    Pattern calibrate_Pattern = (Pattern) atoi(argv[6]); // CHESSBOARD = 0 or CIRCLES_GRID = 1 or ASYMMETRIC_CIRCLES_GRID = 2
    int numberOfInput = atoi(argv[7]); // 30
    int secondsBetweenFrame = atoi(argv[8]); // 1

    string calibrationFile = argv[9]; // "calibration.txt"

    string windowsName = "Live";
    if (argc >=11){
        windowsName = argv[10];
    }
    VideoCapture cap(capIndex);
    cam = Video(cap, windowsName);
    calib = CalibrationProcess(boardSize_Width, boardSize_Height, square_Size, calibrate_Pattern);


    // init for thread
    XInitThreads();


    // start display video capture
    cout << "start display..." << endl;
    thread displayThread(display);


    // calibration
    thread calibrationThread;
    
    if (mode == 0){
        cout << "loading calibration..." << endl;
        calibrationThread = thread(loadCalibration, calibrationFile);
    }
    else if (mode == 1){
        cout << "saving calibration..." << endl;
        calibrationThread = thread(saveCalibration, numberOfInput, secondsBetweenFrame, calibrationFile);
    }
    calibrationThread.join();

    float arucoSquareDimention = 0.125f;
    thread arucoMarkerThread(detectionArucoMarker, arucoSquareDimention);

    arucoMarkerThread.join();
    displayThread.join();

    return 0;
}
