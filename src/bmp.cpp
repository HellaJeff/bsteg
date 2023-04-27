#include "bmp.hpp"

/* bmp_file_header */

std::string bmp_file_header::to_string() const {
	
	std::stringstream s_str;
	
	s_str << "File type: " << std::hex << this->file_type << '\n';
	s_str << "File size: " << std::dec << this->file_size << '\n';
	s_str << "Reserved1: " << this->reserved1 << '\n';
	s_str << "Reserved2: " << this->reserved2 << '\n';
	s_str << "Offset at: " << std::hex << this->offset_data << '\n';
	
	return s_str.str();
	
}

/* bmp_info_header */

std::string bmp_info_header::to_string() const {
	
	std::stringstream s_str;
	
	s_str << "Info size: " << std::dec << this->size << '\n';
	s_str << "Image width: " << this->width << '\n';
	s_str << "Image height: " << this->height << '\n';
	s_str << "Planes: " << this->planes << '\n';
	s_str << "BPP: " << this->bit_count << '\n';
	s_str << "Compression: " << this->compression << '\n';
	s_str << "Size image: " << this->size_image << '\n';
	s_str << "X ppm: " << this->x_pixels_per_meter << '\n';
	s_str << "Y ppm: " << this->y_pixels_per_meter << '\n';
	s_str << "Colors used: " << this->colors_used << '\n';
	s_str << "Colors important: " << this->colors_important << '\n';
	
	return s_str.str();
	
}

/* bmp_color_header */

std::string bmp_color_header::to_string() const {
	
	std::stringstream s_str;
	
	s_str << "Red mask: " << std::hex << this->red_mask << '\n';
	s_str << "Green mask: " << this->green_mask << '\n';
	s_str << "Blue mask: " << this->blue_mask << '\n';
	s_str << "Alpha mask: " << this->alpha_mask << '\n';
	s_str << "Color space: " << this->color_space_type << '\n';
	
	s_str << "Unused: \n\t";
	for(uint8_t c = 0; c < 15; c++)
		s_str << (uint16_t)this->unused[c] << ' ';
	
	s_str << (uint16_t)this->unused[15] << '\n';
	
	return s_str.str();
	
}

bool bmp_color_header::operator==(const bmp_color_header &color_header) const {
	return (
			(this->color_space_type == color_header.color_space_type) && 
			(this->red_mask == color_header.red_mask) && 
			(this->green_mask == color_header.green_mask) && 
			(this->blue_mask == color_header.blue_mask) && 
			(this->alpha_mask == color_header.alpha_mask)
		);
}

/* bmp_file */

// Create a BMP file in memory from nothing
bmp_file::bmp_file(int32_t bmp_width, int32_t bmp_height, bool has_alpha) {
	
	// Set the width and height immediately
	this->info_header.width = bmp_width;
	this->info_header.height = bmp_height;
	
	// Get the absolute values of the width and height to handle both modes
	uint32_t abs_width = std::abs(bmp_width);
	uint32_t abs_height = std::abs(bmp_height);
	
	// Set the default size of the info header and the default pixel data offset
	this->info_header.size = sizeof(bmp_info_header);
	this->file_header.offset_data = sizeof(bmp_file_header) + sizeof(bmp_info_header);
	
	// Check if we will be using an alpha channel
	if(has_alpha) {
		
		// Add the color header sizes to our info header size and pixel data offset
		this->info_header.size += sizeof(bmp_color_header);
		this->file_header.offset_data += sizeof(bmp_color_header);
		
		// Set our bits per pixel, compression, and row stride accordingly
		this->info_header.bit_count = 32;
		this->info_header.compression = 3;
		this->row_stride = abs_width * 4;
		
		// Set our data size to the number of bytes needed for each row and our absolute height
		this->data.resize(this->row_stride * abs_height);
		
	}
	else {
		
		this->info_header.bit_count = 24;
		this->info_header.compression = 0;
		this->row_stride = abs_width * 3;
		
		this->data.resize(this->row_stride * abs_height);
		
		// Add padding bytes to our file size based on the height and how many padding bytes are needed for each row
		this->file_header.file_size += this->info_header.height * (ROUNDUP(this->row_stride, STRIDE_ALIGN) - this->row_stride);
		
	}
	
	// Add the size of the pixel data and the headers to the file suzem which up to this point was zero or the length of our padding bytes
	this->file_header.file_size += this->data.size() + this->file_header.offset_data;
	
}

// Read a BMP file into memory
bmp_file::bmp_file(const char *read_file) {
	this->read(read_file);
}

int8_t bmp_file::read(const char *read_file) {
	
	// Attempt to open file for binary reading
	std::fstream input_file{read_file, std::ios::in | std::ios::binary};
	if(!input_file.is_open())
		throw std::runtime_error("Unable to open image file for reading.");
	
	// Read the file header, throw an error if the wrong file type is found
	input_file.read((char *)&this->file_header, sizeof(bmp_file_header));
	if(this->file_header.file_type != 0x4D42)
		throw std::runtime_error("Attempting to read unrecognized file format.");
	
	// Read the info header, check if this includes an alpha channel
	input_file.read((char *)&this->info_header, sizeof(bmp_info_header));
	if(this->info_header.bit_count == 32) {
		
		// Check if the info header size is large enough to contain the info header and color header, throw an error if not
		if(this->info_header.size < (sizeof(bmp_info_header) + sizeof(bmp_color_header)))
			throw std::runtime_error("Color header information not found.");
		
		// Read the color header, throw an error if it is non-standard (handle differently later)
		input_file.read((char *)&this->color_header, sizeof(bmp_color_header));
		if(!standard_color_header())
			throw std::runtime_error("Non-standard color header information found. This is currently unsupported.");
		
		// Discard any additional information that might be in the info header
		this->info_header.size = sizeof(bmp_info_header) + sizeof(bmp_color_header);
		
	}
	else
		this->info_header.size = sizeof(bmp_info_header);
	
	// Seek to the pixel data position
	input_file.seekg(this->file_header.offset_data);
	
	// Adjust the data offset to remove any potential extra data that isn't needed to display the bmp
	this->file_header.offset_data = sizeof(bmp_file_header) + sizeof(bmp_info_header);
	if(this->info_header.bit_count == 32) this->file_header.offset_data += sizeof(bmp_color_header);

	// Set the initial size to the data offset, since this covers all of our header sizes
	this->file_header.file_size = this->file_header.offset_data;

	// Set our pixel data vector size accordingly to fit our number of pixels and channels per pixel
	this->data.resize(this->info_header.width * this->info_header.height * (this->info_header.bit_count >> 3));
	// Add the size of the data to our total file size
	this->file_header.file_size += this->data.size();
	
	if(this->info_header.width % 4) {
		
		this->row_stride = this->info_header.width * (this->info_header.bit_count >> 3);
		uint32_t new_stride = ROUNDUP(this->row_stride, STRIDE_ALIGN);
		std::vector<uint8_t> padding_row(new_stride - this->row_stride);
		
		uint32_t abs_height = std::abs(this->info_header.height);
		
		// Read each row into our data array, skipping any padding
		for(uint32_t y = 0; y < abs_height; y++) {
			
			// Read the actual row data, stopping before the padding
			input_file.read((char *)(this->data.data() + (y * this->row_stride)), this->row_stride);
			
			// The data we read isn't important, just need to skip past the padding
			input_file.read((char *)padding_row.data(), padding_row.size());
			
		}
		
		// Add the padding to the file size
		this->file_header.file_size += this->info_header.height * padding_row.size();
		
	}
	else
		input_file.read((char *)this->data.data(), this->data.size());
	
	return 0;
	
}

int8_t bmp_file::write(const char *write_file) const {
	
	std::fstream output_file(write_file, std::ios::out | std::ios::binary | std::ios::trunc);
	if(!output_file.is_open())
		throw std::runtime_error("Unable to open file for writing.");
	
	// Write headers
	output_file.write((const char *)&this->file_header, sizeof(bmp_file_header));
	output_file.write((const char *)&this->info_header, sizeof(bmp_info_header));
	if(this->info_header.bit_count == 32)
		output_file.write((const char *)&this->color_header, sizeof(bmp_color_header));
	else if(this->info_header.bit_count != 24)
		throw std::runtime_error("Only 24/32 BPP is supported currently.");
	
	if(this->info_header.width % 4) {
		
		uint32_t new_stride = ROUNDUP(this->row_stride, STRIDE_ALIGN);
		std::vector<uint8_t> padding_row(new_stride - this->row_stride);
		
		uint32_t abs_height = std::abs(this->info_header.height);
		
		for(uint32_t y = 0; y < abs_height; y++) {
			
			// Read the actual row data, stopping before the padding
			output_file.write((char *)(this->data.data() + (y * this->row_stride)), this->row_stride);
			
			// The data we read isn't important, just need to skip past the padding
			output_file.write((char *)padding_row.data(), padding_row.size());
			
		}
		
	}
	else
		output_file.write((const char *)this->data.data(), this->data.size());
	
	return 0;
	
}

size_t bmp_file::size() const {
	return this->data.size();
}

uint32_t bmp_file::width() const {
	return this->info_header.width;
}

uint32_t bmp_file::height() const {
	return this->info_header.height;
}

pixel bmp_file::get_pixel(uint32_t x, uint32_t y) const {
	
	pixel p;
	
	if(this->info_header.bit_count == 32) {
		// Find the (x, y) position in the array by adding x to y * width
		// Multiply by 4 to account for the color channels
		// Convert to a uint32_t pointer and dereference to get the ARGB value
		p.set_argb(*((uint32_t *)&this->data[(x + y * this->info_header.width) * 4]));
	}
	else {
		uint32_t data_position = (x + y * this->info_header.width) * 3;
		p.set_argb(0, this->data[data_position], this->data[data_position + 1], this->data[data_position + 2]);
	}
	
	return p;
	
}

void bmp_file::set_pixel(uint32_t x, uint32_t y, pixel p) {
	
	if(this->info_header.bit_count == 32) {
		*((uint32_t *)&this->data[(x + y * this->info_header.width) * 4]) = p.get_argb();
	}
	else {
		uint32_t data_position = (x + y * this->info_header.width) * 3;
		this->data[data_position] = p.red();
		this->data[data_position + 1] = p.green();
		this->data[data_position + 2] = p.blue();
	}
	
}

uint8_t bmp_file::operator[](uint32_t byte_index) const {
	return this->data[byte_index];
}
uint8_t &bmp_file::operator[](uint32_t byte_index) {
	return this->data[byte_index];
}

std::string bmp_file::to_string() const {
	
	std::stringstream s_str;
	
	s_str << "Headers: \n";
	s_str << this->file_header << '\n';
	s_str << this->info_header << '\n';
	
	if(this->info_header.bit_count == 32)
		s_str << this->color_header << "\n\n";
	
	s_str << "Pixel count: " << std::dec << this->info_header.width * this->info_header.height << '\n';
	s_str << "Data size: " << this->data.size() << '\n';
	s_str << "Row stride: " << this->row_stride << '\n';
	
	return s_str.str();
	
}

bool bmp_file::standard_color_header() const {
	bmp_color_header std_color_header;
	return this->color_header == std_color_header;
}