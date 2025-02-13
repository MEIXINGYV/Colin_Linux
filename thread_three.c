#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<regex.h>

char buffA[4096];
char buffB[4096];
int  cont = -1;
pthread_mutex_t lock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void * thread_T1(void*arg)
{   
	pthread_mutex_lock(&lock);
	if(cont != -1){
		pthread_cond_wait(&cond,&lock);
	}

	int fd = open("./Server_info.log",O_RDONLY);
	if(fd != -1){
		printf("open file successs\n");
	}


	int read_get = read(fd,buffA,sizeof(buffA));
	printf("read:%d\n",read_get);
    
	cont = 0;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
	usleep(30000);

	//int write_put = write(fd,buffA,read_get);
	//printf("write: %d\n",write_put);
	//printf("getAAA %s",buffA);

}
void * thread_T2(void*arg)
{ 
   pthread_mutex_lock(&lock);
   if(cont != 0){
	   pthread_cond_wait(&cond,&lock);
	}
   printf("T2 working..\n");
   //printf("%s\n",buffA);

   char *pattern = "^XTARGET\\b.*";
   char line[1024]; 
   char buf[1024];

   regex_t preg;
   int ret = regcomp(&preg,pattern,REG_EXTENDED | REG_NEWLINE);

   if(ret != 0){

	   char errormsg[1024];
	   regerror(ret,&preg,errormsg,sizeof(errormsg));
	   fprintf(stderr,"Regex failed:%s\n",errormsg);
	   return 1;
   }
  
  char *token = strtok(buffA,"\n");

  while(token != NULL){
 
      strcpy(line,token);

	  ret = regexec(&preg,line,0,NULL,0);
		  if(ret == 0){
			  if(strlen(buf) + strlen(line) + 1 <1024){
				  if(strlen(buf) > 0){
					  strcat(buf,"\n");
				  }
				  strcat(buf,line);
			  }else{
				  printf("Buffer is full\n");
				  break;
			  }
		  }
		  token = strtok(NULL,"\n");
	  }
  

   regfree(&preg);
   printf("T2 BUF:\n%s\n",buf);

   strncpy(buffB,buf,sizeof(buffB));

   //printf("T2 buffB :  %s\n",buffB);

   cont = 1;
   pthread_cond_signal(&cond);
   pthread_mutex_unlock(&lock);
   usleep(30000);
   
}
void * thread_T3(void*arg)
{ 
  sleep(1);
  pthread_mutex_lock(&lock);
  if(cont != 1){
	  pthread_cond_wait(&cond,&lock);
  }
  printf("T3 working..\n");

  printf("T3 buffB:\n%s\n",buffB);
  
  int fd = open("Result",O_RDWR);
  int write_count = write(fd,buffB,sizeof(buffB));

  if(write_count > 0)
  {
	  printf("write success..\n");
  }
  cont = 3;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
  usleep(30000);
 
}


 

int main(){
	pthread_t tid;
	pthread_create(&tid,NULL,thread_T1,NULL);
	pthread_create(&tid,NULL,thread_T2,NULL);
    pthread_create(&tid,NULL,thread_T3,NULL);
	while(1)sleep(1);

}
