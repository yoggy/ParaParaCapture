#include "stdafx.h"
#include "../common/CvSharedMat.h"

int main(int argc, char* argv[])
{
	bool break_flag = false;

	CvSharedMat shared_img("movie", cv::Size(640, 480), CV_8UC3);
	cv::Mat img;

	while (!break_flag) {
		shared_img.download(img);

		cv::imshow("ParaParaSharedMemoryPlayer", img);

		int c = cv::waitKey(33);

		switch (c) {
		case 27:
			break_flag = true;
			break;
		}
	}

	cv::destroyAllWindows();
}
