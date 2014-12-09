#pragma once

#include <string>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class CvSharedMat
{
public:
	CvSharedMat(const char *name, const cv::Size &size, const int &type, const bool &use_mutex = false);
	~CvSharedMat(void);

	int shmem_size() const;
	cv::Size size() const;
	int channels() const;

	void upload(const cv::Mat &image);
	void download(cv::Mat &image);

protected:
	char		name_[256];
	HANDLE		shmem_;
	HANDLE		mutex_;
	uchar		*buf_;
	cv::Mat		image_;
};
