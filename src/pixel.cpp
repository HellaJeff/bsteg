#include "pixel.hpp"

pixel::pixel() {}

pixel::pixel(uint8_t init_r, uint8_t init_g, uint8_t init_b, uint8_t init_a) {
	this->set_argb(init_a, init_r, init_g, init_b);
}

pixel::pixel(uint32_t init_argb) {
	this->set_argb(init_argb);
}

// Access color components by name
uint8_t &pixel::alpha() 	{return colors[COMP_ALPHA];}
uint8_t &pixel::red() 		{return colors[COMP_RED];}
uint8_t &pixel::green() 	{return colors[COMP_GREEN];}
uint8_t &pixel::blue() 	{return colors[COMP_BLUE];}

uint32_t pixel::get_argb() const {
	return *(uint32_t *)this->colors;
}
uint32_t pixel::get_rgb() const {
	return this->get_argb() & 0x00FFFFFF;
}

void pixel::set_argb(uint32_t new_argb) {
	*(uint32_t *)this->colors = new_argb;
}

void pixel::set_argb(uint8_t new_a, uint8_t new_r, uint8_t new_g, uint8_t new_b) {
	this->colors[COMP_ALPHA] = new_a;
	this->colors[COMP_RED] = new_r;
	this->colors[COMP_GREEN] = new_g;
	this->colors[COMP_BLUE] = new_b;
}

void pixel::set_rgb(uint8_t new_r, uint8_t new_g, uint8_t new_b) {
	this->set_argb(this->colors[COMP_ALPHA], new_r, new_g, new_b);
}

void pixel::set_rgb(uint32_t new_rgb) {
	this->set_argb((uint32_t)(this->colors[COMP_ALPHA] << 24) | new_rgb);
}