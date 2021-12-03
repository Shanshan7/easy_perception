#include<iostream>
#include<stdio.h>
#include<opencv2/opencv.hpp>
//#include<conio.h>
#include<termio.h>
using namespace std;
using namespace cv;

/*int scanKeyboard(){

	int in;
	struct termios new_settings;
	struct termios stored_settings;	
	tcgetattr(0,&stored_settings);
	new_settings = stored_settings;
	new_settings.c_lflag &= (~ICANON);
	new_settings.c_cc[VTIME] = 0;
	tcgetattr(0,&stored_settings);
	new_settings.c_cc[VMIN] = 1;
	tcgetattr(0,TCSANOW,&new_settings);

	in = getchar();

	tcgetattr(0,TCSANOW,&stored_settings);
	return in;
}*/


int main(int argc, char **argv) {
	VideoCapture cap(0);

	if (!cap.isOpened())
	{
		cout << "Failed to get frame" << endl;
		return 0;
	}
	int loop = -1;
	while (1) {

		loop++;
		Mat src, resultMat;
		cap >> src;


		if (src.empty()) {
			destroyAllWindows();
			cout<<"fail"<<endl;
			return 0;
		}
		src.copyTo(resultMat);

#if 1
		char key = 0;	
		if ((key = cv::waitKey(1)) != -1) { 
		//if (_kbhit()) {
			//key = _getch();
			cout<<"press key:"<<key<<endl;
			if (key == 'c')
			{
				char savefilename[100];
				sprintf(savefilename, "./img/%06d.jpg", loop);
				imwrite(savefilename, src);

				char text[100];
				sprintf(text, "picture capture:%06d.jpg", loop);
				putText(resultMat, text, Point(5, 30), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255));
			}
		}
#endif
		
		cv::imshow("video", resultMat);
		cv::waitKey(30);
	}

	return 0;
}
