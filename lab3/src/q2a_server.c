
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include<stdlib.h>
#include <dirent.h>

#define FNAME file1
#define PORT 8010
#define buff_size 128
#define LISTENQ 5

int main(int argc, char **argv)
{
	int	bind_socket, connection_value, fd, pid, n, size;
	struct sockaddr_in server_addr;
	char buf[buff_size],file_name[50],ans[50];

	struct stat stat_buf;


	bind_socket = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port        = htons(PORT);
	
	bind(bind_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

	listen(bind_socket, LISTENQ);
		
		
	

	for ( ; ; )
	{
		
		printf("Listening :  \n");
		connection_value = accept(bind_socket, (struct sockaddr *) NULL, NULL);
		
		printf("Handling connection request\n");
		recv(connection_value, ans, 50, 0);
		printf("Received command is: %s\n", ans);
		if(strcmp(ans, "PUT") == 0){
			recv(connection_value,file_name,50,0);
			printf("File name is %s :  ", file_name);
			fd=open("Newfile_Made_by_put",O_WRONLY|O_CREAT,S_IRWXU);
			while ( (n = read(connection_value, buf, buff_size-1)) > 0)
			{
				buf[n] = '\0';
				write(fd,buf,n);
				if( n < buff_size-2)
					break;
			}

			printf("file receiving completed \n");
			
			
			close(connection_value);
			close(fd);
		}

		
		else if(strcmp(ans, "GET") == 0){
			recv(connection_value,file_name,50,0);
			printf("File sent is %s  \n ", file_name);  

			fd=open(file_name,O_RDONLY,S_IRUSR);
			fstat(fd, &stat_buf);
			size = stat_buf.st_size;
				
			printf(" size is of the received file is : %d\n", size); 

			printf("\nopened file\n");
			while ( (n = read(fd, buf, buff_size-1)) > 0) 
			{
				buf[n] = '\0';
				printf("%s\n",buf);
				write(connection_value,buf,n);
			}
			printf("file transfer completed \n");
			
		
			close(connection_value);
			close(fd);
			
		}
		else if(strcmp(ans, "LIST") == 0){
			struct dirent *de;

		    DIR *dr = opendir(".");
		    if (dr == NULL)
		    {
		    	char result[50] = "Could not open current directory\n";
		        printf("%s\n", result);
		        send(connection_value, result, sizeof(result), 0);
		    }

			while ((de = readdir(dr)) != NULL){
                char string[50];
                strcpy(string, de->d_name);
                printf("%s\n", string);
			    send(connection_value, string, sizeof(string), 0);
            }

		    char stop[] = "STOP";
		    send(connection_value, stop, sizeof(stop), 0);
		    closedir(dr);
		    printf("The listing of the file is completed. \n");
		    
		    
		    close(connection_value);
		}
	
	}
}
