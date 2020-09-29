#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
//#include<ifdef.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <linux/in.h>
#include <unistd.h>

#define FNAME file1
#define PORT 8010
#define BUF_SIZE 256
#define LINQ 10
int i=0;
typedef struct connection_type
{
    int sock,addr_len;
    struct sockaddr address;
    
}connection_type;

void* process(void* ptr)
{
    int fd,n,size,pid;
    struct sockaddr_in servaddr;
    char buf[BUF_SIZE],f_name[50];
	char command[50];

    struct connection_type* connection;
    struct connection_type* conn;
    pthread_t thread;

    struct stat stat_buf;

    int len; long addr=0;

    if(!ptr)pthread_exit(0);

    conn=(struct connection_type*)ptr;

    printf("Connection request\n");

    recv(conn->sock,f_name,50,0);


    printf("File name is %s\n",f_name);

    fd=open(f_name,O_RDONLY,S_IRUSR);

    fstat(fd,&stat_buf);

    size = stat_buf.st_size;

    printf("Size:%d\n",size);

    printf("\nOpened the file\n");


    while((n=read(fd,buf,BUF_SIZE-1))>0)
    {
        buf[n]='\0';

        write(conn->sock,buf,n);
    }

    printf("File transfer completed\n");

    close(conn->sock);
    close(fd);

    free(conn);
    pthread_exit(0);
}

int main(int argc,char **argv)
{
    int listenfd,connfd,fd,size,pid,n;

    struct sockaddr_in servaddr;
    char buf[BUF_SIZE],f_name[50],command[50];
    struct connection_type* connection;
    pthread_t thread;

    struct stat stat_buf;


    listenfd=socket(AF_INET,SOCK_STREAM,0);

    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);

    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

    listen(listenfd,LINQ);

    printf("Listening\n");

    while(1)
    {
        connection=(struct connection_type*)malloc(sizeof(connection_type));
        connection->sock=accept(listenfd,&connection->address,&connection->addr_len);

        if(connection->sock<=0)
        free(connection);
        
        else
        {
            pthread_create(&thread,0,process,(void*)connection);
            pthread_detach(thread);
        }

    }
    return 0;
}

