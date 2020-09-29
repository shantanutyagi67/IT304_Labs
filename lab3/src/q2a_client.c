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
#define buff_size 128

int
main(int argc, char **argv)
{
	int socket_value, fd, n, size,count=0;
	long int size1,size2;
	char storage[buff_size], file_name[50], ans[50];
	struct sockaddr_in server_addr;
	struct stat stat_buf;

	if (argc != 2) {
	printf("Usage: %s server_address", argv[0]);
	exit(1);
	}

	printf("11\n");
	if ( (socket_value = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{printf("Socket has not been created.");
		exit(1);}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port   = htons(PORT);
	if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
		exit(1);

	printf("22\n");

	if (connect(socket_value, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
		exit(1);
	printf("connection has been established\n");

	printf("Enter FTP Command: \n");
	printf("Enter GET , PUT or LIST : \n");
	
	scanf("%s",ans);
	send(socket_value,ans,sizeof(ans),0);
		if(strcmp(ans, "PUT") == 0){
		printf("Enter the name of the file to send : ");
		scanf("%s",file_name);
		send(socket_value,file_name,sizeof(file_name),0);
		fd=open(file_name,O_RDONLY,S_IRUSR);
		fstat(fd, &stat_buf);
		size = stat_buf.st_size;
		printf(" size is %d\n", size);
		printf("\nopened file\n");
		while ( (n = read(fd, storage, buff_size-1)) > 0)
		{
			storage[n] = '\0';
			write(socket_value,storage,n);
		}
		printf("file transfer completed \n");
		close(socket_value);
		close(fd);

	}
	
	else if(strcmp(ans, "GET") == 0){
		
		printf("Enter the name of the file u want to receive : ");
	scanf("%s",file_name);
	send(socket_value,file_name,sizeof(file_name),0);

		
	//read(socket_value, size1, sizeof(size1));
	
	//printf("Size of the File is : %d ",size1);
			
	fd=open("NewFile_made_by_get",O_WRONLY|O_CREAT,S_IRWXU);		
	while ( (n = read(socket_value, storage, buff_size-1)) > 0)
	{
		storage[n] = '\0';
		printf("%s\n",storage);	
		write(fd,storage,n);
		if( n < buff_size-2)
			break;
	}
	
	printf("file receiving completed \n");
	close(socket_value);
	close(fd);
	
	}
	else if(strcmp(ans, "LIST") == 0){
		printf("Files in the current directory : \n");
		for(;;){
			memset(&file_name, '\0', 50);
			int temp_1 = recv(socket_value, file_name, 50, 0);
			if(strcmp(file_name, "STOP") == 0){
				printf("No more files exists .\n");
				break;
			}
			printf("%s\n", file_name);
		}
		close(socket_value);
	}
    else{
        printf("Error: Not a valid FTP ans.\n");
        close(socket_value);
    }
	exit(0);
}
