#include "stdafx.h"

const char *base_filename_format = "../capture/%s";

char *prefix = nullptr;

int frame_count = 0;

char base_filename[MAX_PATH];

void set_current_directory();
void usage();
void start_capture();
void stop_capture(const float &fps);
void save_image(const char *base_filename, const cv::Mat &src, const cv::Size &dst_size);

cv::Mat resize_image(const cv::Mat &src, const cv::Size &dst_size);

int main(int argc, char* argv[])
{
	bool rv;
	bool break_flag;
	cv::VideoCapture capture;
	cv::Mat capture_img;
	cv::Mat canvas;

	set_current_directory();
	::CreateDirectory("../capture/", NULL);

	if (argc < 4) {
		usage();
		return -1;
	}

	char *movie_file = argv[1];
	prefix = argv[2];
	float fps = (float)atof(argv[3]);

	cv::Size output_size;
	if (argc == 6) {
		output_size.width  = atoi(argv[4]);
		output_size.height = atoi(argv[5]);
	}

	rv = capture.open(movie_file);
	if (rv == false) {
		printf("error : capture.open() failed...movie_file=%s\n", movie_file);
		return -1;
	}

	break_flag = false;

	int count = 0;
	std::vector<cv::Mat> movie_imgs;

	while (!break_flag) {
		capture >> capture_img;
		if (capture_img.empty()) break;

		cv::Mat resize_img = resize_image(capture_img, output_size);
		movie_imgs.push_back(resize_img);

		// キャプチャ画面を描画
		resize_img.copyTo(canvas);
		int n = count / 15;
		if (n % 2 == 0) {
			cv::circle(canvas, cv::Point(50, 50), 40, CV_RGB(255, 0, 0), CV_FILLED);
		}
		cv::imshow("ParaParaConvert", canvas);
		count++;
		if (count == 1000) break;

		int c = cv::waitKey(1);
	}

	count = 0;
	start_capture();
	while (true) {
		cv::Mat img = movie_imgs[count];

		// 読み取った画像を保存
		save_image(base_filename, img, output_size);

		// キャプチャ画面を描画
		img.copyTo(canvas);
		int n = count / 15;
		if (n % 2 == 0) {
			cv::circle(canvas, cv::Point(50, 50), 40, CV_RGB(255, 0, 0), CV_FILLED);
		}
		cv::imshow("ParaParaConvert", canvas);

		count++;
		if (movie_imgs.size() <= count) break;

		int c = cv::waitKey(1);
	}
	// 終了処理
	stop_capture(fps);
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
	printf("usage : ParaParaConvert.exe [movie_file] [prefix] [fps] ([output_width] [output_height])\n");
	printf("\n");
	printf("\n");
}

void start_capture()
{
	frame_count = 0;

	_snprintf_s(base_filename, MAX_PATH, base_filename_format, prefix);

	printf("start_capture() : base_filename=%s\n", base_filename);
}

void stop_capture(const float &fps)
{
	float frame_t = 1.0f / fps;
	printf("stop_capture() : frame_count=%d, fps=%f, frame_t=%f\n", frame_count, fps, frame_t);

	// 設定ファイルの書き出し.
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

void save_image(const char *base_filename, const cv::Mat &src, const cv::Size &dst_size)
{
	char image_filename[MAX_PATH];

	// ファイル名作成
	_snprintf_s(image_filename, MAX_PATH, "%s-%04d.jpg", base_filename, frame_count);

	cv::imwrite(image_filename, src);

	frame_count++;
}

cv::Mat resize_image(const cv::Mat &src, const cv::Size &dst_size)
{
	cv::Mat dst;

	if (dst_size == cv::Size()) {
		src.copyTo(dst);
		return dst;
	}

	if (src.cols == dst_size.width && src.rows == dst_size.height) {
		src.copyTo(dst);
		return dst;

	}

	// 縦横比計算
	float src_a = src.cols / (float)src.rows;
	float dst_a = dst_size.width / (float)dst_size.height;

	if (abs(src_a - dst_a) < 0.001f) {
		// 縦横比が同じの場合は縮小するだけ
		cv::resize(src, dst, dst_size);
	}
	else {
		cv::Rect src_roi;
		if (src_a > dst_a) {
			// dst_size.heightに合わせる場合
			src_roi.width = (int)(src.cols / (float)src_a * (float)dst_a);
			src_roi.height = src.rows;
			src_roi.x = (src.cols - src_roi.width) / 2;
			src_roi.y = 0;
		}
		else {
			// dst_size.widthに合わせる場合
			src_roi.width = src.cols;
			src_roi.height = (int)(src.rows / (float)dst_a * (float)src_a);
			src_roi.x = 0;
			src_roi.y = (src.rows - src_roi.height) / 2;
		}
		cv::resize(src(src_roi), dst, dst_size);
	}

	return dst;
}
