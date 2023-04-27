#ifndef PIXEL_HPP
#define PIXEL_HPP

#include <cstdint>

#define COMP_ALPHA 0
#define COMP_RED 1
#define COMP_GREEN 2
#define COMP_BLUE 3

class pixel {

public:
	pixel();
	pixel(uint8_t init_r, uint8_t init_g, uint8_t init_b, uint8_t init_a = 0xFF);
	pixel(uint32_t init_argb);
	
	// Access color components by name
	uint8_t &alpha();
	uint8_t &red();
	uint8_t &green();
	uint8_t &blue();
	
	uint32_t get_argb() const;
	uint32_t get_rgb() const;
	
	void set_argb(uint32_t new_argb);
	void set_argb(uint8_t new_a, uint8_t new_r, uint8_t new_g, uint8_t new_b);

	void set_rgb(uint8_t new_r, uint8_t new_g, uint8_t new_b);
	void set_rgb(uint32_t new_rgb);
	
private:
	uint8_t colors[4] = {0};
	
};

#endif