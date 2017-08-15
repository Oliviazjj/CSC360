#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

int getTwoBytes(char *mapping, int offset) {
	int *tmp1 = malloc(sizeof(int));
	int *tmp2 = malloc(sizeof(int));
	int result;	
	* tmp1 = (unsigned char) mapping[offset];
	* tmp2 = (unsigned char) mapping[offset + 1];
	result = *tmp1 + ((*tmp2) << 8);
	free(tmp1);
	free(tmp2);	
	return result;
}

int getFourBytes(char *mapping, int offset){
	int *tmp1 = malloc(sizeof(int));
	int *tmp2 = malloc(sizeof(int));
	int *tmp3 = malloc(sizeof(int));
	int *tmp4 = malloc(sizeof(int));
	int result;	
	* tmp1 = (unsigned char) mapping[offset];
	* tmp2 = (unsigned char) mapping[offset + 1];
	* tmp3 = (unsigned char) mapping[offset + 2];
	* tmp4 = (unsigned char) mapping[offset + 3];
	result = *tmp1 + ((*tmp4) << 16) + ((*tmp3) << 12) + ((*tmp2) << 8);	
	free(tmp1);
	free(tmp2);
	free(tmp3);
	free(tmp4);	
	return result;
}

int getFileSize(char *mapping,int offset){
	return getFourBytes(mapping, offset+28);
}

void getName(char *name, char *mapping, int offset){
	int i=0;
	char *file_name=malloc(sizeof(char) * 8);
	while(i<8 && mapping[offset+i]!=' '){
		file_name[i] = mapping[offset + i];
		i++;
	}

	if(mapping[offset+8]!=' '){
		file_name[i++]='.';
		int j=0;
		while(j<3 && mapping[offset+8+j]!=' '){
			file_name[i]=mapping[offset+8+j];
			i++;
			j++;
		}
	}
	file_name[i]='\0';
	int j;
	for(j=0; j<i; j++){
		name[j]=file_name[j];
	}
	free(file_name);
}
int getNextEntry(char *mapping, int start_point){
	if(start_point&1){
		return (mapping[512+start_point*3/2]>>4 | mapping[512+start_point*3/2+1]<<4);
	}else{
		return (mapping[512+start_point*3/2]| mapping[512+start_point*3/2+1]&0x0F<<8);
	}
	return -1;
}

void getFile(char *mapping, char *f){
	int offset=0;
	int bytes_per_sector  = getTwoBytes(mapping,11);
	int i;
	int found= 0;
	int found_offset=0;
	int stop_loop=0;
	char *name=malloc(sizeof(char) * 20);
	for (i = 19; i <= 32 && found==0 && stop_loop==0; i ++) {
		// Directory entries are 32 bytes long
		int j = 0;
		for (j = 0; j < 16 && found==0 && stop_loop==0; j++) {
			offset = (i * bytes_per_sector) + (j * 32);
			if(mapping[offset]==0x00){
				stop_loop=1;
				break;
			}
			getName(name, mapping, offset);
			if(strcmp(name,f)==0){
				found_offset=offset;
				found=1;
				break;
			}
		}
	}
	if(found==0){
		printf("File Not Found\n");
		exit(0);
	}
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    	int fd = open(name, O_WRONLY | O_CREAT, mode);
    	if (fd == -1)
    	{
        	printf("Failed in reading file\n");
        	exit(0);
    	}
	offset=found_offset;
	int file_size=getFileSize(mapping,offset);
	int total_sector_count=getTwoBytes(mapping,19);
	int fatEntry=getTwoBytes(mapping, 0x1a);
	while(fatEntry < total_sector_count)
    	{
        	/* physical offset from disk */
        	int physical_offset = (fatEntry + 33 - 2) * 512;
        	/* next FAT entry index from this entry */
        	int nextFatEntry = getNextEntry(mapping, fatEntry);
		if(nextFatEntry==-1){
			printf("No more free entry\n");
			return;
		}
        	int write_size = (file_size > 512) ? 512 : file_size;
        	int status = write(fd, &mapping[physical_offset], write_size);
        	file_size -= 512;
        	if(status == -1){
            		close(fd);
            		exit(EXIT_FAILURE);
        	}
        	fatEntry = nextFatEntry;
    	}
	close(fd);

}

int main(int argc, char *argv[]){

    	if(argc != 3){
		printf("Invalid argument. Please use: diskinfo <file system image>\n");
		return -1;
    	}

    	char *fileName=argv[1];
	int fd= open(fileName, O_RDONLY);
	struct stat fs;
	char *map;
    	if(fd){
    		if(fstat(fd, &fs)==-1){
            		printf("Invalid file stat\n");
            		close(fd);
            		exit(0);
        	}
        	map = mmap(NULL, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
        	getFile(map,argv[2]);
    	}
    	else{
        	printf("Failed in opening the file\n");
        	exit(0);
    	}

	munmap(map, fs.st_size);
	close(fd);
	return 0;


}
