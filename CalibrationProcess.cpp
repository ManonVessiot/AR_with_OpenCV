#include "CalibrationProcess.h"

CalibrationProcess::~CalibrationProcess(){
    
}

CalibrationProcess::CalibrationProcess(int _boardSize_Width, int _boardSize_Height, float _square_Size, Pattern _calibrate_Pattern){
    boardSize = Size(_boardSize_Width, _boardSize_Height);
    square_Size = _square_Size;
    calibrate_Pattern = _calibrate_Pattern;
}

bool CalibrationProcess::addInput(Mat &_image){
    bool found = false;
    vector<Point2f> pointBuf;
    int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;

    if (calibrate_Pattern == CHESSBOARD){
        found = findChessboardCorners(_image, boardSize, pointBuf, chessBoardFlags);
        if (found){
            Mat viewGray;
            cvtColor(_image, viewGray, COLOR_BGR2GRAY);
            cornerSubPix( viewGray, pointBuf, Size(11,11), Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.0001 ));
        }
    }
    else if (calibrate_Pattern == CIRCLES_GRID)
    {
        found = findCirclesGrid(_image, boardSize, pointBuf);
    }
    else if (calibrate_Pattern == ASYMMETRIC_CIRCLES_GRID)
    {
        found = findCirclesGrid(_image, boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
    }

    if (found){        
        Mat inputToShow = _image.clone();
        drawChessboardCorners(inputToShow, boardSize, Mat(pointBuf), found);
        namedWindow("CalibrationProcess", CV_WINDOW_AUTOSIZE);
        imshow("CalibrationProcess", inputToShow);
        moveWindow("CalibrationProcess", 0, 0);

        inputImages.push_back(_image);
        inputPointBuf.push_back(pointBuf);
    }
    return found;
}

void CalibrationProcess::clearInput(){
    inputImages.clear();
    inputPointBuf.clear();
}

int CalibrationProcess::inputSize(){
    return inputImages.size();
}

void CalibrationProcess::calibrate(Mat& _cameraMatrix, Mat& _distanceCoefficients){
    vector<vector<Point3f>> worldSpaceCornerPoints(1);

    createKnownBoardPosition(worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(inputPointBuf.size(), worldSpaceCornerPoints[0]);

    vector<Mat> rVectors, tVectors;
    distanceCoefficients = Mat::zeros(8, 1, CV_64F);

    calibrateCamera(worldSpaceCornerPoints, inputPointBuf, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);

    _cameraMatrix = cameraMatrix;
    _distanceCoefficients = distanceCoefficients;
}

void CalibrationProcess::createKnownBoardPosition(vector<Point3f>& corners){
    for (int i = 0; i < boardSize.height; i++)
    {
        for (int j= 0; j < boardSize.width; j++)
        {
            corners.push_back(Point3f(j * square_Size, i * square_Size, 0.0f));
        }        
    }    
}

bool CalibrationProcess::saveCalibration(string name){
    ofstream outStream(name);
    if (outStream){
        uint16_t rows = cameraMatrix.rows;
        uint16_t columns = cameraMatrix.cols;
        
        outStream << rows << endl;
        outStream << columns << endl;

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < columns; c++)
            {
                double value = cameraMatrix.at<double>(r, c);
                outStream << value << endl;
            }            
        }
        rows = distanceCoefficients.rows;
        columns = distanceCoefficients.cols;
        
        outStream << rows << endl;
        outStream << columns << endl;

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < columns; c++)
            {
                double value = distanceCoefficients.at<double>(r, c);
                outStream << value << endl;
            }            
        }

        outStream.close();
        return true;
    }
    return false;
}