//Krzysztof Borowicz, Hyunsoo Kim, Jimmy Tran
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>

#define BUFFER_LEN 128
#define MAX_ARG 32

long findTimeDif(struct timeval timeBefore, struct timeval timeAfter);
void stats(struct rusage udata, struct rusage oldudata, int hasrun, struct timeval timeBefore, struct timeval timeAfter);

int main(){
  struct rusage oldudata;
  int hasrun = 0;//flag vatriable 0 means has not run 1 means has run
  int toomanyargflag = 0; //0 if good amount, too many flags = 1
  char cwd[1024];//used to print the current working dir
  getcwd(cwd, BUFFER_LEN);
	while(1){
		int argc;
		char usrinput[BUFFER_LEN];
		char* argv[MAX_ARG];
		char *token;

		printf("%s>ourshell>",cwd);

	//Checks to make sure that you aren't sending a NULL command
    if (fgets(usrinput, BUFFER_LEN, stdin) == NULL) { 
    	printf("\n");
    	exit(0);
    }
	//checks to make sure a command is there
    if (strlen(usrinput) == 1 && usrinput[strlen(usrinput) - 1] == '\n') {
    	printf("You must insert a command!\n");
    	continue;
    }
    
    //Checks to make sure it isn't longer than our BUFFER_LEN which is 128
    if (usrinput[strlen(usrinput)-1] != '\n') {
    	int islong = 0;
    	char nextchar;

      // Loop until we reach the end of line or end of file
    	while (((nextchar = getchar()) != '\n') && (nextchar != EOF)) {
      	islong = 1;
      }

      // Check to see if the end of line was reached
      // If so, print an error and ask for new user input
      if (islong) {
      	printf("Letter count must be 128 characters or less!\n");
      	continue;
      }
    }
		//Grabs everything before the new line and puts it into usrinput
		token = strtok(usrinput, "\n");

		//handle exit
		if (strcmp(token, "exit") == 0){
			break;
		}

		//Tokening user input

		token = strtok(token, " ");
		int i = 0;


		while (token != NULL ){
			if (i>MAX_ARG){
				printf("You can only use less than 32 arguments\n");
				toomanyargflag = 1;
	   	 	break;
			}

			argv[i] = token;
			token = strtok(NULL, " ");
			i++;
		}
		//There's too many arg and thus goes back to the beginning
		if (toomanyargflag == 1){
			continue;
		}

		argv[i] = NULL;

    if (strcmp("cd",argv[0]) == 0){
      // do cd
       if(chdir(argv[1]) == -1){
	printf("Directory doesn't exist.\n");
       }
      getcwd(cwd, BUFFER_LEN);
      continue;
    }
		int pid = fork(); //Make the process have an id

		if (pid != 0) {
			//This right here is the parent process
			int status;
			struct timeval timeBefore;
			struct timeval timeAfter;

			//Now get the time we start this command
			gettimeofday(&timeBefore, NULL);

			//Waiting for child to finish
			waitpid(pid, &status, 0);
			//And if it works correcctly, print out stats 

			if (WEXITSTATUS(status) == 0) {
				//Get time AFTER it's finished
				gettimeofday(&timeAfter, NULL);

				//Get stats of child process 
				struct rusage udata;
				getrusage(RUSAGE_CHILDREN, &udata);

				//Prints them out with our funct 
				stats(udata, oldudata, hasrun, timeBefore, timeAfter);
        
        oldudata = udata;
        
        hasrun = 1;
			}
		}else{
		//Child process here 
    	int result = execvp(argv[0], &argv[0]);
    	if (result == -1) {
    		printf("Invalid command! Err Num: %i\n", errno);
      	exit(1);
    	}
    toomanyargflag = 0;
    }
	}
	return 0;
}


//finds the time difference between the start and end of the process
long findTimeDif(struct timeval timeBefore, struct timeval timeAfter){
//Gets the diff and converts to microsec
	long diff = (long) ((timeAfter.tv_sec - timeBefore.tv_sec) * 1000000);
	
	long microDiff = (long) (timeAfter.tv_usec - timeBefore.tv_usec);

	diff = diff + microDiff;
	//converts to milliseconds
	return (diff / 1000);

}


//retrieves stats from the process running
void stats(struct rusage udata, struct rusage oldudata, int hasrun, struct timeval timeBefore, struct timeval timeAfter){

  long diff, userTime, sysTime, involsw, volsw, pgfaulthard, pgfaultsoft;
  if(hasrun == 1){
 
		diff = findTimeDif(timeBefore, timeAfter);

		//Takes the cumulative stats and subtracts from the old stats in order to find just 1 process stats.
		userTime = ((udata.ru_utime.tv_sec - oldudata.ru_utime.tv_sec )* 1000) + ((udata.ru_utime.tv_usec - oldudata.ru_utime.tv_usec) / 1000);
		sysTime = ((udata.ru_stime.tv_sec - oldudata.ru_stime.tv_sec) * 1000) + ((udata.ru_utime.tv_usec - oldudata.ru_utime.tv_usec)/ 1000);
		involsw = (udata.ru_nivcsw - oldudata.ru_nivcsw);
		volsw = (udata.ru_nvcsw - oldudata.ru_nvcsw);
		pgfaulthard = (udata.ru_majflt - oldudata.ru_majflt);
		pgfaultsoft = (udata.ru_minflt - oldudata.ru_minflt);
  }else{
  	diff = findTimeDif(timeBefore, timeAfter);
		userTime = (udata.ru_utime.tv_sec * 1000) + (udata.ru_utime.tv_usec / 1000);
		sysTime = (udata.ru_stime.tv_sec * 1000) + (udata.ru_utime.tv_usec / 1000);
		involsw = (udata.ru_nivcsw);
		volsw = (udata.ru_nvcsw);
		pgfaulthard = (udata.ru_majflt);
		pgfaultsoft = (udata.ru_minflt);
  }

	//print stats
	printf("\n");
	printf("User time is %ld mseconds\n", diff);
	printf("CPU Time is %ld mseconds\n", userTime);
	printf("Sys Time is %ld mseconds\n", sysTime);
	printf("No. of Involuntary Switches %ld \n", involsw);
	printf("No. of Voluntary Switches %ld \n", volsw);
	printf("No. of hard faults %ld \n", pgfaulthard);
	printf("No. of soft faults that could be reclaimed %ld \n", pgfaultsoft);
	
}

