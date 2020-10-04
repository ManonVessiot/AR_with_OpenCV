#include "main.h"  

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
    cout << " --> capIndex identify your webcam (0)" << endl;
    cout << " --> calibrationFileName is the name of the file, where the calibration is saved" << endl;
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
    cout << "$ ./exc mode capIndex calibrationFileName" << endl;
    cout << "$ ./exc 0 0 calibration.txt" << endl;
    cout << endl;
    cout << "Save Calibration = 1 : " << endl;
    cout << "$ ./exc mode capIndex calibrationFileName boardSize_Width boardSize_Height square_Size calibrate_Pattern numberOfInput secondsBetweenFrame" << endl;
    cout << "$ ./exc 1 0 calibration.txt 9 6 0.025 0 30 1" << endl;
    cout << endl;
    cout << "Load and Detect Aruco Markers = 2 : " << endl;
    cout << "$ ./exc mode capIndex calibrationFileName arucoSquareDimention" << endl;
    cout << "$ ./exc 2 0 calibration.txt 0.125" << endl;
    cout << endl;
    cout << "OpenGL = 3 : " << endl;
    cout << "$ ./exc mode capIndex calibrationFileName" << endl;
    cout << "$ ./exc 3 0 calibration.txt" << endl;

    // ./exc 0 0 calibration.txt
    // ./exc 1 0 calibration.txt 9 6 0.025 0 30 1
    // ./exc 2 0 calibration.txt 0.125
    // ./exc 3 0
}



void draw(){
    // TODO
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // GL_COLOR_BUFFER_BIT & GL_DEPTH_BUFFER_BIT & GL_ACCUM_BUFFER_BIT & GL_STENCIL_BUFFER_BIT
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    cam.updateFrame();
    Mat frame = cam.getFrame();
    
    unsigned int data[frame.rows][frame.cols][3];
    for( size_t y = 0; y <  frame.rows; y++ )
    {
        for( size_t x = 0; x < frame.cols; ++x )
        {

            Vec3b intensity = frame.at<Vec3b>(y,x);
            //cout << intensity[0] << endl;
              data[frame.rows - y - 1][x][0] =intensity[2]* 256 * 256 * 256;
              data[frame.rows - y - 1][x][1] =intensity[1]* 256 * 256 * 256;
              data[frame.rows - y - 1][x][2] =intensity[0]* 256 * 256 * 256;
        }
    }
    glDrawPixels(frame.cols, frame.rows, GL_RGB, GL_UNSIGNED_INT, data);



    // GL_MODELVIEW & GL_PROJECTION & GL_TEXTURE & GL_COLOR
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // glPolygonMode(GL_BACK, GL_LINES);
    // glCullFace(GL_BACK);

    gluLookAt(2, 2, 2, 0, 0, 0, 0, 0, 1);

    glPushMatrix();

    // Repère spatial
    glBegin(GL_LINES);
        //x (vert)
        glColor3f(0.0, 1.0, 0.0);
        glVertex3d(-1.0, 0.0, 0.0);
        glVertex3d(2.0, 0.0, 0.0);
        //y (rouge)
        glColor3f(1.0, 0.0, 0.0);
        glVertex3d(0.0, -1.0, 0.0);
        glVertex3d(0.0, 2.0, 0.0);
        //z (bleu)
        glColor3f(0.0, 0.0, 1.0);
        glVertex3d(0.0, 0.0, -1.0);
        glVertex3d(0.0, 0.0, 2.0);
    glEnd();

    glPopMatrix();

    glutSwapBuffers();
    glutPostRedisplay();
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
        capIndex = atoi(argv[2]);
        calibrationFile = argv[3];

        // init for thread
        XInitThreads();

        cap = VideoCapture(capIndex);
        cam = Video(cap, windowsName);

        if (mode == 0 || mode == 2 || mode == 3){
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
            else if (mode == 3){
                // TODO
                glutInit(&argc, argv);
                // GLUT_RGBA & GLUT_RGB & GLUT_INDEX & GLUT_SINGLE & GLUT_DOUBLE & GLUT_ACCUM &
                // GLUT_ALPHA & GLUT_DEPTH & GLUT_STENCIL & GLUT_MULTISAMPLE & GLUT_STEREO & GLUT_LUMINANCE
                glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

                glutInitWindowSize(cam.width, cam.height);
                glutInitWindowPosition(1500, 100);

                glutCreateWindow("OpenGL");

                glMatrixMode(GL_PROJECTION);
                gluPerspective(70, (double)cam.width / cam.height, 1, 1000);

                glutDisplayFunc(draw);
                glutMainLoop();
                
                cout << "mode 3 DONE" << endl;

                return 0;
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
    printManual();
    return -1;
}
