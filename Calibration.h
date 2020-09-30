#ifndef DEF_CALIBRATION
#define DEF_CALIBRATION

#include "opencv.h"
#include <vector>


enum Pattern {
    CHESSBOARD,
    CIRCLES_GRID,
    ASYMMETRIC_CIRCLES_GRID
};

class Calibration {

    private:
        int boardSize_Width;
        int boardSize_Height;
        float square_Size;

        Pattern calibrate_Pattern;

        vector<Mat> inputImages;
        vector<vector<Point2f>> inputPointBuf;

    public:
        Calibration(){};
        Calibration(int _boardSize_Width, int _boardSize_Height, float _square_Size, Pattern _calibrate_Pattern);
        ~Calibration();

        bool addInput(Mat &_image);
        void clearInput();
        int inputSize();
        void calibrate();
};
#endif
