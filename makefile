################################################################################
 #    MAKEFILE
################################################################################

OPENCV_LIBS = -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_tracking -lopencv_highgui -lopencv_videoio -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect

LIBS = $(OPENCV_LIBS) -lX11 -std=c++0x -pthread

INC = -I /usr/local/include/opencv4 


all: exc

CalibrationProcess.o: CalibrationProcess.cpp CalibrationProcess.h opencv.h
	$(CXX) -c CalibrationProcess.cpp $(INC)

Video.o: Video.cpp Video.h opencv.h
	$(CXX) -c Video.cpp $(INC)

main.o: main.cpp main.h opencv.h
	$(CXX) -c main.cpp $(INC)

exc : main.o Video.o CalibrationProcess.o
	$(CXX) -o exc main.o Video.o CalibrationProcess.o $(LIBS) 

clean :
	rm -f *.o exc
	clear

