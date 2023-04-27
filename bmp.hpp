#ifndef BMP_HPP
#define BMP_HPP

#include <vector>
#include <fstream>
#include <sstream>
#include <ostream>
#include <cstdint>
#include <cstdlib>

#include "pixel.hpp"

/*/
 *	Header file for reading, modifying, and writing bitmap files
 *	For use in my WR327 Project 1 - Extended Definition of Least-Significant-Bit Image Steganography
 *
 *	BMP header info found from https://solarianprogrammer.com/2018/11/19/cpp-reading-writing-bmp-images/ 
 *
/*/

#define STRIDE_ALIGN 4

// These two macros are taken from JOS from Operating Systems II
#define ROUNDDOWN(a, n) ({ \
	uint32_t __a = (uint32_t)(a); \
	(__typeof__(a))(__a - __a % (n)); \
})
#define ROUNDUP(a, n) ({ \
	uint32_t __n = (uint32_t)(n); \
	(__typeof__(a))(ROUNDDOWN((uint32_t)(a) + __n - 1, __n)); \
})

struct bmp_file_header {
	
	uint16_t file_type{0x4D42};	// "BM"
	uint32_t file_size{0};		// File size in bytes
	
	uint16_t reserved1{0};		// Always 0
	uint16_t reserved2{0};		// Always 0
	
	uint32_t offset_data{0};	// Start position of pixel data
	
	std::string to_string() const;
	
} __attribute__((packed));

inline std::ostream &operator<<(std::ostream &os, const bmp_file_header &file_header) {
	
	os << file_header.to_string();
	return os;
	
}

struct bmp_info_header {
	
	uint32_t size{0};				// Size of the info header in bytes
	
	int32_t  width{0};				// Width in pixels
	int32_t  height{0};				// Height in pixels (positive, origin in bottom left; negative, origin in top left)
	
	uint16_t planes{1}; 			// Target device planes, always 1
	uint16_t bit_count{0};			// Bits per pixel
	
	uint32_t compression{0};		// 0 or 3 for uncompressed
	uint32_t size_image{0};			// 0 for uncompressed
	
	int32_t  x_pixels_per_meter{0};	
	int32_t  y_pixels_per_meter{0};	
	
	uint32_t colors_used{0};		// Number of color indices in the color table - 0 for max colors allowed by BPP
	uint32_t colors_important{0};	// Number of colors used in the bitmap - 0 for all colors required
	
	std::string to_string() const;
	
} __attribute__((packed));

inline std::ostream &operator<<(std::ostream &os, const bmp_info_header &info_header) {
	
	os << info_header.to_string();
	return os;
	
}

struct bmp_color_header {
	
	uint32_t red_mask{0x00FF0000};			// Masks for each color channel
	uint32_t green_mask{0x0000FF00};
	uint32_t blue_mask{0x000000FF};
	uint32_t alpha_mask{0xFF000000};
	
	uint32_t color_space_type{0x73524742};	// "sRGB"
	uint32_t unused[16]{0};					// Unused data for sRGB color space
	
	std::string to_string() const;
	
	bool operator==(const bmp_color_header &color_header) const;
	
} __attribute__((packed));

inline std::ostream &operator<<(std::ostream &os, const bmp_color_header &color_header) {
	
	os << color_header.to_string();
	return os;
	
}

class bmp_file {
	
public:
	
	// Create BMP of given dimensions
	bmp_file(int32_t bmp_width, int32_t bmp_height, bool has_alpha = false);
	
	// Read from file
	bmp_file(const char *read_file);
	
	int8_t read(const char *read_file);
	int8_t write(const char *write_file) const;
	
	size_t size() const;
	uint32_t width() const;
	uint32_t height() const;
	
	// Read/write a given pixel
	pixel get_pixel(uint32_t x, uint32_t y) const;
	void set_pixel(uint32_t x, uint32_t y, pixel p);
	
	uint8_t operator[](uint32_t byte_index) const;
	uint8_t &operator[](uint32_t byte_index);
	
	std::string to_string() const;
	
private:
	
	bmp_file_header file_header;
	bmp_info_header info_header;
	bmp_color_header color_header;
	
	std::vector<uint8_t> data;
	uint32_t row_stride{0};
	
	bool standard_color_header() const;
	
};

inline std::ostream &operator<<(std::ostream &os, const bmp_file &b_file) {
	
	os << b_file.to_string();
	return os;
	
}

#endif