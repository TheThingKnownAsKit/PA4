/****************************************************************
 *
 * Author: Justin Bradley
 * Title: signal_timer.c
 * Date: Tuesday, March 29, 2022
 * Description: Illustrative example of how to use signals and timers in Linux
 * with a focus on what is required for PA4.
 *
 * In this example a thread is created which sits in a while(1) loop, sleeping
 * for 1 second between printing a message. A timer is setup to send a SIGALRM
 * in <timer_val> seconds. A SIGALRM handler then kills the thread.
 *
 ****************************************************************/

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


static timer_t timer;
pthread_t thrid;
time_t timer_val = 10; // set 10 seconds

/***********************************************************
 * Function: thread_task -- dumb thread task that just sleeps
 * Parameters: 
 * Return Value: 
 ***********************************************************/
void * thread_task()
{
     // wait for signal
     int i=1;
     while(1)
     {
          printf("Sleeping...%d\n",i);
          sleep(1);
          i++;
     }
		
}


/***********************************************************
 * Function: handle_timer -- handles the signal sent by the timer expiring
 * Parameters: 
 * Return Value: 
 ***********************************************************/
void handle_timer()
{
     // kill the thread
     pthread_kill(thrid, SIGTERM);
}

int main(int argc, char ** argv) 
{
     /******************************************************
      * Setup the timer
      ******************************************************/

     // look here: https://man7.org/linux/man-pages/man7/sigevent.7.html
     struct sigevent sev; 
     sev.sigev_notify = SIGEV_SIGNAL; // notification performed via signal     
     sev.sigev_signo = SIGALRM; // the signal that should be sent 
     sev.sigev_value.sival_ptr = &timer; // indicate the time 
     timer_create(CLOCK_REALTIME,&sev,&timer); 

     // Normally you'd need to do all the above to setup the timer. BUT...
     /* Neat trick: from https://man7.org/linux/man-pages/man2/timer_create.2.html
        Specifying sevp as NULL is equivalent to specifying a pointer to
        a sigevent structure in which sigev_notify is SIGEV_SIGNAL,
        sigev_signo is SIGALRM, and sigev_value.sival_int is the timer
        ID.

        ...which is just what we need!
     */

     // create the timer - defaults to sending SIGALRM -- need handler
     //timer_create(CLOCK_REALTIME, NULL, &timer);


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
      * Now start the timer
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
      * Wait for the thread to join
      ******************************************************/
     pthread_join(thrid, NULL);


}
