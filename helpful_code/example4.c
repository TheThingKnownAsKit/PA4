#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


static timer_t timer;
pthread_t thrid;
time_t timer_val = 3; // seconds

void * thread_task()
{
     // setup to wait for a signal
     sigset_t signal_wait_set;
     sigemptyset (&signal_wait_set); // must call this or results are undefined
     sigaddset(&signal_wait_set, SIGUSR2); // let's wait for SIGUSR2
     int sig;

     // We need to know whether to block or unblock the signal from the set of
     // blocked signals. See here:
     // https://man7.org/linux/man-pages/man2/sigprocmask.2.html
     // Another way to think about it is that sigwait() waits for a signal to
     // become "pending". It then accepts it. To make a signal "pending" it
     // needs to be sent when it is "blocked". If the signal is "unblocked" it
     // is received by the thread.
     // If you unblock the signal the default action will occur.
     // What is the default action of SIGUSR2? Exit with "User defined signal 2

     //pthread_sigmask(SIG_UNBLOCK, &signal_wait_set, NULL); // remove set from
     // blocked signals
     pthread_sigmask(SIG_BLOCK, &signal_wait_set, NULL); // add set to
     // blocked signals
		
     // wait for signal		
     int i=1;
     while(1)
     {
          printf("Waiting for a SIGUSR2...%d\n",i);
          // now wait for the signal
          sigwait(&signal_wait_set, &sig);
          printf("Got the signal...waking up.\n\n");
          i++;
     }
		
}


void handle_timer()
{
     // send the SIGUSR2 signal to the thread
     pthread_kill(thrid, SIGUSR2);
}

int main(int argc, char ** argv) 
{
     /******************************************************
      * Setup the timer
      ******************************************************/
     /* Neat trick: from https://man7.org/linux/man-pages/man2/timer_create.2.html
        Specifying sevp as NULL is equivalent to specifying a pointer to
        a sigevent structure in which sigev_notify is SIGEV_SIGNAL,
        sigev_signo is SIGALRM, and sigev_value.sival_int is the timer
        ID.
     */

     // create the timer - defaults to sending SIGALRM -- need handler
     timer_create(CLOCK_REALTIME, NULL, &timer);


     /******************************************************
      * Setup the SIGALRM handler
      ******************************************************/

     // Setup handler for SIGALRM
     // https://man7.org/linux/man-pages/man2/sigaction.2.html
     struct sigaction sa_alrm;
     sa_alrm.sa_handler = handle_timer;
     sigemptyset(&sa_alrm.sa_mask); // clear the sa_mask
     sa_alrm.sa_flags = 0;
     sigaction(SIGALRM, &sa_alrm, NULL); // NULL = no old action to save

     /******************************************************
      * Create the thread
      ******************************************************/
     pthread_create(&thrid, NULL, thread_task, NULL);

     /******************************************************
      * now start the timer
      ******************************************************/

     // https://man7.org/linux/man-pages/man2/timer_gettime.2.html
     struct itimerspec new_value;
     // it_value = initial timer
     new_value.it_value.tv_sec = timer_val;
     new_value.it_value.tv_nsec = 0;
     // it_interval = allows the time to reload and start counting down again.
     new_value.it_interval.tv_sec = timer_val;
     new_value.it_interval.tv_nsec = 0;
     // now set the timer
     timer_settime(timer, 0, &new_value, NULL);

     /******************************************************
      * Wait for thread to join
      ******************************************************/
     pthread_join(thrid, NULL);
}
