#pragma once

#include <string>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class CvSharedMat
{
public:
	CvSharedMat(const char *name, const cv::Size &size, const int &type);
	~CvSharedMat(void);

	int shmem_size() const;
	cv::Size size() const;
	int channels() const;
	int type() const;

	void upload(const cv::Mat &image);
	void download(cv::Mat &image);

protected:
	char		name_[256];
	HANDLE		shmem_;
	uchar		*buf_;
	cv::Mat		image_;
};
