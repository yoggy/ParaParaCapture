#include "StdAfx.h"

#include "CvSharedMat.h"

#pragma warning(disable : 4996)
 
CvSharedMat::CvSharedMat(const char *name, const cv::Size &size, const int &type)
	: shmem_(INVALID_HANDLE_VALUE), buf_(NULL)
{
	assert(name_ != NULL);
	assert(size.width > 0);
	assert(size.height > 0);

	strncpy(name_, name, 255);
	
	image_.create(size, type);

	char shared_memory_name[512];
	_snprintf(shared_memory_name, 511, "shared_%s", name_); 

	shmem_ = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		shmem_size(),
		shared_memory_name
		);

	buf_ = (uchar*)::MapViewOfFile(shmem_, FILE_MAP_ALL_ACCESS, 0, 0, shmem_size());

	image_.data = buf_;
}
 
CvSharedMat::~CvSharedMat(void)
{
	if (buf_ != NULL) {
		::UnmapViewOfFile(buf_);
		buf_ = NULL;
	}

	if (shmem_ != INVALID_HANDLE_VALUE) {
		::CloseHandle(shmem_);
		shmem_ = INVALID_HANDLE_VALUE;
	}
}
 
void CvSharedMat::upload(const cv::Mat &image)
{
	assert(buf_ != NULL);

	if (image.empty()) {
		image_.setTo(0);
		return;
	}

	if (image.size() == image_.size()) {
		image.copyTo(image_);
	}
	else {
		cv::resize(image, image_, image_.size());
	}
}
 
void CvSharedMat::download(cv::Mat &image)
{
	assert(buf_ != NULL);

	if (image.empty()) {
		image.create(size(), image_.type());
	}
	
	if (image.size() == image_.size()) {
		image_.copyTo(image);
	}
	else {
		cv::resize(image_, image, image.size());
	}	
}
 
int CvSharedMat::shmem_size() const
{
	return this->image_.total() * this->image_.elemSize();
}
 
int CvSharedMat::channels() const
{
	return this->image_.channels();
}
 
int CvSharedMat::type() const
{
	return this->image_.type();
}

cv::Size CvSharedMat::size() const
{
	return this->image_.size();
}
