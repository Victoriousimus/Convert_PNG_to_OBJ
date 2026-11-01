#ifndef GMB_STBI_PNG_CLASS
#define GMB_STBI_PNG_CLASS
#pragma once


#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "STB\stb_image.h"
#include "KHR/khrplatform.h"
typedef void* PMem;
typedef const char* c_wrd;
typedef char* t_wrd;
typedef const char c_sym;
typedef char t_sym;
typedef bool Bool;
typedef float f_4b;
typedef double f_8b;
typedef int8_t i_1b;
typedef uint8_t u_1b;
typedef int16_t i_2b;
typedef uint16_t u_2b;
typedef int32_t i_4b;
typedef uint32_t u_4b;
typedef int64_t i_8b;
typedef uint64_t u_8b;

namespace stbi_png {
	class CImageRGB {
	private:
		i_4b x;
		i_4b y;
		u_1b* data;
		Bool empty;
	public:
		CImageRGB() : empty(true), data(nullptr), x(0), y(0) {}
		~CImageRGB() { free(); }
		CImageRGB(CImageRGB&&) = delete;
		CImageRGB(const CImageRGB&) = delete;
		CImageRGB& operator=(CImageRGB&&) = delete;
		CImageRGB& operator=(const CImageRGB&) = delete;
		void free() {
			if (empty) return;
			stbi_image_free(data);
			empty = true;
			x = y = -1;
		}
		i_4b load(c_wrd _filename) {
			if (!empty) return -1;
			i_4b n{ 0 };
			data = stbi_load(_filename, &(x), &(y), &(n), 3);
			if (!data || n != 3) return -1;
			empty = false;
			return x;
		}
		i_4b getPix(i_2b _X, i_2b _Y, u_1b(*color)[3]) {
			if (_X > x || _X<0 || _Y > y || _Y<0) {
				(*color)[0] = 0;
				(*color)[1] = 0;
				(*color)[2] = 0;
				return -1;
			}
			std::memcpy(*color, &(data[(_Y * y + _X) * 3]), 3);
			return _Y * y + _X + 1;
		}
		i_4b setPix(i_2b _X, i_2b _Y, u_1b(&color)[3]) {
			if (_X > x || _X<0 || _Y > y || _Y<0) return -1;
			std::memcpy(&(data[(_Y * y + _X) * 3]), color, 3);
			return 0;
		}
		u_1b* getData() { return data; }
		i_4b getSizeX() { return x; }
		i_4b getSizeY() { return y; }
	};
	class CImageRGBA {
	private:
		i_4b x;
		i_4b y;
		u_1b* data;
		Bool empty;
	public:
		CImageRGBA() : empty(true), data(nullptr), x(0), y(0) {}
		~CImageRGBA() { free(); }
		CImageRGBA(CImageRGBA&&) = delete;
		CImageRGBA(const CImageRGBA&) = delete;
		CImageRGBA& operator=(CImageRGBA&&) = delete;
		CImageRGBA& operator=(const CImageRGBA&) = delete;
		void free() {
			if (empty) return;
			stbi_image_free(data);
			empty = true;
			x = y = 0;
		}
		i_4b load(c_wrd _filename) {
			if (!empty) return -1;
			i_4b n{ 0 };
			data = stbi_load(_filename, &(x), &(y), &(n), 4);
			if (!data || n != 3) return -1;
			empty = false;
			return x;
		}
		i_4b getPix(i_2b _X, i_2b _Y, u_1b(*color)[4]) {
			if (_X > x || _X<0 || _Y > y || _Y < 0) {
				(*color)[0] = 0;
				(*color)[1] = 0;
				(*color)[2] = 0;
				(*color)[3] = 0;
				return -1;
			}
			std::memcpy(*color, &(data[_Y * 4 * y + _X * 4]), 4);
			return _Y * y + _X + 1;
		}
		i_4b setPix(i_2b _X, i_2b _Y, u_1b(&color)[4]) {
			if (_X > x || _X<0 || _Y > y || _Y < 0) return -1;
			std::memcpy(&(data[_Y * 4 * y + _X * 4]), color, 4);
			return 0;
		}
		u_1b* getData() { return data; }
		i_4b getSizeX() { return x; }
		i_4b getSizeY() { return y; }
	};

}
#endif // !GMB_STBI_PNG_CLASS