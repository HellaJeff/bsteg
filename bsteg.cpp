#include <iostream>
#include <getopt.h>

#include "src/steg.hpp"

#define OPTIONS "i:d:o:b:vh"

static option cli_options[] = {
	{"image", 	required_argument, 	NULL, 'i'},
	{"data", 	required_argument, 	NULL, 'd'},
	{"output", 	required_argument, 	NULL, 'o'},
	{"bits", 	required_argument, 	NULL, 'b'},
	{"verbose",	no_argument,		NULL, 'v'},
	{"help", 	no_argument, 		NULL, 'h'},
	{0, 0, 0, 0}
};

uint8_t verbose = 0;

int32_t main(int32_t argc, char **argv) {
	
	int32_t opt;
	
	std::string input_image_filename, input_data_filename, output_file_filename;
	uint8_t n_bits = 0;
	
	// Parse command-line arguments
	while((opt = getopt_long(argc, argv, OPTIONS, cli_options, NULL)) != -1) {
		
		switch(opt) {
			
			case 'i':
			
				input_image_filename = optarg;
				break;
				
			case 'd':
			
				input_data_filename = optarg;
				break;
				
			case 'o':
			
				output_file_filename = optarg;
				break;
				
			case 'b':
			
				n_bits = std::strtoul(optarg, nullptr, 0);
				
				if(n_bits > 7) {
					std::cerr << "Only up to 7 least-significant bits are supported for writing.\n";
					return 2;
				}
				
				break;
				
			case 'v':
			
				verbose++;
				break;
				
			case 'h':
			
				std::cout << argv[0] << " help\n" <<
					"Least-Significant Bit(s) Bitmap Steganography command-line utility\n" <<
					"Used to store a file of any type inside the n least-significant bits of a standard 24-bit RGB or 32-bit sRGB bitmap (more to come later)\n" <<
					"Usage:\n\t" <<
						argv[0] << " [-i|--image] <image_filename> ([-d|--data] <data_filename>) [-o|--output] <output_filename>  ([-b|--bits] <bit_count>) ([-v|--verbose]) ([-h|--help])\n\t" <<
						"-i -> Specify a bitmap image to use either to encode data into or decode data from.\n\t" <<
						"-d -> Specify the data file to encode into the image file. If omitted, the input image will be decoded.\n\t" <<
						"-o -> Specify an output file to write either the encoded bitmap or the decoded data file.\n\t" <<
						"-b -> Set the number of least significant bits to use in encoding. If omitted, the program will determine the smallest number of LSBs that can be used for the specified image and data set.\n\t" <<
						"-v -> Enable verbose output (not yet implemented).\n\t" <<
						"-h -> Show help text.\n";
				
				return 0;
				
			default:
			
				std::cerr << "Unknown argument received. Use -h for program help.\n";
				return 1;
			
		}
		
	}
	
	if(input_image_filename.empty()) {
		std::cerr << "No input image supplied.\n";
		return 3;
	}
	
	if(output_file_filename.empty()) {
		std::cerr << "No output filename supplied.\n";
		return 4;
	}
	
	bmp_file input_image(input_image_filename.c_str());
	
	// Decode
	if(input_data_filename.empty()) {
		
		// Extract data from the input image
		std::vector<uint8_t> decoded_data = extract_data(input_image);
		
		// Open an output file for writing
		std::fstream output_file(output_file_filename, std::ios::out | std::ios::binary | std::ios::trunc);
		
		// Write decoded data and close
		output_file.write((char *)decoded_data.data(), decoded_data.size());
		output_file.close();
		
	}
	// Encode
	else {
		
		// Open the data file for reading in binary
		std::fstream input_data_file(input_data_filename, std::ios::in | std::ios::binary);
		
		// Move to the end of the file to get its size
		input_data_file.seekg(0, std::ios::end);
		
		// Create a vector to hold as many bytes as the file contains
		std::vector<uint8_t> input_data_vector(input_data_file.tellg());
		
		// Seek back to the beginning of the file
		input_data_file.seekg(0);
		
		// Read the input data
		input_data_file.read((char *)input_data_vector.data(), input_data_vector.size());
		
		// Close the file since we now have its contents in memory
		input_data_file.close();
		
		// Hide the data and write to the output file
		// If a bit count was specified, use that
		if(n_bits) 
			hide_data(input_image, input_data_vector, n_bits).write(output_file_filename.c_str());
		// Otherwise, find the minimum bit count that will allow this data set to fit in this image and use that
		else 
			hide_data(input_image, input_data_vector).write(output_file_filename.c_str());
		
	}
	
	return 0;
	
}