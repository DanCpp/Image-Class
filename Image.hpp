#pragma once
#include <iostream>
#define STEG_HDR_SIZE sizeof(uint32_t) * 8


class Image
{
private:
	enum class ImageType {
		PNG,
		JPG,
		BMP
	};
	ImageType GetImageType(std::string filename)
	{
		std::string ext(strrchr(filename.c_str(), '.'));
		if (ext == ".png") return ImageType::PNG;
		else if (ext == ".jpg" or ext == ".jpeg") return ImageType::JPG;
		return ImageType::BMP;
	}
public:
	uint8_t* data = nullptr;
	size_t size;
	int w, h, channels;
	Image(const Image& img);
	~Image();
	Image(std::string filename);
	Image(int w, int h, int channels);

	bool read(std::string filename);
	bool write(std::string filename);

	Image& avg_grayscale();
	Image& lum_grayscale();
	Image& mask(float r, float g, float b);
	Image rgb_to_bgr();

	Image& encodemessage(std::string msg);

	Image& decodemessage(char* buffer, size_t* msgLen);

	Image& diffmap(Image& img);
	Image& scale_diffmap(Image& img, uint8_t scl = 0);
};

