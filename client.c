#include <sys/types.h> //mkfifo
#include <sys/stat.h>  //mkfifo
#include <stdio.h>     // printf
#include <fcntl.h>     // open
#include <unistd.h>    // unlink
#include <stdlib.h>    // exit
#include <stdbool.h>   // bool
#include <string.h>    // strlen
#include <pthread.h>
#include <pthread.h>
#include <signal.h>

pthread_t threads;
pthread_mutex_t signalLock ;
pthread_cond_t signalCond ;

const char ready[] = "Ready\n";
const char fire[] = "Fire\n";
const char done[] = "done\n";
const char SERVER_FIFO[] = "server_pipe";
bool wait = false;


void handler(int args){
    char CLIENT_FIFO [] = "client_pipe";
    char pid[7];
    memset(pid, '\0', 7);
    sprintf(pid, "%d", getpid());
    strcat(CLIENT_FIFO, pid);
	unlink(CLIENT_FIFO);
	exit(0);
}

void* threading_task(void* ignored){
    pthread_mutex_lock(&signalLock);
     while(!wait){
		pthread_cond_wait(&signalCond,&signalLock);
	 } 
     pthread_mutex_unlock(&signalLock);
	 int fd = * ((int *) ignored); // server FIFO
     char pid[7];
     memset(pid, '\0', 7);
     sprintf(pid, "%d", getpid());
     pid[strlen(pid)] = '\n';
     char input[10];
     printf("press enter\n");
     fgets(input, 10, stdin);
	 int written = write(fd, &pid, strlen(pid));
     if (written < 0){
         perror("ERROR: Error writing to pipe");
     }  

    pthread_mutex_lock(&signalLock);
    wait = false;
    pthread_mutex_unlock(&signalLock);

   	pthread_exit(0);
}


int main(){
    pthread_cond_init(&signalCond, NULL);
  	pthread_mutex_init(&signalLock,NULL);
    
    signal(SIGINT,handler);
    signal(SIGTERM,handler);
    char CLIENT_FIFO [] = "client_pipe";
    char pid[7];
    memset(pid, '\0', 7);
    sprintf(pid, "%d", getpid());
    strcat(CLIENT_FIFO, pid);
    printf("%s\n", CLIENT_FIFO);
    pid[strlen(pid)] = '\n';
    printf("PID of player %s",pid);
    int fifo = open(SERVER_FIFO, O_WRONLY);
    int written = write(fifo, &pid, strlen(pid));
    if (written < 0){
         perror("ERROR: Error writing to pipe");
    }  

    pid[strlen(pid)-1] = '\0';
    int result = mkfifo(CLIENT_FIFO, 0600);
	if (result) {
		perror("Unable to create named pipe");
		exit(EXIT_FAILURE);
	} 
    FILE * fd = fopen(CLIENT_FIFO, "r+");
	if (fd == NULL) {
		perror("Unable to open named pipe");
		exit(EXIT_FAILURE);
	}
    
    char resp [50];
    memset(resp, '\0', 50); // ready
    fgets(resp, 50, fd);
    printf("%s",resp);
    
    while(strcmp(resp,ready) == 0){
       
        int * val =&fifo;
        
        memset(resp, '\0', 50);
        fgets(resp, 50, fd); // fire
        
        printf("%s",resp);
        if(strcmp(resp, fire) != 0){
            break;
        }
        pthread_create( &threads, NULL, threading_task,  val);
        
        pthread_mutex_lock(&signalLock);
	    wait = true;
        pthread_cond_signal(&signalCond);
	    pthread_mutex_unlock(&signalLock);
        
        // pthread_join(threads, NULL);

        memset(resp, '\0', 50);
        fgets(resp, 50, fd); // round score
        printf("%s",resp);

        memset(resp, '\0', 50);
        fgets(resp, 50, fd);// next round
        printf("%s",resp);
    
    }
    memset(resp, '\0', 50);
    fgets(resp, 50, fd);// Score board
    printf("%s",resp);

    memset(resp, '\0', 50);
    fgets(resp, 50, fd);// congratulatory message
    printf("%s",resp);

    unlink(CLIENT_FIFO);
    return 0;
}
