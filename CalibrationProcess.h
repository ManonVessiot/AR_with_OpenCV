#ifndef DEF_CALIBRATION_PROCESS
#define DEF_CALIBRATION_PROCESS

#include "opencv.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>


enum Pattern {
    CHESSBOARD,
    CIRCLES_GRID,
    ASYMMETRIC_CIRCLES_GRID
};

class CalibrationProcess {
    protected:
        Size boardSize;
        float square_Size;

        Pattern calibrate_Pattern;

        vector<Mat> inputImages;
        vector<vector<Point2f>> inputPointBuf;

        Mat cameraMatrix;
        Mat distanceCoefficients;

        void createKnownBoardPosition(vector<Point3f>& corners);

    public:
        CalibrationProcess(){};
        CalibrationProcess(int _boardSize_Width, int _boardSize_Height, float _square_Size, Pattern _calibrate_Pattern);
        ~CalibrationProcess();

        bool addInput(Mat &_image);
        void clearInput();
        int inputSize();

        void calibrate(Mat& _cameraMatrix, Mat& _distanceCoefficients);
        bool saveCalibration(string name);
};
#endif
