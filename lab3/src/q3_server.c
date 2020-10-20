
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <linux/in.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#define FILE_NAME file1
#define PORT_NUMBER 8010
#define BUFF_SIZE 128
#define LISTEN_QUERY 3

int64_t RTT[LISTEN_QUERY];
int64_t MaxRTT;



typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;
    int t;
} connection_t;



pthread_mutex_t mutex_fd;
pthread_t jthreads[LISTEN_QUERY];
pthread_t threads[LISTEN_QUERY];
pthread_attr_t attr;




int64_t max(int64_t RTT[])
{
    int64_t maximum = RTT[0];
    for(int i=1; i<LISTEN_QUERY; i++)
    {
        if(maximum < RTT[i])
        {
            maximum = RTT[i];
        }
    }
    return maximum;
}




int64_t gettime()
{
    struct timespec tms;
    if (clock_gettime(CLOCK_REALTIME,&tms)) 
    {
        return -1;
    }
    int64_t micro_out = tms.tv_sec * 1000000;
    micro_out += tms.tv_nsec/1000;
    if (tms.tv_nsec % 1000 >= 500)
    {
        ++micro_out;
    }
    return micro_out;
}





void * j_process(void * ptr)
{
    int file_id, pid, n, size;
    struct timespec tms;
    struct sockaddr_in servaddr;
    char buf[BUFF_SIZE],file_name[50],command[50];
    pthread_t thread;
    struct stat stat_buf;
    int len;
    connection_t * conn;
    long addr ;
    addr=0;
    if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;
    printf("Handling the request\n");
    int64_t t1 = gettime();
    send(conn->sock, command, 50, 0);
    recv(conn->sock, command, 50, 0);
    int64_t t2 = gettime();
    printf("RTT of this client is %"PRId64" microseconds\n", t2-t1);
    RTT[conn->t] = t2 - t1;
    pthread_exit((void*) conn->t);
}





void* process(void* ptr)
{
    int file_id, pid, n, size;
    struct timespec tms;
    struct sockaddr_in servaddr;
    char buf[BUFF_SIZE],file_name[50],command[50], strt[50], string_Round_trip_time[50];
    pthread_t thread;
    struct stat stat_buf;
    int len;
    connection_t * conn;
    long addr = 0;
    if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;
    printf("Handling the requests\n");
    //Tring to get the file.
    recv(conn->sock,file_name,50,0);
    printf("Name of the file is %s \n:  ", file_name);
    pthread_mutex_lock(&mutex_fd);
    file_id=open(file_name,O_RDONLY,S_IRUSR);
    fstat(file_id, &stat_buf);
    size = stat_buf.st_size;
    printf(" size is %d\n", size);
    printf("\nopened file\n");
    int64_t Time_in_ms = clock();
    sprintf(strt, "%"PRId64"", Time_in_ms);
    send(conn->sock, strt, sizeof(strt), 0);
    sprintf(string_Round_trip_time, "%"PRId64"", MaxRTT);
    send(conn->sock, string_Round_trip_time, sizeof(string_Round_trip_time), 0);
    while ( (n = read(file_id, buf, BUFF_SIZE-1)) > 0)
    {
        buf[n] = '\0';
        write(conn->sock,buf,n);
    }
    printf("file transfer completed \n");
    pthread_mutex_unlock(&mutex_fd);
    close(conn->sock);
    close(file_id);
    pthread_mutex_destroy(&mutex_fd);
    pthread_exit((void*) conn->t);
}




int main(int argc, char **argv)
{
    int listen_fd, connfd, file_id, pid, n, size, t;
    struct sockaddr_in servaddr;
    char buf[BUFF_SIZE],file_name[50],command[50];
    connection_t **connection = (connection_t **)malloc(LISTEN_QUERY*sizeof(connection_t*));
    for(int i = 0; i < LISTEN_QUERY; i++) connection[i] = (connection_t *)malloc(sizeof(connection_t));
    pthread_t thread;
    struct stat stat_buf;
    pthread_mutex_init(&mutex_fd, NULL);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PORT_NUMBER);
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listen_fd, LISTEN_QUERY);
    printf("listening for requests\n");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc;
    void *status;
    for ( ; ; )
    {
        for(t=0; t<LISTEN_QUERY;) 
        {
            connection[t]->sock = accept(listen_fd, &connection[t]->address, &connection[t]->addr_len);
            printf("Main: creating thread %d\n", t);
            connection[t]->t = t;
            if (connection[t]->sock <= 0)
            {
                free(connection[t]);
            }
            else
            {
                pthread_create(&jthreads[t], &attr, j_process, (void *)connection[t]);
                t++;
            }
        }
        pthread_attr_destroy(&attr);
        for(t=0; t<LISTEN_QUERY; t++) 
        {
            pthread_join(jthreads[t], &status);
            printf("Main: completed join with thread %d having a status of %ld \n",t,(long)status);
        }
        MaxRTT = max(RTT);
        printf("Maximum RTT is: %"PRId64" microseconds\n", MaxRTT);
        for(t=0; t<LISTEN_QUERY;t++)
        {
            pthread_create(&threads[t], 0, process, (void *)connection[t]);
            pthread_detach(threads[t]);
        }

    }
    return 0;
}

