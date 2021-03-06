#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define BOUND_BYTE(value) value < 0 ? 0 : (value > 255 ? 255 : value)
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

Image& Image::avg_grayscale()
{
	if (channels < 3) {
		throw new std::exception("this picture has been already grayscale");
		return *this;
	}
	for (int i = 0; i < size; i += channels) // (r + g + b) / 3
	{
		int gray = (data[i] + data[i + 1] + data[i + 2]) / 3;
		memset(data + i, gray, 3);
	}
	return *this;
}

Image& Image::lum_grayscale()
{
	if (channels < 3) {
		throw new std::exception("this picture has been already grayscale");
		return *this;
	}
	for (int i = 0; i < size; i += channels) // (r + g + b) / 3
	{
		int gray = 0.2126 * data[i] + 0.7152 * data[i + 1] + 0.0722 * data[i + 2];
		memset(data + i, gray, 3);
	}
	return *this;
}

Image& Image::mask(float r, float g, float b)
{
	if (channels < 3) {
		throw new std::exception("This picture needs at least 3 channels");
		return *this;
	}
	for (int i = 0; i < size; i += channels)
	{
		data[i] *= r;
		data[i + 1] *= g;
		data[i + 2] *= b;
	}
	return *this;
}

Image Image::rgb_to_bgr()
{
	if (channels < 3) {
		throw new std::exception("Needs at least 3 channels");
		return *this;
	}
	for (int i = 0; i < size; i += channels)
	{
		uint8_t r = data[i];
		data[i] = data[i + 2];
		data[i + 2] = r;
	}
	return *this;
}

Image& Image::encodemessage(std::string msg)
{
	uint32_t len = msg.size() * 8;
	if (len + STEG_HDR_SIZE > size)
	{
		throw new std::exception("this message is to large");
		return *this;
	}
	for (uint32_t i = 0; i < STEG_HDR_SIZE; ++i)
	{
		data[i] &= 0xFE;
		data[i] |= (len >> (STEG_HDR_SIZE - 1 - i)) & 1UL;
	}
	for (uint32_t i = 0; i < len; ++i)
	{
		data[i + STEG_HDR_SIZE] &= 0xFE;
		data[i + STEG_HDR_SIZE] |= (msg[i / 8] >> ((len - 1 - i) % 8)) & 1;
	}
	return *this;
}

Image& Image::decodemessage(char* buffer, size_t* msgLen)
{
	uint32_t len = 0;
	for (uint32_t i = 0; i < STEG_HDR_SIZE; ++i)
	{
		len = (len << 1) | (data[i] & 1);
	}
	*msgLen = len / 8;
	for (uint32_t i = 0; i < len; ++i)
	{
		buffer[i / 8] = (buffer[i / 8] << 1) | (data[i + STEG_HDR_SIZE] & 1);
	}
	return *this;
}

Image& Image::diffmap(Image& img)
{
	int compare_width = fmin(w, img.w);
	int compare_height = fmin(h, img.h);
	int compare_channels = fmin(channels, img.channels);
	for (uint32_t i = 0; i < compare_height; ++i)
	{
		for (uint32_t j = 0; j < compare_width; ++j)
		{
			for (uint32_t k = 0; k < compare_channels; ++k)
			{
				data[(i * w + j) * channels + k] =
					BOUND_BYTE(abs(data[(i * w + j) * channels + k] - img.data[(i * img.w + j) * img.channels + k]));
			}
		}
	}
	return *this;
}

Image& Image::scale_diffmap(Image& img, uint8_t scl)
{
	int compare_width = fmin(w, img.w);
	int compare_height = fmin(h, img.h);
	int compare_channels = fmin(channels, img.channels);
	uint8_t largest = 0;
	for (uint32_t i = 0; i < compare_height; ++i)
	{
		for (uint32_t j = 0; j < compare_width; ++j)
		{
			for (uint32_t k = 0; k < compare_channels; ++k)
			{
				data[(i * w + j) * channels + k] =
					BOUND_BYTE(abs(data[(i * w + j) * channels + k] - img.data[(i * img.w + j) * img.channels + k]));
				largest = fmax(largest, data[(i * w + j) * channels + k]);
			}
		}
	}
	scl = 255 / fmax(1, fmax(scl, largest));
	for (int i = 0; i < size; ++i)
		data[i] *= scl;
	return *this;
}

Image& Image::convole_clampTo0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc)
{
	uint8_t* new_data = new uint8_t[w * h];
	uint64_t center = cr * ker_w + cc;
	for (uint64_t k = channels; k < size; k += channels)
	{
		double sum = 0;
		for (long i = -((long)cr); i < (long)ker_h - cr; ++i)
		{
			long row = ((long)k / channels) / w - i;
			if (row < 0 or row > h - 1)
				continue;
			for (long j = -((long)cc); j < (long)ker_w - cc; ++j)
			{
				long col = ((long)k / channels) % w - j;
				if (col < 0 or col > w - 1)
					continue;
				sum += ker[center + i * (long)ker_w + j] * data[(row * w + col) * channels + channel];
			}
		}
		new_data[k / channels] = (uint8_t)BOUND_BYTE(round(sum));
	}
	for (uint64_t k = channels; k < size; k += channels)
	{
		data[k] = new_data[k / channels];
	}
	delete[] new_data;
	return *this;
}

Image& Image::convole_clampToBorder(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc)
{
	uint8_t* new_data = new uint8_t[w * h];
	uint64_t center = cr * ker_w + cc;
	for (uint64_t k = channels; k < size; k += channels)
	{
		double sum = 0;
		for (long i = -((long)cr); i < (long)ker_h - cr; ++i)
		{
			long row = ((long)k / channels) / w - i;
			if (row < 0) row = 0;
			else if (row > h - 1) row = h - 1;
			for (long j = -((long)cc); j < (long)ker_w - cc; ++j)
			{
				long col = ((long)k / channels) % w - j;
				if (col < 0) col = 0;
				else if (col > w - 1) col = w - 1;
				sum += ker[center + i * (long)ker_w + j] * data[(row * w + col) * channels + channel];
			}
		}
		new_data[k / channels] = (uint8_t)BOUND_BYTE(round(sum));
	}
	for (uint64_t k = channels; k < size; k += channels)
	{
		data[k] = new_data[k / channels];
	}
	delete[] new_data;
	return *this;
}

Image& Image::convole_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc)
{
	uint8_t* new_data = new uint8_t[w * h];
	uint64_t center = cr * ker_w + cc;
	for (uint64_t k = channels; k < size; k += channels)
	{
		double sum = 0;
		for (long i = -((long)cr); i < (long)ker_h - cr; ++i)
		{
			long row = ((long)k / channels) / w - i;
			if (row < 0) row = (row % h) + h;
			else if (row > h - 1) row %= h;
			for (long j = -((long)cc); j < (long)ker_w - cc; ++j)
			{
				long col = ((long)k / channels) % w - j;
				if (col < 0) col = (col % w) + w;
				else if (col > w - 1) col %= w;
				sum += ker[center + i * (long)ker_w + j] * data[(row * w + col) * channels + channel];
			}
		}
		new_data[k / channels] = (uint8_t)BOUND_BYTE(round(sum));
	}
	for (uint64_t k = channels; k < size; k += channels)
	{
		data[k] = new_data[k / channels];
	}
	delete[] new_data;
	return *this;
}

Image::Image(const Image& img) : Image(img.w, img.h, img.channels)
{
	memcpy(data, img.data, size);
}

Image::~Image()
{
	stbi_image_free(data);
}
