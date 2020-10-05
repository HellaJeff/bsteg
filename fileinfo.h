#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//Provides very basic file info for ease of use

typedef struct file_info_d {
	char *name;
	int32_t fd;
} file_info_t;

file_info_t *fi_new(void);
void fi_free(file_info_t *);
size_t fi_file_size(file_info_t);

//Create a blank file_info_t
file_info_t *fi_new(void) {
	
	//Allocate memory (must be freed with fi_free or with manual calls to free)
	file_info_t *fi = (file_info_t *)malloc(sizeof(file_info_t));
	
	//Set the initial values so missing assignments can be caught
	fi->name = NULL;
	fi->fd = -1;
	
	return fi;
	
}

//Free a file_info_t
void fi_free(file_info_t *fi) {
	
	//Check to make sure fi is not null
	if(fi) {
		
		//Check to make sure fi->name is not null
		if(fi->name) {
			free(fi->name);
		}

		free(fi);
		
	}
	
}

//Get a file's size in bytes
size_t fi_file_size(file_info_t fi) {
	
	size_t fsize;
	
	struct stat finfo;
	
	//Get file info for the given fd
	fstat(fi.fd, &finfo);
	
	//Store the found file size into the return variable
	fsize = finfo.st_size;
	
	return fsize;

}