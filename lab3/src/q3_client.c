//////******PLEASE CONNECT THREE CLIENTS TO GET THE RESULT********////////

#include<sys/ioctl.h>
#include <stdint.h>
#include <inttypes.h>
#include<net/if_arp.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include <stdio.h>
#include <time.h>
#define F_NAME file1
#define PORT 8010
#define BUF_SIZE 256

int64_t get_time()
{
    struct timespec t_m_s;

    if(clock_gettime(CLOCK_REALTIME,&t_m_s))
    return -1;
    
    int64_t m=t_m_s.tv_nsec*1000000;

    m+=t_m_s.tv_nsec/1000;

    if(t_m_s.tv_nsec%1000>=500)
    ++m;
    
    return m;
}

int main(int argc,char **argv)
{
    int sockfd,fd,n,size,count=0;

    int64_t T_m_r;

    char T_m_s[50]; char *end;

    long int size_1,size_2;

    char buf[BUF_SIZE],f_name[50],command[50],s_RTT[50];

    struct sockaddr_in serv_addr;

    struct stat stat_buf;

    if(argc!=2) 
    exit(0);
    	
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) 
    exit(0);	//If socket establishment not successfull,then exit;
	
    bzero(&serv_addr,sizeof(serv_addr)); // set the structure serv_addr to zero

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);

    if(inet_pton(AF_INET,argv[1],&serv_addr.sin_addr)<=0)
    exit(0);

    if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    exit(0);

    printf("Hello\n");

    recv(sockfd,command,50,0);
    send(sockfd,command,50,0);

    printf("Enter the name of the file which you want to receive: ");
    scanf("%s",f_name);

    send(sockfd,f_name,sizeof(f_name),0);

    recv(sockfd,T_m_s,sizeof(T_m_s),0);

    recv(sockfd,s_RTT,sizeof(s_RTT),0);

    long long l_Tms=strtoll(T_m_s,&end,10);

    long long RTT=strtoll(s_RTT,&end,10);

    fd=open(f_name,O_WRONLY|O_CREAT,S_IRWXU);

    while((n=read(sockfd,buf,BUF_SIZE-1))>0)
    {
        buf[n]='\0';
        write(fd,buf,n);

        if(n<BUF_SIZE-2)
	{
            T_m_r = get_time();
            break;
        }
    }

    printf("RTT is %lld microseconds.\n",RTT); // Printing the RTT

    printf("T_m_s is %lld microseconds.\n",l_Tms); 

    printf("T_m_r is %"PRId64" microseconds\n",T_m_r);

    printf("File receiving has been completed in %lld microseconds.\n",(long long)T_m_r-l_Tms);

    close(sockfd);
    close(fd);

    exit(0);
}
