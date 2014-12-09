#include "stdafx.h"
#include "CvSharedMat.h"

std::vector<cv::Mat > images;

void set_current_directory();
void usage();
bool load_images(const char *config_filename);

double frame_t = 0.033;

int main(int argc, char* argv[])
{
	set_current_directory();

	if (argc != 2) {
		usage();
		return -1;
	}

	bool rv;
	rv = load_images(argv[1]);
	if (rv == false) {
		printf("error : load_images() ...filename=%s\n", argv[1]);
		return -1;
	}

	CvSharedMat shared_mat("movie", images[0].size(), CV_8UC3);


	bool break_flag = false;
	bool enable_play = false;
	int frame_count = 0;

	cv::Point play_icon[3];
	play_icon[0] = cv::Point(20, 10);
	play_icon[1] = cv::Point(100, 50);
	play_icon[2] = cv::Point(20, 90);

	DWORD st = ::GetTickCount();
	while (!break_flag) {
		cv::Mat img = images[frame_count];

		// キャプチャ画像を共有メモリにアップロード
		shared_mat.upload(img);

		// キャプチャ画像を描画
		cv::Mat canvas;
		img.copyTo(canvas);
		if (enable_play) {
			int n = frame_count / 15;
			if (n % 2 == 0) {
				cv::fillConvexPoly(canvas, play_icon, 3, CV_RGB(0, 255, 0));
			}
		}

		cv::imshow("ParaParaPlayer", canvas);

		if (enable_play) {
			frame_count++;
			if (images.size() == frame_count) {
				frame_count = images.size() - 1;
				enable_play = false;
			}
		}

		int c = cv::waitKey(1);

		switch (c) {
		case 27:
			break_flag = true;
			break;
		case ' ':
			enable_play = !enable_play;
			break;
		case 'c':
			frame_count = 0;
			enable_play = false;
			break;
		}

		DWORD t = ::GetTickCount();
		DWORD diff = t - st;
		if (diff < frame_t * 1000) {
			double sleep_time = frame_t * 1000 - diff;
			//printf("sleep_time=%f\n", sleep_time);
			::Sleep((DWORD)sleep_time);
		}
		st = t;
	}

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

void usage()
{
	printf("usage : ParaParaPlayer.exe [config_filename]  \n");
	printf("\n");
	printf("\n");
}

void chomp(char *str)
{
	if (str == nullptr) return;

	size_t len = strlen(str) - 1;
	if (len == 0) return;

	if (str[len] == '\n') str[len] = '\0';
}

bool load_images(const char *config_filename)
{
	FILE *fp = fopen(config_filename, "r");
	if (fp == nullptr) {
		printf("error : fopen() failed...config_filename=%d", config_filename);
		return false;
	}

	char base_filename[MAX_PATH];
	fgets(base_filename, MAX_PATH, fp);
	chomp(base_filename);

	char frame_count_str[MAX_PATH];
	fgets(frame_count_str, MAX_PATH, fp);
	chomp(frame_count_str);
	int frame_count = atoi(frame_count_str);

	if (frame_count == 0) {
		printf("error : load_images() invalid frame_count...config_filename=%d", frame_count);
		return false;
	}

	char frame_t_str[MAX_PATH];
	fgets(frame_t_str, MAX_PATH, fp);
	chomp(frame_t_str);
	frame_t = atof(frame_t_str);

	printf("loading images...config_filename=%s, frame_count=%d\n", config_filename, frame_count);
	images.clear();

	char image_filename[MAX_PATH];
	for (int i = 0; i < frame_count; ++i) {
		_snprintf_s(image_filename, MAX_PATH, "%s-%04d.jpg", base_filename, i);

		cv::Mat img = cv::imread(image_filename);
		if (img.empty()) {
			printf("error : image loading error...image_filename=%s\n", image_filename);
		}

		images.push_back(img);

		if (i % 100 == 0) {	
			printf("    ....%4d\n", i);
		}
	}
	return true;
}