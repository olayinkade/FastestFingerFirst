#include <sys/types.h> //mkfifo
#include <sys/stat.h>  //mkfifo
#include <stdio.h>     // printf
#include <fcntl.h>     // open
#include <unistd.h>    // unlink
#include <stdlib.h>    // exit
#include <stdbool.h>   // bool
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define ROUNDS 3
#define NOOFOPPS 2
#define LARGESTPID 5
#define NOOFTHREAD 1

pthread_t threads;
pthread_mutex_t signalLock ;
pthread_cond_t signalCond ;

bool wait = false;
const char SERVER_FIFO[] = "server_pipe";

struct Opps{
	char pid [5];
	int client;
	int totalW;
};

struct Opps opp [NOOFOPPS];

void handler(int args){
	const char error [] = "Server closed press ctrl + c\n";
	for(int i =0; i < NOOFOPPS; i++){
		write(opp[i].client, error , strlen(error));
	}
	unlink(SERVER_FIFO);
	exit(0);
	
}

FILE * initServerFifo(){
	int result = mkfifo(SERVER_FIFO, 0600);
	if (result) {
		perror("Unable to create named pipe");
		exit(EXIT_FAILURE);
	} 
	printf("opening FILE\n");

	FILE * fd = (FILE * ) malloc(sizeof(FILE *));
	fd = fopen(SERVER_FIFO, "r+");
	if (fd == NULL) {
		perror("Unable to create named pipe");
		exit(EXIT_FAILURE);
	}
	printf("done opening FILE\n");
	return fd;
}

int openClient(char * pid, FILE * fd ){
	char CLIENT_FIFO [] = "client_pipe";
	char * player = (char * ) malloc(sizeof(char) * 6);
	memset(player, '\0', 6);
	
	fgets(player, LARGESTPID + 2, fd);
	player[strlen(player)-1] = '\0';
	strcat(CLIENT_FIFO, player);
	int client = open(CLIENT_FIFO, O_RDWR);
	if (client <  0) {
		perror("Unable to open named pipe ");
		exit(EXIT_FAILURE);
	}
	strcpy(pid,player); 

	return client;
}

void initClientFifo(FILE * fd){

	for( int i = 0; i < NOOFOPPS; i++){
		char player1[6];
		int client = openClient( player1, fd );
		struct Opps * a = (struct Opps*) malloc(sizeof(struct Opps));
		strcpy(a->pid,player1);
		a->client =  client;
		opp[i] = *a;
		printf("Player %d with id %s has connected\n",i+1,a->pid);

	}
}


void* threading_task(void* ignored){
	
  	const char ready [] ="Ready\n";
	const char fire [] = "Fire\n";
	const char w [] = "you won the round\n";
	const char l [] = "you lose, faster fingers\n";
	
	FILE* fd = (FILE*) ignored;
	for(int i =0; i < NOOFOPPS; i++){
		write(opp[i].client, ready, strlen(ready));
	}
	pthread_cond_signal(&signalCond);
	while(!wait){
		pthread_cond_wait(&signalCond,&signalLock);
	}
	pthread_mutex_unlock(&signalLock);
	for(int i =0; i < NOOFOPPS; i++){
		write(opp[i].client, fire, strlen(fire));
	}

	char winner [7];
	char loser [7];
	memset(winner, '\0', 7);
	memset(loser, '\0', 7);

	fgets(winner, LARGESTPID + 2, fd);
	winner[strlen(winner)-1] = '\0';
	printf("First place goes to %s\n", winner);
	fgets(loser, LARGESTPID + 2, fd);
	
	printf("Second place goes to %s\n", loser);
	if(strcmp(opp[0].pid, winner) == 0){
		opp[0].totalW = opp[0].totalW + 1;
		write(opp[0].client, w, strlen(w));
		write(opp[1].client, l, strlen(l));
	} 
	else if(strcmp(opp[1].pid, winner) == 0){
		opp[1].totalW = opp[1].totalW + 1;
		write(opp[1].client, w, strlen(w));
		write(opp[0].client, l, strlen(l));
	}
	pthread_mutex_lock(&signalLock);
	wait = false;
	pthread_mutex_unlock(&signalLock);

   	pthread_exit(0);
}

void fire(){
	srand(time(0));
	int lower = 2000;
	int upper = 5000;
	int ran = (rand() % (upper - lower + 1)) + lower;
	printf("sleeping for %d milliseconds \n", ran );
	usleep(ran * 1000);
}

void scoreboard(){
	const char done[] ="GAME OVER !!!\n";
	const char champ[] = "CHAMPION YOU WON IN THIS AND IN LIFE\n";
	const char LOSER[] = "FAILURE, YOU are a Loser\n";
	for(int i =0; i < NOOFOPPS; i++){
	    write(opp[i].client, done, strlen(done));
	 	char score [50];
		sprintf(score, "Your score is %d your oppenent score is %d \n", opp[i].totalW, 3-opp[i].totalW);
		write(opp[i].client, score, strlen(score));
	}
	if(opp[0].totalW > 1){
		write(opp[0].client, champ, strlen(champ));
		write(opp[1].client, LOSER, strlen(LOSER));
	}
	else if(opp[1].totalW > 1){
		write(opp[1].client, champ, strlen(champ));
		write(opp[0].client, LOSER, strlen(LOSER));
	}
}

int main(){

	pthread_cond_init(&signalCond, NULL);
  	pthread_mutex_init(&signalLock,NULL);

	signal(SIGINT,handler);
    	signal(SIGTERM,handler);

	FILE * fd = initServerFifo();
	initClientFifo(fd);

	for(int i = 0; i < ROUNDS; i++){
		pthread_create( &threads, NULL, threading_task, fd);
		fire();
		pthread_mutex_lock(&signalLock);
		wait = true;
		pthread_cond_signal(&signalCond);
		pthread_mutex_unlock(&signalLock);
		
		pthread_join(threads, NULL);

	}
	scoreboard();	
	unlink(SERVER_FIFO);

    return 0;
}
	
