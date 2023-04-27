#ifndef STEG_HPP
#define STEG_HPP

#include <cmath>

#include "bmp.hpp"

// Include logging headers and a logging macro only when PROG_VERBOSE is defined
#ifdef PROG_VERBOSE
	#include <iostream>
	#include <bitset>
	#define VERBOSE_LOG(a) (std::cout << a << '\n')
#else
	#define VERBOSE_LOG(a) {}
#endif

bmp_file hide_data(bmp_file orig_file, std::vector<uint8_t> data, uint8_t bits);
bmp_file hide_data(bmp_file orig_file, std::vector<uint8_t> data);

std::vector<uint8_t> extract_data(bmp_file modified_file);

#endif