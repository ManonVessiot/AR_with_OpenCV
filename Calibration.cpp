#include "Calibration.h"

Calibration::Calibration(int _boardSize_Width, int _boardSize_Height, float _square_Size, Pattern _calibrate_Pattern){
    boardSize_Width = _boardSize_Width;
    boardSize_Height = _boardSize_Height;
    square_Size = _square_Size;
    calibrate_Pattern = _calibrate_Pattern;
}

Calibration::~Calibration(){
    
}

bool Calibration::addInput(Mat &_image){
    bool found = false;
    vector<Point2f> pointBuf;
    Size chessboardSize(boardSize_Width, boardSize_Height);
    int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;

    if (calibrate_Pattern == CHESSBOARD){
        found = findChessboardCorners(_image, chessboardSize, pointBuf, chessBoardFlags);
        if (found){
            Mat viewGray;
            cvtColor(_image, viewGray, COLOR_BGR2GRAY);
            cornerSubPix( viewGray, pointBuf, Size(11,11), Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.0001 ));
        }
    }
    else if (calibrate_Pattern == CIRCLES_GRID)
    {
        found = findCirclesGrid(_image, chessboardSize, pointBuf);
    }
    else if (calibrate_Pattern == ASYMMETRIC_CIRCLES_GRID)
    {
        found = findCirclesGrid(_image, chessboardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
    }

    if (found){        
        Mat inputToShow = _image.clone();
        drawChessboardCorners(inputToShow, chessboardSize, Mat(pointBuf), found);
        cv::imshow("Calibration", inputToShow);

        inputImages.push_back(_image);
        inputPointBuf.push_back(pointBuf);
    }
    return found;
}

void Calibration::clearInput(){
    inputImages.clear();
    inputPointBuf.clear();
}

int Calibration::inputSize(){
    return inputImages.size();
}

void Calibration::calibrate(){
    // TODO
}