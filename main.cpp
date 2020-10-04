#include "Video.h"
#include "CalibrationProcess.h"
#include "main.h"
#include "opengl.h"
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
        //*
        uint16_t rows;
        uint16_t columns;

        // cameraMatrix
        inStream >> rows;
        inStream >> columns;

        cam.cameraMatrix = Mat::zeros(rows, columns, CV_64F);
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

        cam.distanceCoefficients = Mat::zeros(rows, columns, CV_64F);
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
        cam.calibrated = true;
        //*/
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
        if (!frame.empty()){
            aruco:: detectMarkers(frame, markerDictionary, markerCorners, markerIds);
            aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimention, cam.cameraMatrix, cam.distanceCoefficients,
                                            rotationVectors, translationVectors);
            
            for (int i = 0; i < markerIds.size(); i++)
            {
                aruco::drawAxis(frame, cam.cameraMatrix, cam.distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);            
            }
            imshow("Aruco Marker Axis", frame);
        }
        else
        {
            stop = true;
        }        

        runningLock.lock(); // Thread Lock
        stop = stop || !cam.running;
        runningLock.unlock(); // Thread unLock
    }

    return true;
}

void printManual(){
    /*
    if (mode == 0) // Loading
        mode
        calibrationFileName

    if (mode == 1) // Saving
        mode
        calibrationFileName
        capIndex
        boardSize_Width, boardSize_Height
        square_Size
        calibrate_Pattern
        numberOfInput
        secondsBetweenFrame

    if (mode == 2) // Loading and Detect Aruco
        mode
        calibrationFileName
        capIndex
        arucoSquareDimention
    */
    cout << " VARIABLES :" << endl;
    cout << endl;
    cout << " --> mode :" << endl;
    cout << "      -> Load Calibration = 0" << endl;
    cout << "      -> Save Calibration = 1" << endl;
    cout << "      -> Load and Detect Aruco Markers = 2" << endl;
    cout << " --> calibrationFileName is the name of the file, where the calibration is saved" << endl;
    cout << " --> capIndex identify your webcam (0)" << endl;
    cout << " --> boardSize_Width, boardSize_Height is the size of your pattern (9, 6 ou 6, 6 ou 11, 4)" << endl;
    cout << " --> square_Size is the size of your pattern's square (0.025)" << endl;
    cout << " --> calibrate_Pattern is the type of pattern :" << endl;
    cout << "      -> CHESSBOARD = 0" << endl;
    cout << "      -> CIRCLES_GRID = 1" << endl;
    cout << "      -> ASYMMETRIC_CIRCLES_GRID = 2" << endl;
    cout << " --> numberOfInput is the number of frame to use for calibration (30)" << endl;
    cout << " --> secondsBetweenFrame is the number of seconde between frame used (1)" << endl;
    cout << " --> arucoSquareDimention is is the size of the aruco's markers, which is a square (0.125)" << endl;
    cout << endl;
    cout << "Load Calibration = 0 : " << endl;
    cout << "$ ./exc mode calibrationFileName capIndex" << endl;
    cout << "$ ./exc 0 calibration.txt 0" << endl;
    cout << endl;
    cout << "Save Calibration = 1 : " << endl;
    cout << "$ ./exc mode calibrationFileName capIndex boardSize_Width boardSize_Height square_Size calibrate_Pattern numberOfInput secondsBetweenFrame" << endl;
    cout << "$ ./exc 1 calibration.txt 0 9 6 0.025 0 30 1" << endl;
    cout << endl;
    cout << "Load and Detect Aruco Markers = 2 : " << endl;
    cout << "$ ./exc mode calibrationFileName capIndex arucoSquareDimention" << endl;
    cout << "$ ./exc 2 calibration.txt 0 0.125" << endl;

    // ./exc 0 calibration.txt 0
    // ./exc 1 calibration.txt 0 9 6 0.025 0 30 1
    // ./exc 2 calibration.txt 0 0.125
}



void draw(){
    // TODO
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPolygonMode(GL_BACK, GL_LINES);
    //glCullFace(GL_BACK);

    gluLookAt(9,9,10,0,0,0,0,0,1);

    glPushMatrix();
    
    GLfloat Lposition2 [4] = {9.0, -9.0, 10.0, 1.0}; /* position “reelle” */
    glLightfv (GL_LIGHT0, GL_POSITION, Lposition2);

    GLfloat Lambiant [4] = {0.4, 0.4, 0.4, 1.0};
    GLfloat Lblanche [4] = {1.0, 1.0, 1.0, 1.0};
    glLightfv (GL_LIGHT0, GL_AMBIENT, Lambiant);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, Lblanche);
    glLightfv (GL_LIGHT0, GL_SPECULAR, Lblanche);


    glPopMatrix();

    glBegin(GL_LINES);
        //repere
        //x(rouge)
        glColor3f(1.0, 0.0, 0.0);
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(0.0, 5.0, 0.0);
        //y(vert)
        glColor3f(0.0, 1.0, 0.0);
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(5.0, 0.0, 0.0);
        //z(bleu)
        glColor3f(0.0, 0.0, 1.0);
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(0.0, 0.0, 5.0);
    glEnd();

    glColor3f(1.0, 1.0, 1.0);

    GLUquadric* quadric = gluNewQuadric();

    gluQuadricDrawStyle(quadric, GLU_FILL );
    gluSphere(quadric , 4 , 36 , 18 );


    glutSwapBuffers();
}

int main( int argc, char **argv ){
    string windowsName = "Webcam";

    int mode, capIndex, boardSize_Width, boardSize_Height, numberOfInput, secondsBetweenFrame;
    float square_Size, arucoSquareDimention;
    Pattern calibrate_Pattern;
    string calibrationFile;
    VideoCapture cap;

    if (argc >= 4){
        mode = atoi(argv[1]);
        calibrationFile = argv[2];
        capIndex = atoi(argv[3]);

        // init for thread
        XInitThreads();

        cap = VideoCapture(capIndex);
        cam = Video(cap, windowsName);

        if (mode == 0 || mode == 2){
            // load calibration
            thread loadCalibrationThread(loadCalibration, calibrationFile);
            loadCalibrationThread.join();

            if (!cam.calibrated){
                cout << "not calibrated" << endl;

                cout << "default calibration" << endl;
                cam.setDefaultCalibration();
            }

            if (mode == 2){
                if (argc >= 5){
                    // detect aruco marker
                    arucoSquareDimention = atof(argv[4]); // 0.125f

                    // start display video capture
                    cout << "start display..." << endl;
                    thread displayThread(display);

                    cout << "start detection..." << endl;
                    thread arucoMarkerThread(detectionArucoMarker, arucoSquareDimention);

                    arucoMarkerThread.join();
                    displayThread.join();

                    cout << "mode 2 DONE" << endl;
                    return 0;
                }
            }
            else
            {
                cout << "mode 0 DONE" << endl;
                return 0;
            }
        }
        else if (mode == 1){
            if (argc >= 10){
                // save calibration
                boardSize_Width = atoi(argv[4]);
                boardSize_Height = atoi(argv[5]);
                square_Size = atof(argv[6]);
                calibrate_Pattern = (Pattern)atoi(argv[7]);
                numberOfInput = atoi(argv[8]);
                secondsBetweenFrame = atoi(argv[9]);


                calib = CalibrationProcess(boardSize_Width, boardSize_Height, square_Size, calibrate_Pattern);

                // start display video capture
                cout << "start display..." << endl;
                thread displayThread(display);
                
                cout << "saving calibration..." << endl;
                thread saveCalibrationThread(saveCalibration, numberOfInput, secondsBetweenFrame, calibrationFile);
                
                saveCalibrationThread.join();
                displayThread.join();

                cout << "mode 1 DONE" << endl;
                return 0;
            }
        }
    }
    else if (argc >= 2 && atoi(argv[1]) == 3){
        // TODO
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

        glutInitWindowSize(500, 500);
        glutInitWindowPosition(100, 100);

        glutCreateWindow("extrude test");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPerspective(70,1,1,1000);
        glutDisplayFunc(draw);
        glutMainLoop();
        return 0;
    }
    printManual();
    return -1;
}
