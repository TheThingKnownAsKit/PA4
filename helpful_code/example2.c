#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>


double time_function(void (*func)(void))
{
	struct timespec tend, tstart;

	clock_gettime(CLOCK_REALTIME, &tstart);
	func();
	clock_gettime(CLOCK_REALTIME, &tend);
	return tend.tv_sec - tstart.tv_sec
		+ ((double)tend.tv_nsec - tstart.tv_nsec)/1e9;
}

void spin_func()
{
	int i;
	for (i = 0; i < 1e6; i++) { }
}

void yield_func()
{
	int i;
	for (i = 0; i < 1e6; i++) { sched_yield(); }
}

int main(int argc, char **argv)
{
	printf("spin_func took %lf seconds\n", time_function(spin_func));
	printf("yield_func took %lf seconds\n", time_function(yield_func));
	return EXIT_SUCCESS;
}
