#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "fileinfo.h"

#define OPTIONS "hvds:i:o:"
/*/
 * h: help (exits)
 * v: verbose flag
 * s: steg file (file to hide)
 * i: img file (file to hide in)
 * o: output file
 * d: decode flag
/*/

/*/
 * Read in a file to implant into a bitmap
 * Test both sizes, insert only if enough room in bitmap
 * Then, create a decoder that takes the original and the new files
 * Reconstruct the hidden file from this
 * Insert a "header" as well, consisting of the filename of the hidden file
/*/

//Compile line:
//gcc -Wall -g -o bsteg bsteg.c fileinfo.h

static uint8_t verbose;

int32_t main(int32_t argc, char **argv) {
	
	uint8_t decode;
	
	int32_t opt;
	
	file_info_t *file_steg, *file_img, *file_out;
	
	static struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"verbose", no_argument, NULL, 'v'},
		{"decode", no_argument, NULL, 'd'},
		{"steg", required_argument, NULL, 's'},
		{"img", required_argument, NULL, 'i'},
		{"out", required_argument, NULL, 'o'},
		{0, 0, 0, 0}
	};
	
	//Help text for help flag and invalid arguments
	static char *help_msg = "Bitmap Steganography for UNIX/Linux - bsteg\n\n"
							"Used to hide a file inside the LSBs of a bitmap file, or to decode a hidden file from two similar bitmap images.\n\n"
							"Usage: %s [{-s|--steg}  <steg_file>] [{-i|--img} <bitmap_file>] [{-o|--out} <out_file>] [{-d|--decode}] [{-v|--verbose}] [{-h|--help}]\n\n"
							"{-s|--steg}    : specify the file to hide. When used with {-decode|-d}, specify the edited bitmap file to extract data from.\n"
							"{-i|--img}     : specify the image file used to hide the steg file. When used with {-decode|-d}, specify the original bitmap file for comparison.\n"
							"{-o|--out}     : specify the file to save results to.\n"
							"{-d|--decode}  : extract a hidden file by comparing an edited bsteg bitmap to the unedited image.\n"
							"{-v|--verbose} : send verbose output to stderr.\n"
							"{-h|--help}    : print a help message.\n\n";
							
	//Example text for help flag
	static char *ex_msg =   "Examples:\n\n"
							"%s --steg text_doc.txt --img img_file.bmp --out hidden_file.bmp --verbose\n"
							"%s --steg hidden_file.bmp --img img_file.bmp --out decoded_file.txt --decode\n"
							"%s -s small_image.bmp -i big_img.bmp -o new_img.bmp -v\n"
							"%s -s new_img.bmp -i big_img.bmp -o decoded_img.bmp -d\n\n";
	
	//Allocate memory for simple file info (name + descriptor)
	file_steg = fi_new();
	file_img = fi_new();
	file_out = fi_new();
	
	//Set a default file name for the out file
	file_out->name = "hidden_file.bmp";
	
	//Default to "encode" rather than "decode"
	decode = 0;
	
	//Handle command-line arguments
	while((opt = getopt_long(argc, argv, OPTIONS, long_options, NULL)) != -1) {	
	
		switch(opt) {
			
			case 0:
			
				printf("0");
				break;
				
			case 'h':
				
				printf(help_msg, *argv);
				printf(ex_msg, *argv, *argv, *argv, *argv);
				exit(0);
				break;
			
			case 'v':
			
				verbose = 1;
				break;
				
			case 'd':
			
				decode = 1;
				break;
				
			case 's':
			
				file_steg->name = optarg;
				break;
				
			case 'i':
			
				file_img->name = optarg;
				break;
				
			case 'o':
			
				file_out->name = optarg;
				break;
							
				
			default:
			
				fprintf(stderr, "Invalid argument received\n");
				fprintf(stderr, help_msg, *argv);
				exit(1);
				break;
			
		}
	
	}
	
	//Check to ensure steg file is specified
	if(!file_steg->name) {
		
		//Free allocated file info structs
		fi_free(file_steg);
		fi_free(file_img);
		fi_free(file_out);
		
		fprintf(stderr, "Please specify a file to hide ({-s|--steg} <file>).\n");
		exit(2);
		
	}
	
	//Open steg file as read-only
	file_steg->fd = open(file_steg->name, O_RDONLY);
	
	if(file_steg->fd < 0) {
		
	}
	
	//Check to ensure img file is specified
	if(!file_img->name) {
		
		close(file_steg->fd);
		
		//Free allocated 
		fi_free(file_steg);
		fi_free(file_img);
		fi_free(file_out);
		
		fprintf(stderr, "Please specify a bitmap to hide data in ({-i|--img} <file>).\n");
		exit(2);
		
	}
	
	file_img->fd = open(file_img->name, O_RDWR);
	
	if(verbose) {
		fprintf(stderr, "Files:\n\tSteg file: %s\n\tSize: %ld\n\tImg file: %s\n\tSize: %ld\n\tOut file: %s\nDecode flag: %s\n", file_steg->name, fi_file_size(*file_steg), file_img->name, fi_file_size(*file_img), file_out->name, decode ? "True" : "False");
	}
	
	if(decode) {
		//Extract file in file_steg from file_img
	}
	else {
		//Implant file in file_steg to file_img
	
		//Check if the img file has enough room
		
		//
	
	}
	
}












