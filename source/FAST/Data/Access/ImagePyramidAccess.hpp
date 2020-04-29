#pragma once

#include <FAST/Object.hpp>
#include <FAST/Data/DataTypes.hpp>

typedef struct _openslide openslide_t;

namespace fast {

class Image;
class ImagePyramid;

typedef struct ImagePyramidPatch {
	std::unique_ptr<uchar[]> data;
	int width;
	int height;
	int offsetX;
	int offsetY;
} Patch;

typedef struct ImagePyramidLevel {
	int width;
	int height;
	int patches;
	bool memoryMapped;
	uint8_t* data;
#ifdef WIN32
	void* fileHandle;
#else
	int fileHandle;
#endif
} Level;

class FAST_EXPORT ImagePyramidAccess : Object {
public:
	typedef std::unique_ptr<ImagePyramidAccess> pointer;
	ImagePyramidAccess(std::vector<ImagePyramidLevel> levels, openslide_t* fileHandle, SharedPointer<ImagePyramid> imagePyramid, bool writeAccess);
	void setScalar(uint x, uint y, uint level, uint8_t value, uint channel = 0);
	void setScalarFast(uint x, uint y, uint level, uint8_t value, uint channel = 0) noexcept;
	uint8_t getScalar(uint x, uint y, uint level, uint channel = 0);
	uint8_t getScalarFast(uint x, uint y, uint level, uint channel = 0) noexcept;
	std::unique_ptr<uchar[]> getPatchData(int level, int x, int y, int width, int height);
	ImagePyramidPatch getPatch(std::string tile);
	ImagePyramidPatch getPatch(int level, int patchX, int patchY);
	SharedPointer<Image> getLevelAsImage(int level);
	SharedPointer<Image> getPatchAsImage(int level, int offsetX, int offsetY, int width, int height);
	SharedPointer<Image> getPatchAsImage(int level, int patchIdX, int patchIdY);
	void release();
	~ImagePyramidAccess();
private:
	SharedPointer<ImagePyramid> m_image;
	std::vector<ImagePyramidLevel> m_levels;
	bool m_write;
	openslide_t* m_fileHandle;
};

}