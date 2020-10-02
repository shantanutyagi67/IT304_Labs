#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<net/if_arp.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include <stdio.h>

#define FNAME file1
#define PORT 8010
#define BUFSIZE 128

int main(int argc, char **argv)
{
	int sock_fd;
	int fd, n, FINAL_size;
	long int size1,size2;
	char buf_val[BUFSIZE], fname[50], command_1[50];
	struct sockaddr_in servaddr;
	struct stat stat_buf;

	if (argc != 2) {
	printf("%s server_address is being used", argv[0]);
	exit(1);
	}

	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit(1);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		exit(1);

	
	if (connect(sock_fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		exit(1);
	printf("connection has been established\n");

	printf("Enter command (with respect to FTP : GET , PUT , LIST ): ");
	scanf("%s",command_1);
	send(sock_fd,command_1,sizeof(command_1),0);

	if(strcmp(command_1, "GET") == 0){
		
		printf("Enter the name of the file you want to receive : ");		//for GET COMMAND
		scanf("%s",fname);
		send(sock_fd,fname,sizeof(fname),0);

		fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
		
		
		while ( (n = read(sock_fd, buf_val, BUFSIZE-1)) > 0)
		{
			buf_val[n] = '\0';
			write(fd,buf_val,n);
			if( n < BUFSIZE-2)
				break;
		}

		printf("file has been received \n");
		
		close(sock_fd);
		close(fd);
	}
	else if(strcmp(command_1, "PUT") == 0){				// FOR PUT COMMAND
		printf("Enter the name of the file u want to receive : ");
		scanf("%s",fname);
		send(sock_fd,fname,sizeof(fname),0);
		
		
		fd=open(fname,O_RDONLY,S_IRUSR);
		fstat(fd, &stat_buf);
		
		
		FINAL_size = stat_buf.st_size;
		
		
		printf(" size is %d\n", FINAL_size);
		printf("\nfile opened : \n");
		while ( (n = read(fd, buf_val, BUFSIZE-1)) > 0)
		{
			buf_val[n] = '\0';
			write(sock_fd,buf_val,n);
		}
		printf("file transfer has been completed \n");
		close(sock_fd);
		close(fd);

	}
	else if(strcmp(command_1, "LIST") == 0){			// FOR LIST COMMAND
		
		
		printf("Files present in the current directory : \n");
		for(;;)
		{
			
			memset(&fname, '\0', 50);
			int temp = recv(sock_fd, fname, 50, 0);
			
			if(strcmp(fname, "STOP") == 0){
				printf("No more files in the directory.\n");
				break;
			}
			
			printf("%s\n", fname);
		}
		
		close(sock_fd);
	}
    else{
        printf("Error Occured: This is Not a valid FTP command.\n");	 //INVALID COMMAND
        close(sock_fd);
    }
	exit(0);
}
