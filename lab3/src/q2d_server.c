#include<sys/types.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<dirent.h>
#include<fcntl.h>
#include<stdio.h>
#include<pthread.h>
#include<sys/socket.h>
#include<string.h>
#include<linux/in.h>

#define FNAME file01
#define BUFSIZE 128
#define LISTENQ 5
#define PORT 8080

typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

pthread_mutex_t mutex_fd;

void * handle(void * ptr)
{
	int fd, pid, n, size, len;
	char buf[BUFSIZE],fname[100],command[100];
	struct stat stat_buf;
	struct sockaddr_in servaddr;
	connection_t * connect;
	pthread_t thread;
	connection_t * conn;
	long addr = 0;

	if (!ptr) pthread_exit(0);
	conn = (connection_t *)ptr;

	printf("connection request recieved...\n");
	recv(conn->sock, command, 100, 0);
	printf("command received : %s\n", command);

	if(strcmp(command, "PUT") == 0){
		recv(conn->sock,fname,100,0);
		printf("File name : %s :  ", fname);
        pthread_mutex_lock(&mutex_fd);
		fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
		while ( (n = read(conn->sock, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(fd,buf,n);
			if( n < BUFSIZE-2)
				break;
		}

		printf("file receiving completed. \n");
        pthread_mutex_unlock(&mutex_fd);
		close(conn->sock);
		close(fd);
	}
	else if(strcmp(command, "GET") == 0){
		recv(conn->sock,fname,100,0);
		printf("File name : %s \n:  ", fname);

		pthread_mutex_lock(&mutex_fd);
		fd=open(fname,O_RDONLY,S_IRUSR);
		fstat(fd, &stat_buf);
		size = stat_buf.st_size;

		printf(" size : %d\n", size);

		printf("\nopened file...\n");
		while ( (n = read(fd, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(conn->sock,buf,n);
		}
		printf("file transfer completed \n");
		pthread_mutex_unlock(&mutex_fd);
		close(conn->sock);
		close(fd);
	}
	else if(strcmp(command, "LIST") == 0){
	struct dirent *de;
	    DIR *dr = opendir(".");
	    if (dr == NULL)
	    {
	        char result[100] = "Couldn't open current directory!\n";
	        printf("%s\n", result);
	        send(conn->sock, result, sizeof(result), 0);
	    }


	    while ((de = readdir(dr)) != NULL){
            char str[100];
            strcpy(str, de->d_name);
            printf("%s\n", str);
            send(conn->sock, str, sizeof(str), 0);
        }

		char stop[] = "STOP";
		send(conn->sock, stop, sizeof(stop), 0);
		closedir(dr);
		printf("The listing is completed. \n");
		close(conn->sock);
    }
	free(conn);
	pthread_mutex_destroy(&mutex_fd);
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	int listen_fd, pid, size, n, confd, fd;

	struct sockaddr_in servaddr;
	char buf[BUFSIZE],fname[100],command[100];
	connection_t * connect;
	pthread_t thread;
	struct stat stat_buf;

	pthread_mutex_init(&mutex_fd, NULL);


	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(PORT);

	bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listen_fd, LISTENQ);
		printf("listening...\n");

	while(1)
	{
        connect = (connection_t *)malloc(sizeof(connection_t));
        connect->sock = accept(listen_fd, &connect->address, &connect->addr_len);
		if (connect->sock <= 0)
		{
			free(connect);
		}
		else
		{
			pthread_create(&thread, 0, handle, (void *)connect);
			pthread_detach(thread);
		}

    }
    return 0;
}

