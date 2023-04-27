#include "steg.hpp"

// Total needed data: 3 bytes (determine bitness) + (4 + data.size()) * 8 / bits
bmp_file hide_data(bmp_file orig_file, std::vector<uint8_t> data, uint8_t bits) {

	VERBOSE_LOG("Begin encoding");
	
	if(bits > 8 && bits)
		throw std::runtime_error("Bit count exceeds bits per channel.");
	
	size_t image_bytes = orig_file.size();
	// Image bytes needed to store this data:
	//	3 + the ceiling of 8 / bits (image bytes needed for each data byte) times 4 + the data size
	//  3 -> bytes needed to decode the bitness
	//  4 -> 32 bits used to determine the data size on decode
	size_t bytes_needed = 3 + (size_t)std::ceil((4 + data.size()) * (8.f / bits));
	
	// Check if we have the space needed to store this document in this image's lowest n bits
	if(bytes_needed > image_bytes) {
		
		std::stringstream err_s_str;
		
		err_s_str << "Not enough space in this image (" << image_bytes << ") to store this data set (" << bytes_needed << " bytes needed) for " << bits << " bits.";
		
		throw std::runtime_error(err_s_str.str());
		
	}
	
	uint32_t byte_cursor = 0;
	
	// Set the first three image bytes' least significant bits such that they will encode the bitness
	while(byte_cursor < 3) {
		orig_file[byte_cursor] &= 0xFE;
		orig_file[byte_cursor] |= (bits >> (2 - byte_cursor)) & 1;
		byte_cursor++;
	}	
	
	// Grab the data size in a uint32_t so we can put it at the beginning of the data set
	uint32_t data_size = data.size();
	
	VERBOSE_LOG("Data size: " << data_size);
	
	// Put the data size at the beginning of the data set
	data.insert(data.begin(), (uint8_t)((data_size) & 0xFF));
	data.insert(data.begin(), (uint8_t)((data_size >> 8) & 0xFF));
	data.insert(data.begin(), (uint8_t)((data_size >> 16) & 0xFF));
	data.insert(data.begin(), (uint8_t)(data_size >> 24));
	
	// Bitmask used to get the lowest n bits
	// Complement can be used to get the highest 8 - n bits
	uint8_t bitmask = (1 << bits) - 1;
	
	// Keep track of how many bits are still available to use in this image byte
	uint8_t image_bits_remaining = bits;
	// Clear the lower n bits of this image byte for storing data
	orig_file[byte_cursor] &= ~bitmask;
	
	// Keep track of how many data bits are remaining in this data byte
	uint8_t data_bits_remaining;

	// Insert each data byte into the image
	for(uint32_t c = 0; c < data.size(); c++) {
		
		data_bits_remaining = 8;
		
		if(image_bits_remaining < bits) {
			
			// Get the top k bits that will still fit in this channel's byte
			// Store those in the lowest k bits of the channel's byte and mark to get the next image byte after
			orig_file[byte_cursor] |= (data[c] >> (data_bits_remaining - image_bits_remaining));
			
			data_bits_remaining -= image_bits_remaining;
			
			// Grab the next image byte
			byte_cursor++;
			image_bits_remaining = bits;
			
			// Clear the lowest n bits for writing
			orig_file[byte_cursor] &= ~bitmask;
			
		}
		
		// Loop until we have written this data byte's full contents
		do {
		
			// If our image byte does not have more space than we have data to write, write what we can and move to the next image byte
			if(data_bits_remaining >= image_bits_remaining) {
			
				// If we still have space in this image byte, write what we can of our data
				if(image_bits_remaining) {
					
					orig_file[byte_cursor] |= (data[c] >> (data_bits_remaining - bits)) & bitmask;
					
					data_bits_remaining -= image_bits_remaining;
					
				}
			
				byte_cursor++;
				image_bits_remaining = bits;
				
				orig_file[byte_cursor] &= ~bitmask;
			
			}
			// Otherwise, write the remainder of our data bits and exit the loop
			else {
				
				orig_file[byte_cursor] |= (data[c] & ((1 << data_bits_remaining) - 1)) << (image_bits_remaining - data_bits_remaining);
				
				// Used to indicate that there are still bits left in this image byte that can be used by the next data byte
				image_bits_remaining -= data_bits_remaining;
				
				break;
				
			}
			
		} while(data_bits_remaining);
		
	}
	
	VERBOSE_LOG("Finished encoding");
	
	return orig_file;
	
}

bmp_file hide_data(bmp_file orig_file, std::vector<uint8_t> data) {

	VERBOSE_LOG("Determining minimum bit count");

	double bit_count = 3 + ((data.size() + 4) << 3);
	
	VERBOSE_LOG("Total bits: " << bit_count);
	
	uint64_t bits_needed = (uint64_t)std::ceil(bit_count / orig_file.size());
	
	VERBOSE_LOG("Bits needed per byte: " << bits_needed);
	
	if(bits_needed < 8)
		return hide_data(orig_file, data, bits_needed);
	else
		throw std::runtime_error("Image is too small to store this data set.");

}

std::vector<uint8_t> extract_data(bmp_file modified_file) {
	
	VERBOSE_LOG("Begin extracting");
	
	uint8_t encoding_bits = 0;
	size_t byte_cursor = 0;
	
	while(byte_cursor < 3) {
		encoding_bits <<= 1;
		encoding_bits |= modified_file[byte_cursor++] & 1;
	}
	
	VERBOSE_LOG("Bits used in encoding: " << (uint16_t)encoding_bits);
	
	// Initial image bits
	uint8_t image_bits_remaining = encoding_bits;
	
	// Start off with as many bits as are needed to store the size of this data set (default 8 * 4 = 32)
	uint8_t data_bits_remaining = sizeof(uint32_t) << 3;
	// Bitmask for getting the lowest n bits
	uint8_t bitmask = (1 << encoding_bits) - 1;
	// Initialize data_size to 0 so we can shift and OR to get the size from the encoded data
	uint32_t data_size = 0;
	
	do {
		
		// Check if the data_size variable can still fit all n least significant bits
		if(data_bits_remaining >= encoding_bits) {
			
			// Shift left by n to make room for the next n bits
			data_size <<= encoding_bits;
			
			// Put the lowest n bits into data_size
			data_size |= modified_file[byte_cursor++] & bitmask;
			
			// Subtract from the remaining data bits
			data_bits_remaining -= encoding_bits;
			
		}
		else {
			
			// Make room for the last bits we need at the end of data_size
			data_size <<= data_bits_remaining;
			
			// Grab only the top n - remaining bits from this image byte into the end of the data_size
			data_size |= (modified_file[byte_cursor] & bitmask) >> (encoding_bits - data_bits_remaining);
			
			// Subtract the last used bits from our remaining image bits so we don't grab them again in the start of the first data byte
			image_bits_remaining -= data_bits_remaining;
			
			data_bits_remaining = 0;
			
		}
		
	// Loop until we have filled our data_size
	} while(data_bits_remaining);
	
	VERBOSE_LOG("Data size: " << data_size);
	
	// Set our vector to the size of our data to extract
	std::vector<uint8_t> extracted_data(data_size);
	
	// Decode every data byte
	for(size_t c = 0; c < data_size; c++) {
		
		// This should already be 0, maybe unnecessary
		extracted_data[c] = 0;
		// Reset the number of data bits remaining
		data_bits_remaining = 8;	
		
		// Grab any leftover image bits from the last data byte
		if(image_bits_remaining < encoding_bits) {
			
			extracted_data[c] |= modified_file[byte_cursor++] & ((1 << image_bits_remaining) - 1);
			
			data_bits_remaining -= image_bits_remaining;
			
			image_bits_remaining = encoding_bits;
			
		}
		
		do {
			
			// If there is enough room in this data byte to store the remainder of this image byte's data, extract it and move to the next image byte
			if(data_bits_remaining >= image_bits_remaining) {
				
				if(image_bits_remaining) {
					
					extracted_data[c] <<= image_bits_remaining;	
					extracted_data[c] |= modified_file[byte_cursor] & bitmask;
					
					data_bits_remaining -= image_bits_remaining;
					
				}
				
				byte_cursor++;
				image_bits_remaining = encoding_bits;
				
			}
			// Otherwise, extract the top k bits that will fit in this data byte and move onto the next data byte
			else {
				
				extracted_data[c] <<= data_bits_remaining;
				extracted_data[c] |= (modified_file[byte_cursor] & bitmask) >> (image_bits_remaining - data_bits_remaining);
				
				image_bits_remaining -= data_bits_remaining;
				
				data_bits_remaining = 0;
				
			}
			
		} while(data_bits_remaining);
		
	}
	
	VERBOSE_LOG("Finished extracting");
	
	return extracted_data;
	
}
