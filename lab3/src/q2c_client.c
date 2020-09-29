#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <linux/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define FNAME file1
#define PORT 8010
#define BUF_SIZE 256
#define LINQ 10

typedef struct connection_type
{
    int sock,addr_len;
    struct sockaddr address;
    
}connection_type;

pthread_t threads[LINQ];
pthread_attr_t attr;


char *my_itoa(int num, char *str)
{
    if(!str)
    return NULL;
    
    sprintf(str,"%d",num);
    return str;
}

void* process(void* ptr)
{
    int sockfd,fd,n,size,count=0;
    long int size1,size2;

    char buf[BUF_SIZE],f_name[50];
	char command[50],st[3];

    struct sockaddr_in servaddr;
    struct stat stat_buf;

    int t=(int)ptr;

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
	printf("Error\n");
	exit(1);
    }

    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);

    if (inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr)<=0)
    exit(0);

    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        printf("Error in connecting\n");
        exit(0);
    }

    printf("Connection has been established\n");

    strcpy(f_name,"50MB");
    my_itoa(t,st);
    strcat(f_name,st);

    strcat(f_name,".zip");

    send(sockfd,"50MB.zip",50,0);

    fd=open(f_name,O_WRONLY|O_CREAT,S_IRWXU);


    while((n=read(sockfd,buf,BUF_SIZE-1))>0)
    {
        buf[n]='\0';

        write(fd,buf,n);

        if(n<BUF_SIZE-2)
        break;
    }

    printf("File received completely\n");

    close(sockfd);
    close(fd);

    pthread_exit(0);
}

int main(int argc,char **argv)
{
    void* status;


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);


    for(int j=0;j<LINQ;j++)
    pthread_create(&threads[j],0,process,(void*)j);

    
    pthread_attr_destroy(&attr);


    for(int p=0;p<LINQ;p++) 
    {
        pthread_join(threads[p],&status);

        printf("Main: completed join with thread %d having a status of %ld 		\n",p,(long)status);

    }
    return 0;
}

