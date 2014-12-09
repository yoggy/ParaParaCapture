#include "stdafx.h"

const char *base_filename_format = "../capture/capture-%Y%m%d-%H%M%S";

int frame_count = 0;
bool enable_capture = false;

DWORD st;

char base_filename[MAX_PATH];

void set_current_directory();
void start_capture();
void stop_capture();
void save_image(const cv::Mat &img);

int main(int argc, char* argv[])
{
	bool rv;
	bool break_flag;
	cv::VideoCapture capture;
	cv::Mat capture_img;
	cv::Mat canvas;

	set_current_directory();
	::CreateDirectory("../capture/", NULL);

	rv = capture.open(0);
	if (rv == false) {
		printf("error : capture.open(0) failed...\n");
		return -1;
	}

	break_flag = false;

	while (!break_flag) {
		capture >> capture_img;

		// キャプチャ画像を保存
		if (enable_capture) {
			save_image(capture_img);
		}

		// キャプチャ画面を描画
		canvas = capture_img;
		if (enable_capture) {
			int n = frame_count / 15;
			if (n % 2 == 0) {
				cv::circle(canvas, cv::Point(50, 50), 40, CV_RGB(255, 0, 0), CV_FILLED);
			}
		}
		cv::imshow("ParaParaCapture", canvas);

		// 
		int c = cv::waitKey(1);
		switch (c) {
		case 27:
			break_flag = true;
			break;
		case ' ':
			if (enable_capture == true) {
				stop_capture();
			}
			else {
				start_capture();
			}
			break;
		}
	}
	
	// 終了処理
	stop_capture();
	cv::destroyAllWindows();
}

void set_current_directory()
{
	char exe_path[MAX_PATH];
	::GetModuleFileName(NULL, exe_path, MAX_PATH);

	char drive[MAX_PATH];
	char dir[MAX_PATH];
	char fname[MAX_PATH];
	char ext[MAX_PATH];

	::_splitpath_s(
		exe_path,
		drive, MAX_PATH,
		dir, MAX_PATH,
		fname, MAX_PATH,
		ext, MAX_PATH
		);

	char exe_dir[MAX_PATH];
	_makepath_s(
		exe_dir, MAX_PATH,
		drive,
		dir,
		"",
		""
		);

	::SetCurrentDirectory(exe_dir);
}

void start_capture()
{
	enable_capture = true;
	frame_count = 0;

	_tzset();

	time_t ltime;
	time(&ltime);

	struct tm today;
	_localtime64_s(&today, &ltime);

	strftime(base_filename, MAX_PATH, base_filename_format, &today);

	printf("start_capture() : base_filename=%s\n", base_filename);

	st = ::GetTickCount();
}

void stop_capture()
{
	if (enable_capture == false) return;

	DWORD diff = ::GetTickCount() - st;
	double frame_t = frame_count / (double)diff;

	enable_capture = false;
	printf("stop_capture() : frame_count=%d, diff=%d, frame_t=frame_t\n", frame_count, diff, frame_t);

	char config_filename[MAX_PATH];
	_snprintf_s(config_filename, MAX_PATH, "%s.txt", base_filename);

	FILE *fp = fopen(config_filename, "w");
	if (fp == nullptr) {
		printf("stop_capture() : fileopen failed...filename=%s\n", config_filename);
		return;
	}

	// base_filenameとframe_countとframe_tをファイルに記録する
	fprintf_s(fp, "%s\n%d\n%f\n", base_filename, frame_count, frame_t);

	fflush(fp);
	fclose(fp);
}

void save_image(const cv::Mat &img)
{
	char image_filename[MAX_PATH];

	_snprintf_s(image_filename, MAX_PATH, "%s-%04d.jpg", base_filename, frame_count);
	cv::imwrite(image_filename, img);
	frame_count++;
	if (frame_count >= 1000) {
		stop_capture();
	}
}
