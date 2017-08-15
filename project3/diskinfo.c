#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

int attribute[242];
int attribute_count=0;
int name[242];
int name_count=0;
void getAttributeName(char *mapping){
	int bytes_per_sector = getTwoBytes(mapping,11);
	int i;
	int stop_loop=0;
	for(i=19; i<33 && stop_loop==0; i++){
		int j;
		for(j=0; j<16 && stop_loop==0; j++){
			if(mapping[(i * bytes_per_sector) + (j * 32) ]==0x00){
				stop_loop=1;
				break;
			}
			attribute[attribute_count++]=mapping[(i * bytes_per_sector) + (j * 32) + 11];
			name[name_count++]=mapping[(i * bytes_per_sector) + (j * 32)];
		}
	}
}
int getTwoBytes(char *mapping, int offset) {
	return mapping[offset]+(mapping[offset+1]<<8);
}

void getosname(char *os_name, char *mapping){
	int i=0;
    	for(i=0;i<8;i++){
        	os_name[i]=mapping[i+3];
    	}
}


void getLabelName(char *label, char *mapping){
    	if (mapping[43] != ' '){
		int i;
		for(i=0;i<11;i++){
            		label[i]=mapping[i+43];
		}
	}else{
		int i;
		for(i=0; i<224; i++){
			if(name[i]==0x00){
				return ;
			}
			if ((attribute[i] & 0x08) == 0x08 && (attribute[i] & 0x0F) != 0x0F){
				int j;
				for(j = 0; j< 11; j++) {
					label[j] = name[j];
				}
				
				return;
			}
		}
	}


}

int getTotalSize(char *mapping){
    	int total_sector_count=getTwoBytes(mapping,19);
    	int size_per_sector=getTwoBytes(mapping,11);
    	int totalsize=total_sector_count*size_per_sector;
    	return totalsize;
}

int getFreeSize(int diskSize, char *mapping){
    	int free_sector_count=0;
    	int total_sector=getTwoBytes(mapping,19);
    	int i;
    	for(i=2; i<=total_sector-1-33+2; i++){
    		int *a = malloc(sizeof(int));
		int *b = malloc(sizeof(int));
		int result = 0;
		if (i % 2 == 0) {
			*a = mapping[(3*i)/2+512];
			*b = mapping[(3*i)/2+512+1];
			result = *a+ ( (*b & 0x0F)<< 8);
		// If the logical number is odd a >> 4 + b << 4
		} else {
			*a = mapping[(3*i)/2+512];
			*b = mapping[(3*i)/2+512+1];

			result = ((*a & 0xF0) >> 4) + (*b << 4);
		}
		if (result==0x000){
            		free_sector_count++;
   		}
	}
	return free_sector_count*512;


}

int getRootFileNum(char *mapping){
	int count = 0;
	int i;
	for (i = 0; i <= 242; i ++) {
		if(name[i]==0x00){
			return count;
		}
		if (attribute[i] & (0x08 | 0x10)) {
			count++;
		}
	}
	
	return count;
}

int getFATCopyNum(char *mapping){
    	return mapping[16];
}
int getSectorPerFat(char *mapping){
    	return getTwoBytes(mapping,22);
}

int main(int argc, char *argv[]){

    if(argc != 2){
		printf("Invalid argument. Please use: diskinfo <file system image>\n");
		return -1;
    }

    	char *fileName=argv[1];	
    	char *diskLabel=malloc(sizeof(char) * 11);
    	char *osName=malloc(sizeof(char) * 8);
    	int diskTotalSize= 0;
    	int diskFreeSize = 0;
    	int RootFiles_num = 0;
    	int FATCopies_num = 0;
    	int sectors_per_fat = 0;
    	char *map;
	struct stat fs;
    	int fd= open(fileName, O_RDONLY);
    	if(fd){
    		if(fstat(fd, &fs)==-1){
            		printf("Invalid file stat\n");
            		close(fd);
            		exit(0);
        	}	
        	map = mmap(NULL, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
        	getosname(osName,map);   
		getAttributeName(map);
        	getLabelName(diskLabel,map);
        	diskTotalSize=getTotalSize(map);
        	diskFreeSize = getFreeSize(diskTotalSize,map);
        	RootFiles_num=getRootFileNum(map);
        	FATCopies_num =getFATCopyNum(map);
        	sectors_per_fat =getSectorPerFat(map);
    	}
    	else{
        	printf("Failed in opening the file");
        	exit(0);
    	}
    	printf("OS Name: %s\n", osName);
	printf("Label of the disk: %s\n", diskLabel);
	printf("Total size of the disk: %d bytes\n", diskTotalSize);
	printf("Free size on the disk: %d bytes\n", diskFreeSize);
	printf("==========================================\n");
	printf("Number of files in the root directory: %d\n", RootFiles_num);
	printf("==========================================\n");
	printf("Number of FAT copies: %d\n", FATCopies_num);
	printf("Sectors per FAT: %d\n", sectors_per_fat);

	munmap(map, fs.st_size);
	close(fd);
	return 0;
}


   

