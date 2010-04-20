#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <native/mutex.h>
#include <native/cond.h>
#include <native/alarm.h>

#include "applicfg.h"
#include "can_driver.h"
#include "timer.h"

#define TIMERLOOP_TASK_CREATED        1

TimerCallback_t exitall;

RT_MUTEX condition_mutex;
RT_SEM CanFestival_mutex;
RT_SEM control_task; 
RT_COND timer_set;	
RT_TASK timerloop_task;
 
RTIME last_time_read;
RTIME last_occured_alarm;
RTIME last_timeout_set;

int stop_timer = 0;

/**
 * Init Mutex, Semaphores and Condition variable
 */
void TimerInit(void)
{
  	int ret = 0;
  	char taskname[32];

	// lock process in to RAM
  	mlockall(MCL_CURRENT | MCL_FUTURE);

  	snprintf(taskname, sizeof(taskname), "S1-%d", getpid());
	rt_sem_create(&CanFestival_mutex, taskname, 1, S_FIFO);

  	snprintf(taskname, sizeof(taskname), "S2-%d", getpid());
  	rt_sem_create(&control_task, taskname, 0, S_FIFO);
  	  	
  	snprintf(taskname, sizeof(taskname), "M1-%d", getpid());
  	rt_mutex_create(&condition_mutex, taskname);
  	
  	snprintf(taskname, sizeof(taskname), "C1-%d", getpid());
  	rt_cond_create(&timer_set, taskname);
}

/**
 * Stop Timer Task
 * @param exitfunction
 */
void StopTimerLoop(TimerCallback_t exitfunction)
{
	exitall = exitfunction;
	stop_timer = 1;
	rt_cond_signal(&timer_set);
}

void cleanup_all(void)
{
	if (rt_task_join(&timerloop_task) != 0){
		printf("Failed to join with Timerloop task\n");
	}
	rt_task_delete(&timerloop_task);
}

/**
 * Clean all Semaphores, mutex, condition variable and main task
 */
void TimerCleanup(void)
{
	rt_sem_delete(&CanFestival_mutex);
	rt_mutex_delete(&condition_mutex);
	rt_cond_delete(&timer_set);
	rt_sem_delete(&control_task);
	if (rt_task_join(&timerloop_task) != 0){
		printf("Failed to join with Timerloop task\n");
	}
	rt_task_delete(&timerloop_task);
}

/**
 * Take a semaphore
 */
void EnterMutex(void)
{
	rt_sem_p(&CanFestival_mutex, TM_INFINITE);
}

/**
 * Signaling a semaphore
 */
void LeaveMutex(void)
{
	rt_sem_v(&CanFestival_mutex);
}

static TimerCallback_t init_callback;

/**
 * Timer Task
 */
void timerloop_task_proc(void *arg)
{
	int ret = 0;

	getElapsedTime();
	last_timeout_set = 0;
	last_occured_alarm = last_time_read;
	
	/* trigger first alarm */
	SetAlarm(NULL, 0, init_callback, 0, 0);
	RTIME current_time;
	RTIME real_alarm;
	do{
		
		rt_mutex_acquire(&condition_mutex, TM_INFINITE);
		if(last_timeout_set == TIMEVAL_MAX)
		{
			ret = rt_cond_wait(
				&timer_set,
				&condition_mutex,
				TM_INFINITE
				);		/* Then sleep until next message*/
			rt_mutex_release(&condition_mutex);
		}else{
			current_time = rt_timer_read();
			real_alarm = last_time_read + last_timeout_set;
			ret = rt_cond_wait( /* sleep until next deadline */
				&timer_set,
				&condition_mutex,
				(real_alarm - current_time)); /* else alarm consider expired */   
			if(ret == -ETIMEDOUT){
				last_occured_alarm = real_alarm;
				rt_mutex_release(&condition_mutex);
				EnterMutex();
				TimeDispatch();
				LeaveMutex();
			}else{ 
				rt_mutex_release(&condition_mutex);
			}
		}
	}while ((ret == 0 || ret == -EINTR || ret == -ETIMEDOUT) && !stop_timer);
	
	if(exitall){
		EnterMutex();
		exitall(NULL,0);
		LeaveMutex();
	}
}

/**
 * Create the Timer Task
 * @param _init_callback
 */
void StartTimerLoop(TimerCallback_t _init_callback)
{
	int ret = 0;
	stop_timer = 0;	
	init_callback = _init_callback;
	
	char taskname[32];
	snprintf(taskname, sizeof(taskname), "timerloop-%d", getpid());

	/* create timerloop_task */
	ret = rt_task_create(&timerloop_task, taskname, 0, 50, T_JOINABLE);
	if (ret) {
		printf("Failed to create timerloop_task, code %d\n",errno);
		return;
	}
 	
	/* start timerloop_task */
	ret = rt_task_start(&timerloop_task,&timerloop_task_proc,NULL);
	if (ret) {
		printf("Failed to start timerloop_task, code %u\n",errno);
		goto error;
	}
	
	return;
	
error:
	cleanup_all();
}

/**
 * Create the CAN Receiver Task
 * @param fd0 CAN port
 * @param *ReceiveLoop_task CAN receiver task
 * @param *ReceiveLoop_task_proc CAN receiver function
 */
void CreateReceiveTask(CAN_PORT fd0, TASK_HANDLE *ReceiveLoop_task, void* ReceiveLoop_task_proc)
{	
	int ret;
	static int id = 0;
	char taskname[32];
	snprintf(taskname, sizeof(taskname), "canloop%d-%d", id, getpid());
	id++;

	/* create ReceiveLoop_task */
	ret = rt_task_create(ReceiveLoop_task,taskname,0,50,T_JOINABLE);
	if (ret) {
		printf("Failed to create ReceiveLoop_task number %d, code %d\n", id, errno);
		return;
	}
	/* start ReceiveLoop_task */
	ret = rt_task_start(ReceiveLoop_task, ReceiveLoop_task_proc,(void*)fd0);
	if (ret) {
		printf("Failed to start ReceiveLoop_task number %d, code %d\n", id, errno);
		return;
	}
	rt_sem_v(&control_task);
}

/**
 * Wait for the CAN Receiver Task end
 * @param *ReceiveLoop_task CAN receiver thread
 */
void WaitReceiveTaskEnd(TASK_HANDLE *ReceiveLoop_task)
{
	if (rt_task_join(ReceiveLoop_task) != 0){
		printf("Failed to join with Receive task\n");
	}
	rt_task_delete(ReceiveLoop_task);
}

/**
 * Set timer for the next wakeup
 * @param value
 */
void setTimer(TIMEVAL value)
{
	rt_mutex_acquire(&condition_mutex, TM_INFINITE);
	last_timeout_set = value;
	rt_mutex_release(&condition_mutex);
	rt_cond_signal(&timer_set);
}

/**
 * Get the elapsed time since the last alarm
 * @return a time in nanoseconds
 */
TIMEVAL getElapsedTime(void)
{
	RTIME res;
	rt_mutex_acquire(&condition_mutex, TM_INFINITE);
	last_time_read = rt_timer_read();
	res = last_time_read - last_occured_alarm;
	rt_mutex_release(&condition_mutex);
	return res;
}
