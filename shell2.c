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

struct job {
  	int pid;
  	char* command;
  	struct timeval timeStart;
};

struct listofJob {
  struct job *ajob;
  struct listofJob *nextJob;
};

long findTimeDif(struct timeval timeBefore, struct timeval timeAfter);
void bgJobs(struct listofJob *list, struct listofJob **firstJob);
void rembgJob(struct listofJob *ajob, struct listofJob **current);
void printInfo(struct listofJob *alist, struct listofJob *target, int index);
void printjobs(struct listofJob *alist, int index);
struct listofJob* storeJob(struct listofJob *jobs, int pid, char* command, struct timeval timeBefore);
void stats(struct rusage udata, struct timeval timeBefore, struct timeval timeAfter);

int main(){
  struct listofJob *JobsInBack = malloc(sizeof(struct listofJob));
  struct rusage udata;
  int toomanyargflag = 0;
  char cwd[1024];
  int isbackground = 0; //checks to see if something is running in background
  int result = 1;
  getcwd(cwd, BUFFER_LEN);
	while(1){
		int argc;
		char usrinput[BUFFER_LEN];
		char* argv[MAX_ARG];
		char *token;
    

		printf("%s>ourshell$ ",cwd); //Prints our shell so it somewhat mimics an actual shell

    if (fgets(usrinput, BUFFER_LEN, stdin) == NULL) { //Checks if a null command is sent in
	bgJobs(JobsInBack, &JobsInBack);//Processes the bg jobs
	if(!(JobsInBack->ajob == NULL && JobsInBack->nextJob == NULL)) { //Doesn't exit while something is running
		printf("Still background processes running. Please wait for completion.\n\n");
		while(!(JobsInBack->ajob == NULL && JobsInBack->nextJob == NULL)) {
			bgJobs(JobsInBack, &JobsInBack);
		}
	}
    	printf("\n");
    	exit(0);
    }


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

    token = strtok(usrinput, "\n");


    //set background process flag
    if (token[strlen(token)-1] == '&'){ 
      isbackground = 1;
      token = strtok(token, "&");
    }
		//handle exit
		if (strcmp(token, "exit") == 0){
			bgJobs(JobsInBack, &JobsInBack);
			if(!(JobsInBack->ajob == NULL && JobsInBack->nextJob == NULL)) {
				printf("Still background processes running. Please wait for completion.\n\n");
				while(!(JobsInBack->ajob == NULL && JobsInBack->nextJob == NULL)) {
				bgJobs(JobsInBack, &JobsInBack);
				}
			}
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
    
		if (toomanyargflag == 1){
			continue;
		}

		argv[i] = NULL;

    if (strcmp("cd",argv[0]) == 0){
      // do cd
	bgJobs(JobsInBack, &JobsInBack);
       if(chdir(argv[1]) == -1){
					printf("Directory doesn't exist.\n");
       }
      getcwd(cwd, BUFFER_LEN);
      continue;
    }

	if (strcmp("jobs",argv[0]) == 0){
 		if(JobsInBack->ajob == NULL){
			printf("Currently no jobs\n");
			continue;
		}
	bgJobs(JobsInBack, &JobsInBack);
	printjobs(JobsInBack, 1);
	continue;
	}
		int pid = fork(); //Make the process have an id

		if (pid != 0) {
			//This right here is the parent process
			int status;
			struct timeval timeBefore;
			struct timeval timeAfter;
			int statsshown = 1;
			
			bgJobs(JobsInBack, &JobsInBack);
			//Now get the time we start this command
			gettimeofday(&timeBefore, NULL);

      if (isbackground != 1){
				//Wait for specific child to finish
				result = wait4(pid, &status, 0, &udata);
      }else{//don't wait and reset flag

	result = wait4(pid, &status, WNOHANG, &udata);
	if (result <= 0) {
		struct listofJob *newjobs;
		newjobs = storeJob(JobsInBack, pid, argv[0], timeBefore);

		printInfo(JobsInBack, newjobs, 1);

		statsshown = 0; //Tells us that stats have not been shown

	}
		
      }
			//And if it works correcctly, print out stats

			if (statsshown && WEXITSTATUS(status) == 0) {
				//Get time AFTER it's finished
				gettimeofday(&timeAfter, NULL);


				//Prints them out with our funct (WIP)
				stats(udata, timeBefore, timeAfter);
				

			}
		}else{
		//Child process here
    	result = execvp(argv[0], &argv[0]);

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

void rembgJob(struct listofJob *ajob, struct listofJob **current) {


    if ((*current)->nextJob == NULL) { //Checks if it's the only job in list       
        (*current)->ajob = NULL;
    } else if (ajob == (*current)) { //Removes the job
        *current = (*current)->nextJob;
    } else if (ajob == (*current)->nextJob) {
        (*current)->nextJob = ajob->nextJob;
        free(ajob->ajob->command);
        free(ajob->ajob);
        free(ajob);
    } else {
        rembgJob(ajob, &(*current)->nextJob);
    }
}

void printInfo(struct listofJob *alist, struct listofJob *target, int index) {
    if (alist == target) {
        printf("Job [%i] %i %s\n", index, target->ajob->pid, target->ajob->command);
    } else if (alist != NULL) {
        printInfo(alist->nextJob, target, index + 1);
    }
}

void bgJobs(struct listofJob *list, struct listofJob **firstJob) {
    if ((list != NULL) && (list->ajob != NULL)) {
        int status;
	int result;
        struct rusage stat;
        struct timeval timeEnd;

        result = wait4(list->ajob->pid, &status, WNOHANG, &stat);
        if (result > 0) {
 
            gettimeofday(&timeEnd, NULL);
        
            stats(stat, list->ajob->timeStart, timeEnd);//prints stats
	    printf("%s job has finished. PID: %i \n", list->ajob->command, list->ajob->pid);
            rembgJob(list, firstJob);//removes job from list
        }
        bgJobs(list->nextJob, firstJob);//repeats until none exist
    }
}

struct listofJob* storeJob(struct listofJob *jobs, int pid, char* command, struct timeval timeBefore) {
 
    struct job *newJob = malloc(sizeof(struct job));
    newJob->pid = pid;
    newJob->command = strdup(command);
    newJob->timeStart = timeBefore;

    // Checks if there's a job already
    if (jobs->ajob != NULL) {
        // Find the last job
        struct listofJob *alist = jobs;
        while (alist->nextJob != NULL) {
            alist = alist->nextJob;
        }

        // Make a new list of jobs
        struct listofJob *newList = malloc(sizeof(struct listofJob));
        newList->ajob = newJob;
        newList->nextJob = NULL;
        alist->nextJob = newList;
        return newList;
    } else {
        // First job, so just set values = to that.
        jobs->ajob = newJob;
        jobs->nextJob = NULL;
        return jobs;
    }
}

void printjobs(struct listofJob *alist, int index) {
    if (alist != NULL && alist->ajob != NULL) {
        printf("Job [%d] %d %s\n", index, alist->ajob->pid, alist->ajob->command);
        printjobs(alist->nextJob, index + 1);
    }
}
//retrieves stats from the process running
void stats(struct rusage udata, struct timeval timeBefore, struct timeval timeAfter){

  long diff, userTime, sysTime, involsw, volsw, pgfaulthard, pgfaultsoft;
 
  	diff = findTimeDif(timeBefore, timeAfter);
		userTime = (udata.ru_utime.tv_sec * 1000) + (udata.ru_utime.tv_usec / 1000);
		sysTime = (udata.ru_stime.tv_sec * 1000) + (udata.ru_utime.tv_usec / 1000);
		involsw = (udata.ru_nivcsw);
		volsw = (udata.ru_nvcsw);
		pgfaulthard = (udata.ru_majflt);
		pgfaultsoft = (udata.ru_minflt);


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


