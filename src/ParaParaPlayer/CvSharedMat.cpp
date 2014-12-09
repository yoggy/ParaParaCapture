#include "StdAfx.h"

#include "CvSharedMat.h"

#pragma warning(disable : 4996)

class ScopedMutexLock {
protected:
	HANDLE mutex_;

public:
	ScopedMutexLock(HANDLE mutex) : mutex_(INVALID_HANDLE_VALUE) {
		mutex_ = mutex;
		if (mutex_ != INVALID_HANDLE_VALUE) {
			WaitForSingleObject(mutex_, INFINITE);
		}
	};

	virtual ~ScopedMutexLock() {
		if (mutex_ != INVALID_HANDLE_VALUE) {
			::ReleaseMutex(mutex_);
			mutex_ = INVALID_HANDLE_VALUE;
		}
	};
};

 
CvSharedMat::CvSharedMat(const char *name, const cv::Size &size, const int &type, const bool &use_mutex)
	: shmem_(INVALID_HANDLE_VALUE), mutex_(INVALID_HANDLE_VALUE), buf_(NULL)
{
	assert(name_ != NULL);
	assert(size.width > 0);
	assert(size.height > 0);

	strncpy(name_, name, 255);
	
	image_.create(size, type);

	if (use_mutex) {
		char mutex_name[512];
		_snprintf(mutex_name, 511, "mutex_%s", name_); 
		mutex_ = ::CreateMutex(NULL, FALSE, mutex_name);
	}

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

	if (mutex_ != INVALID_HANDLE_VALUE) {
		::ReleaseMutex(mutex_);
		mutex_ = INVALID_HANDLE_VALUE;
	}
}
 
void CvSharedMat::upload(const cv::Mat &image)
{
	ScopedMutexLock lock(mutex_);

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
	ScopedMutexLock lock(mutex_);

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
 
cv::Size CvSharedMat::size() const
{
	return this->image_.size();
}
