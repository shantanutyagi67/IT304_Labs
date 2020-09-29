#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<net/if_arp.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>

#define BUFSIZE 128
#define FNAME file01
#define PORT 8080

int main(int argc, char **argv)
{
	int sockfd, n, size, fd,count=0;
	long int size1,size2;
	char buf[BUFSIZE], f_name[100], cmd[100];
	struct sockaddr_in servaddr;
	struct stat stat_buf;

	if (argc != 2) {
	printf("Invalid input format", argv[0]);
	exit(1);
	}

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit(1);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		exit(1);

	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		exit(1);
	printf("Connection has been established\n");

	printf("Enter a FTP command: ");
	scanf("%s",cmd);
	send(sockfd,cmd,sizeof(cmd),0);

	if(strcmp(cmd, "PUT") == 0){
		printf("Name the file you want to send : ");
		scanf("%s",f_name);
		send(sockfd,f_name,sizeof(f_name),0);
		fd=open(f_name,O_RDONLY,S_IRUSR);
		fstat(fd, &stat_buf);
		size = stat_buf.st_size;
		printf(" size : %d\n", size);
		printf("\nfile opened\n");
		while ( (n = read(fd, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(sockfd,buf,n);
		}
		printf("Transfer completed \n");
		close(sockfd);
		close(fd);

	}
	else if(strcmp(cmd, "GET") == 0){
		printf("Name the file you want to receive : ");
		scanf("%s",f_name);
		send(sockfd,f_name,sizeof(f_name),0);

		fd=open(f_name,O_WRONLY|O_CREAT,S_IRWXU);
		while ( (n = read(sockfd, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(fd,buf,n);
			if( n < BUFSIZE-2)
				break;
		}

		printf("File received \n");
		close(sockfd);
		close(fd);
	}
	else if(strcmp(cmd, "LIST") == 0){
		printf("Files in current directory : \n");
		for(;;){
			memset(&f_name, '\0', 100);
			int temp = recv(sockfd, f_name, 100, 0);
			if(strcmp(f_name, "STOP") == 0){
				printf("No more files in current directory.\n");
				break;
			}
			printf("%s\n", f_name);
		}
		close(sockfd);
	}
    else{
        printf("Not a valid FTP command!\n");
        close(sockfd);
    }
	exit(0);
}
