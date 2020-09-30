################################################################################
 #    MAKEFILE
################################################################################

OPENCV_LIBS = -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_tracking -lopencv_highgui -lopencv_videoio -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect

LIBS = $(OPENCV_LIBS)

INC = -I /usr/local/include/opencv4 


all: exc

Calibration.o: Calibration.cpp Calibration.h opencv.h
	$(CXX) -c Calibration.cpp $(INC)

Video.o: Video.cpp Video.h opencv.h
	$(CXX) -c Video.cpp $(INC)

main.o: main.cpp main.h opencv.h
	$(CXX) -c main.cpp $(INC)

exc : main.o Video.o Calibration.o
	$(CXX) -o exc main.o Video.o Calibration.o $(LIBS) -std=c++0x -pthread

clean :
	rm -f *.o exc

