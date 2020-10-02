// first run server with ./a.out command or ./objname command
// it can listen to multiiple clients at once



#define FNAME file1
#define PORT 8010
#define BUFSIZE 128
#define LISTENQ 5



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <linux/in.h>
#include <unistd.h>




typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

void * process(void * ptr)			//to handle individual threads
{
        int fd, pid, n, size;
	struct sockaddr_in servaddr;
	char buf[BUFSIZE],fname[50],command_1[50];
	connection_t * connection;
	pthread_t thread;
	struct stat stat_buf;

	int len;
	connection_t * conn;
	long addr = 0;

	if (!ptr) pthread_exit(0);

	conn = (connection_t *)ptr;
	printf("Handling connection request\n");
	recv(conn->sock, command_1, 50, 0);
	printf("command received: %s\n", command_1);		//print command name

	if(strcmp(command_1, "GET") == 0){			//if GET Command is receieved
		recv(conn->sock,fname,50,0);
		printf("name of the file is %s \n:  ", fname);

		fd=open(fname,O_RDONLY,S_IRUSR);
		fstat(fd, &stat_buf);
		size = stat_buf.st_size;

		printf(" size of the file is %d\n", size);

		printf("\nopened file\n");
		while ( (n = read(fd, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(conn->sock,buf,n);
		}
		printf("file transfer completed \n");
		close(conn->sock);
		close(fd);
	}
	else if(strcmp(command_1, "PUT") == 0){			//if PUT command is receieved
		recv(conn->sock,fname,50,0);
		printf("File name is %s :  ", fname);
		fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
		while ( (n = read(conn->sock, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(fd,buf,n);
			if( n < BUFSIZE-2)
				break;
		}

		printf("file has been received \n");
		close(conn->sock);
		close(fd);
	}
	
	
	else if(strcmp(command_1, "LIST") == 0){		//if LIST command is receieved
		struct dirent *de;

	    DIR *dr = opendir(".");
	    if (dr == NULL)					//to handle null directory cases
	    {
	    	char result[50] = "Could not open current directory\n";
	        printf("%s\n", result);
	        send(conn->sock, result, sizeof(result), 0);
	    }




	    while ((de = readdir(dr)) != NULL){
            char string[50];
            strcpy(string, de->d_name);
            printf("%s\n", string);
            send(conn->sock, string, sizeof(string), 0);
        }

	    char stop_signal[] = "STOP";
	    send(conn->sock, stop_signal, sizeof(stop_signal), 0);
	    closedir(dr);
	    printf("file listing completed \n");
		close(conn->sock);
    }
	free(conn);
	pthread_exit(0);		//exit or terminate thread
}

int main(int argc, char **argv)
{
	int	listen_fd, conn_fd, fd, pid, n, size;
	struct sockaddr_in servaddr;
	char buf[BUFSIZE],fname[50];
	connection_t * connection;
	pthread_t thread;
	struct stat stat_buf;


	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(PORT);

	bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listen_fd, LISTENQ);
		printf("Listening to CLient\n");		//Listen to CLient continuosly unless the program is terminated



	for ( ; ; )
	{
	
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(listen_fd, &connection->address, &connection->addr_len);
		
		if (connection->sock <= 0)
		{
			free(connection);
		}
		
		else
		{
			//create a new thread too handle the current re
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}

    }
    return 0;
}

