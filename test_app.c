#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

int8_t write_buf[1024];
int8_t read_buf[1024];

int main()
{
	int fd;
	char option;
/*
function	:int open(char *name,int flags,int perms);
des		:to open a file
input param	:char *name - character string containing the filename
		 int flags  - Specifies how the file is to be open
		            - O_RDONLY open for reading only
			      O_WRONLY open writing only
			      O_RDWR open for both reading and writing
		 Note:These above const are defined in <fcntl.h>
		 int perms - 9 bits of permission information associated with
		 the file.
output param	:Returns a file descriptor on success and -1 for errors

*/	

	fd = open("/dev/myBlockDriver",O_RDWR);
	if(fd<0){
		printf("cannot open device file:%d...\n",fd);
		return 0;
	}

	while(1){
		printf("Pls enter option 1.Write 2.Read 3.Exit");
		scanf(" %c",&option);
		getchar();
		printf("\nOption = %c",option);
		
		switch(option){
			case '1':	printf("Enter the string to write in driver:");
					scanf("%[^\t\n]s",write_buf);
/*
function	:int n_written = write(int fd,char *buf,int n);
des		:To write to a file
input param	:int fd-file descriptor
		 char *buf - character array in the program from where the data goes...
		 int n - number of bytes to be transferred
output param	:int n_written - count of the number of bytes transferred.
*/
					write(fd,write_buf,strlen(write_buf)+1);
					break;
					
			case '2':	printf("Data reading\n");
/*
function        :int n_read= read(int fd,char *buf,int n);
des             :To write to a file
input param     :int fd-file descriptor
                 char *buf - character array in the program where the data comes after reading.
                 int n - number of bytes to be transferred
output param    :int n_read - count of the number of bytes transferred.
*/

					read(fd,read_buf,1024);
					printf("Data = %s\n\n",read_buf);
					//getchar();
					break;
		
			case '3':	close(fd);
					exit(1);
					break;	
					
			default:	printf("Enter a valid option");
				}
		close(fd);
		fd = open("/dev/myBlockDriver",O_RDWR);
			if(fd<0){
				printf("cannot open device file....\n");
				exit(1);
			}
		}

close(fd);

}
