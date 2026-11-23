#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

void handler(int signo)
{
	static int turn = 0;
	if (turn == 0) {
		printf("tick\n");
	}
	else {
		printf("tock\n");
	}
	turn = !turn;
}

int main(int argc, char **argv)
{
	// assign our handler to SIGUSR1
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);

	// our timer will signal SIGUSR1 instead of SIGALRM
	timer_t timerid;
	struct sigevent se;
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = SIGUSR1;
	timer_create(CLOCK_REALTIME, &se, &timerid);

	//Set up the structs (fire first at 5 s, then every 1 s)
	struct itimerspec value;
	value.it_interval.tv_sec = 1;
	value.it_interval.tv_nsec = 0;
	value.it_value.tv_sec = 5;
	value.it_value.tv_nsec = 0;

	// start the timer
	timer_settime(timerid, 0, &value, NULL);

	// don't exit too soon
	for (;;) {
		pause();
	}

	return EXIT_SUCCESS;
}
