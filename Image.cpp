#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

Image::Image(std::string filename)
{
	if (!read(filename))
	{
		throw new std::exception("Can not read a file, delete this object");
		this->~Image();
	}
	std::cout << "success" << std::endl;
}

Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels)
{
	size = w * h * channels;
	data = new uint8_t[size];
}

bool Image::read(std::string filename)
{
	data = stbi_load(filename.c_str(), &w, &h, &channels, 0);
	size = w * h * channels;
	return data != nullptr;
}

bool Image::write(std::string filename)
{
	ImageType type = GetImageType(filename);
	int success = 0;
	switch (type)
	{
		case ImageType::PNG:
		{
			success = stbi_write_png(filename.c_str(), w, h, channels, data, w * channels);
			break;
		}
		case ImageType::JPG:
		{
			success = stbi_write_jpg(filename.c_str(), w, h, channels, data, 100);
			break;
		}
		case ImageType::BMP:
		{
			success = stbi_write_bmp(filename.c_str(), w, h, channels, data);
			break;
		}
	}
	return success;
}

Image::Image(const Image& img) : Image(img.w, img.h, img.channels)
{
	memcpy(data, img.data, size);
}

Image::~Image()
{
	stbi_image_free(data);
}
