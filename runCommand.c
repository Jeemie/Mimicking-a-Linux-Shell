//Krzysztof Borowicz, Hyunsoo Kim, Jimmy Tran
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

long findTimeDif(struct timeval timeBefore, struct timeval timeAfter);
void stats(struct rusage udata, struct timeval timeBefore, struct timeval timeAfter);

int main(int argc, char* argv[]){

	//Checks to make sure there's actually some arguments.
	if(argc < 2) {
		printf("Please provide a command and arguments if needed.\n");
		return 1;

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
			stats(udata, timeBefore, timeAfter);
		}
	}else{
	//Child process here
		int result = execvp(argv[1], &argv[1]);
		if (result == -1) {
		printf("Invalid command! Err Num: %i\n", errno);
		exit(1);
	 	}
	}		
	return 0;
	}
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
void stats(struct rusage udata, struct timeval timeBefore, struct timeval timeAfter){

	long diff = findTimeDif(timeBefore, timeAfter);
	long userTime = (udata.ru_utime.tv_sec * 1000) + (udata.ru_utime.tv_usec / 1000);
	long sysTime = (udata.ru_stime.tv_sec * 1000) + (udata.ru_utime.tv_usec / 1000);
	long involsw = (udata.ru_nivcsw);
	long volsw = (udata.ru_nvcsw);
	long pgfaulthard = (udata.ru_majflt);
	long pgfaultsoft = (udata.ru_minflt);

	//print stats
	printf("User time is %ld mseconds\n", diff);
	printf("CPU Time is %ld mseconds\n", userTime);
	printf("Sys Time is %ld mseconds\n", sysTime);
	printf("No. of Involuntary Switches %ld \n", involsw);
	printf("No. of Voluntary Switches %ld \n", volsw);
	printf("No. of hard faults %ld \n", pgfaulthard);
	printf("No. of soft faults that could be reclaimed %ld \n", pgfaultsoft);
}


