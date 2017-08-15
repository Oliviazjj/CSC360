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
void getTime(char *createdTime, char*mapping, int offset){
	char *datetime=malloc(sizeof(char) * 20);
	int t=getTwoBytes(mapping, offset+14);
	int d=getTwoBytes(mapping, offset+16);
	int date[3];
	date[0] = ((d & 0xFE00) >> 9) + 1980;
	date[1] = (d & 0x01E0) >> 5;
	date[2] = d & 0x1F;
	int time[2];
	time[0] = (t & 0xF800) >> 11;
    	time[1] = (t & 0x07E0) >> 5;
	sprintf(datetime, "%d-%02d-%02d %d:%02d", date[0], date[1], date[2], time[0], time[1]);
	int i;
	for(i=0; i<20;i++){
		createdTime[i]=datetime[i];
	}
}
int main(int argc, char *argv[]){
	if(argc != 2){
            printf("Invalid argument. Please use: diskinfo <file system image>\n");
            return -1;
        }
        char *fileName=argv[1];
	char *map;
        int fd= open(fileName, O_RDONLY);
	struct stat fs;
        if(fd){         
    		if(fstat(fd, &fs)==-1){
            		printf("Invalid file stat\n");
            		close(fd);
            		exit(0);
        	}	
        	map = mmap(NULL, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
		int i;
		int bytes_per_sector = getTwoBytes(map,11);
		int break_flag=0;
		int attribute_value;
		for(i=19; i<33 && break_flag==0; i++){
			if(break_flag==1){
				//break;
			}
			int j;
			for(j=0; j<16; j++){
				attribute_value=map[(i * bytes_per_sector) + (j * 32) + 11];
				if(map[(i * bytes_per_sector) + (j * 32)]==0x00){
					break_flag=1;
					break;
				}
				if( (attribute_value&0x08)!=0x08 && (attribute_value&0x10)!=0x10 && (attribute_value&0x0F)!=0x0F){
					int offset=(i * bytes_per_sector) + (j * 32);
					char type=attribute_value&0x10?'D':'F';
					char name;
					char *fileName=malloc(sizeof(char) * 8);
					char *created_time=malloc(sizeof(char) * 20);
					int fileSize=getFourBytes(map,offset+28);
					getName(fileName,map,offset);
					getTime(created_time,map,offset);
					printf("%c %10u %20s %s\n",type, fileSize,fileName, created_time);
				}	
			}
                	
        	}
        }else{
            printf("Failed in opening the file\n");
            exit(0);
        }
    	munmap(map, fs.st_size);
	close(fd);
	return 0;
}
