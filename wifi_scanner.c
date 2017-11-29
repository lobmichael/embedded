/*********************************
 *	Embedded Real Times Systems	
 *	Project 3
 *	Author: Michael Lobjoit
 *	AEM: 8184
 *	
 ***********************************/

#define _BSD_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

#define QUEUESIZE 5000
#define LOOP 500
#define MAX_WIFI 100

int sampling_time;
static sigset_t   signal_mask; 

struct timeval tmp,prog_start,prog_end;

void sampler_1hz(){
	
	gettimeofday(&tmp,NULL);
  	

}

static void alarm_handler()
{
    return;///does nothing really just breaks to code underneath gettimeofday(&tmp,NULL);
  
}

void *producer (void *args);
void *consumer (void *args);

typedef struct {
  int buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
	char data[QUEUESIZE][MAX_WIFI][32];
	int last_batch[QUEUESIZE];
	long long timestamps[QUEUESIZE][2];
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

clock_t start_t, end_t, total_t;



int main (int argc, char** argv)
{


start_t = clock();
gettimeofday(&prog_start,NULL);

if (argc != 2) {

printf("Incorrect input arguments,1 Sampling Period");

}


  queue *fifo;
  pthread_t pro, con;

  fifo = queueInit ();
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }
	


  sampling_time = atoi(argv[1]);
	
struct itimerval mark;
//char timestamps[n_samples][26];

if(signal(SIGALRM, sampler_1hz)==SIG_ERR){
	printf("Error: Couldn't catch alarm signal\n");
		exit(1);
}

mark.it_value.tv_sec = sampling_time;
mark.it_value.tv_usec = 0;
mark.it_interval=mark.it_value;




	if(setitimer(ITIMER_REAL, &mark, NULL)==-1){
		printf("Error: Couldn't set timer\n");
		exit(1);
	}
    

  pthread_create (&pro, NULL, producer, fifo);

int rc; //return code;

	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGALRM);
	    rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	    if (rc != 0) {
		printf("error blocking signal");
	    }



  pthread_create (&con, NULL, consumer, fifo);
  pthread_join (pro, NULL);
  pthread_join (con, NULL);
  queueDelete (fifo);
	
	end_t = clock();
	printf("start_t = %ld\n", start_t);
	printf("end_t = %ld\n", end_t);
	total_t = (end_t - start_t); /// CLOCKS_PER_SEC;
	//printf("CLOCKS PER SEC: %d\n", CLOCKS_PER_SEC);
   	printf("TICKS: %ld\n", total_t  );
	
	gettimeofday(&prog_end,NULL);
	timersub(&prog_end,&prog_start,&prog_start);
	
	double t = total_t;
	printf("(DOUBLE)TICKS: %f\n", t  );
	long double t1 = (long double)(prog_start.tv_sec+prog_start.tv_usec*0.000001);
	t = t/CLOCKS_PER_SEC;
	printf("CPU TIME IS: %f\n",t);
	printf("TIME ELAPSED IS: %Lf\n",t1);
	
	printf("CPU TIME IS: %f\n",t);
	t = t/t1;
	t = t*100;
	printf("CPU PERCENTAGE IS: %f\n",t);

  return 0;
}

void *producer (void *q)
{
  queue *fifo;
  int i;

  fifo = (queue *)q;


  for (i = 0; i < LOOP; i++) {  ///inf loop here

sleep(10000000);

    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
      printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
    queueAdd (fifo, i);

    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
    
  }
  
  return (NULL);
}

void *consumer (void *q)
{
	


  queue *fifo;
  int i, d;

  fifo = (queue *)q;



 

  for (i = 0; i < LOOP; i++) {
	
    pthread_mutex_lock (fifo->mut);
    while (fifo->empty) {
      printf ("consumer: queue EMPTY.\n");	
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }
    queueDel (fifo, &d);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notFull);
    printf ("consumer: recieved %d.\n", d);
  }
  
  return (NULL);
}

queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

//test



//test



  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
	
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);	
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q, int in)
{
  q->buf[q->tail] = in;
  q->tail++;


//Search wifi segment
struct timeval tmp;
gettimeofday(&tmp,NULL);
printf("Adding %d to queue\n",in);

FILE *fp;
  char path[1035];

char ssids[MAX_WIFI][32];

  /* Open the command for reading. */
  fp = popen("/bin/sh wifiscan.sh", "r");

  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }
 	int i=0;
  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
   
	strtok(path,"\n");
	strcpy(ssids[i],path);
	strcpy(q->data[in][i],ssids[i]);
	i++;
	

  }

q->timestamps[in][0] = (long long)tmp.tv_sec;
q->timestamps[in][1] = (long long)tmp.tv_usec;
q->last_batch[in] = i;

pclose(fp);

//End of search for wifi segment	


  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;


  return;
}

void queueDel (queue *q, int *out)
{
  *out = q->buf[q->head];


if(q->buf[q->head]==0){
	FILE *mylogf;
	mylogf = fopen("log.txt","w+");
	for(int k=0;k<q->last_batch[q->head];k++){
			fprintf(mylogf,"%s %lld %lld\n",q->data[q->head][k],q->timestamps[q->head][0],q->timestamps[q->head][1]);
		}
	fclose(mylogf);
}
else {
char line[1000];
char ssids[MAX_WIFI][100];
int count = 0;

FILE *mylogf;
mylogf = fopen("log.txt","r+");

while (fgets(line, sizeof(line)-1, mylogf) != NULL) {
	
		
		strtok(line,"\n");
		strtok(line,"\":");
		strcat(line,"\"");
		strcpy(ssids[count],line);
		count++;
}




fclose(mylogf);	
int flag=1;		
for(int k=0;k<q->last_batch[q->head];k++){
	
		for(int j=0;j<count;j++){		
			if(strcmp(q->data[q->head][k],ssids[j])==0){
				flag=0;}}
	if(flag==0){char cmd[200];
						strcpy(cmd,"sed -i '/");
						strcat(cmd,strtok(q->data[q->head][k]," "));
						strcat(cmd,"/ s/$/ ");
	//newtimestamps		

						
						char output[50];

						snprintf(output, 50, "%lld %lld",q->timestamps[q->head][0],q->timestamps[q->head][1]);


						strcat(cmd,output);
						strcat(cmd,"/' log.txt");
						system(cmd);
						flag=1;
					}
		else{
	
		
			
	flag=1;
	
			char cmd[200];
			char output[50];
			strcpy(cmd,"echo '");
			snprintf(output, 50, "%s %lld %lld",q->data[q->head][k],q->timestamps[q->head][0],q->timestamps[q->head][1]);
			strcat(cmd,output);
			strcat(cmd,"' >> log.txt");
			
			system(cmd);//echo 'text to append' >> file2
			}

//fclose(mylogf);
	
	}	
}
struct timeval tmp1;
gettimeofday(&tmp1,NULL);

timersub(&tmp1,&tmp,&tmp1);
FILE *times;
	times = fopen("times.txt","a");
fprintf(times,"%f\n",(double)tmp1.tv_sec+0.000001*(double)tmp1.tv_usec);
fclose(times);
  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;
  return;
}
