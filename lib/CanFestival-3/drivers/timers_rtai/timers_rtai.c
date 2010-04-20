#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <sys/poll.h>
#include <rtai_lxrt.h>

#include <rtai_sem.h>
#include <pthread.h>
#include <errno.h>

#include "applicfg.h"
#include "can_driver.h"
#include "timer.h"

#define TIMERLOOP_TASK_CREATED        1

TimerCallback_t exitall;

SEM *CanFestival_mutex;
SEM *condition_mutex;
SEM *control_task; 
CND *timer_set;

// realtime task structures
RT_TASK *timerloop_task;
RT_TASK *Main_Task;

// linux threads id's
static pthread_t timerloop_thr;
 
RTIME last_time_read;
RTIME last_occured_alarm;
RTIME last_timeout_set;

int stop_timer = 0;

void TimerInit(void)
{
    /* Init Main Task */
    if (!(Main_Task = rt_thread_init(rt_get_name(0), 0, 0, SCHED_FIFO, 1))) {
            printf("CANNOT INIT MAIN TASK\n");
            exit(1);
    }
  	
  	/* Init Mutex */
	CanFestival_mutex = rt_sem_init(rt_get_name(0), 1);
	condition_mutex = rt_typed_sem_init(rt_get_name(0), 1, RES_SEM);
	timer_set = rt_cond_init(rt_get_name(0));
	control_task = rt_sem_init(rt_get_name(0), 0);
	/* Set timer mode and start timer */
	rt_set_oneshot_mode();
	start_rt_timer(0);
}

/**
 * Stop Timer Task
 * @param exitfunction
 */
void StopTimerLoop(TimerCallback_t exitfunction)
{
	exitall = exitfunction;
	stop_timer = 1;
	rt_cond_signal(timer_set);
}

/**
 * Clean all Semaphores and main task
 */
void TimerCleanup(void)
{
	/* Stop timer */
	stop_rt_timer();

	/* Delete all mutex and the main task */
	rt_sem_delete(CanFestival_mutex);
	rt_sem_delete(condition_mutex);
	rt_sem_delete(timer_set);
	rt_sem_delete(control_task);
	rt_thread_delete(Main_Task);
}

/**
 * Take a semaphore
 */
void EnterMutex(void)
{
	rt_sem_wait(CanFestival_mutex);
}

/**
 * Signaling a semaphore
 */
void LeaveMutex(void)
{
	rt_sem_signal(CanFestival_mutex);
}

static TimerCallback_t init_callback;

/**
 * Timer Task
 */
void timerloop_task_proc(void *arg)
{
	int ret = 0;
  	// lock process in to RAM
  	mlockall(MCL_CURRENT | MCL_FUTURE);
	timerloop_task = rt_thread_init(rt_get_name(0), 0, 0, SCHED_FIFO, 1);
	rt_make_hard_real_time();

	getElapsedTime();
	last_timeout_set = 0;
	last_occured_alarm = last_time_read;

	/* trigger first alarm */
	SetAlarm(NULL, 0, init_callback, 0, 0);
	
	do{
		RTIME real_alarm;
		rt_sem_wait(condition_mutex);
		if(last_timeout_set == TIMEVAL_MAX)
		{
			ret = rt_cond_wait(
				timer_set,
				condition_mutex);		/* Then sleep until next message*/
 
			rt_sem_signal(condition_mutex);
		}else{
			real_alarm = last_time_read + last_timeout_set;
			ret = rt_cond_wait_until(
				timer_set,
				condition_mutex,
				real_alarm); /* Else, sleep until next deadline */
			if(ret == SEM_TIMOUT){
				last_occured_alarm = real_alarm;
				rt_sem_signal(condition_mutex);
				EnterMutex();
				TimeDispatch();
				LeaveMutex();
			}else{ 
				rt_sem_signal(condition_mutex);
			}
		}
	}while ( ret != SEM_ERR && !stop_timer);
	if(exitall){
		EnterMutex();
		exitall(NULL,0);
		LeaveMutex();
	}
	rt_make_soft_real_time();
	rt_thread_delete(timerloop_task);
}

/**
 * Create the Timer Task
 * @param _init_callback
 */
void StartTimerLoop(TimerCallback_t _init_callback)
{
	stop_timer = 0;	
	init_callback = _init_callback;
	
	/* start timerloop_task ( do nothing and get blocked ) */
	timerloop_thr = rt_thread_create(timerloop_task_proc, NULL, 0);
}

/* We assume that ReceiveLoop_task_proc is always the same */
static void (*rtai_ReceiveLoop_task_proc)(CAN_PORT) = NULL;

/**
 * Enter in realtime and start the CAN receiver loop
 * @param port
 */
void rtai_canReceiveLoop(CAN_PORT port)
{
	RT_TASK *current_task;
	mlockall(MCL_CURRENT | MCL_FUTURE);
	current_task = rt_thread_init(rt_get_name(0), 0, 0, SCHED_FIFO, 1);
	rt_make_hard_real_time();

	rt_sem_signal(control_task);

	/* Call original receive loop with port struct as a param */
	rtai_ReceiveLoop_task_proc(port);

	rt_make_soft_real_time();
	rt_thread_delete(current_task);
}

/**
 * Create the CAN Receiver Task
 * @param fd0 CAN port
 * @param *ReceiveLoop_thread CAN receiver thread
 * @param *ReceiveLoop_task_proc CAN receiver task
 */
void CreateReceiveTask(CAN_PORT fd0, TASK_HANDLE *ReceiveLoop_thread, void* ReceiveLoop_task_proc)
{	
	rtai_ReceiveLoop_task_proc = ReceiveLoop_task_proc;
	*ReceiveLoop_thread = rt_thread_create(rtai_canReceiveLoop, (void*)fd0, 0); 
	rt_sem_wait(control_task);
}

/**
 * Wait for the CAN Receiver Task end
 * @param *ReceiveLoop_thread CAN receiver thread
 */
void WaitReceiveTaskEnd(TASK_HANDLE *ReceiveLoop_thread)
{
	rt_thread_join(*ReceiveLoop_thread);
}

/**
 * Set timer for the next wakeup
 * @param value
 */
void setTimer(TIMEVAL value)
{
	rt_sem_wait(condition_mutex);
	last_timeout_set = value;
	rt_sem_signal(condition_mutex);
	rt_cond_signal(timer_set);
}

/**
 * Get the elapsed time since the last alarm
 * @return a time in nanoseconds
 */
TIMEVAL getElapsedTime(void)
{
	RTIME res;
	rt_sem_wait(condition_mutex);
	last_time_read = rt_get_time();
	res = last_time_read - last_occured_alarm;
	rt_sem_signal(condition_mutex);
	return res;
}
