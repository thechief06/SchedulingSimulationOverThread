A. Objective : To design a scheduler and a process control block (pcb),
pcb-table and queue structures of the sample OS using threads, to
demonstrate the correct operation of scheduler on at least three
processes simulated by threads.

B. Procedure: You will design and implement the following:

	1. The Scheduler: The scheduler schedules processes from two ready
	queues; a First Come First Served (FCFS) system processes queue and a
	real-time priority based (PB) queue. A thread is blocked by waiting on a
	semaphore and resumed by a signal operation on that semaphore. When a
	new task is awakened, the running task may be preempted if it is a PB
	task.
	Unlike a real scheduler, however, our simulation must suspend all
	threads in the ready queues by making them wait on semaphores and
	activate one of them by signalling its semaphore to make sure that
	only one of them is running at a time. The job of the scheduler is to
	find the next pid from the ready queues.
	The scheduler may be implemented by a thread function:
	- void *scheduler(void *anyParam);

	2. Data structures: The following data structures are needed:
	-pcb : Includes task id, priority, type, data area, semaphore etc.
	-pcb_table : An array consisting of pcb elements. A pcb's id (pid)
	is its index in this array
	-queue : Consists of state, front and rear pointers to pcb's

	3. Operations on data structures:
	-int enqueue_proc(pid,queue): Enqueues the pid to the given queue
	-pcbptr dequeue_proc(queue): Dequeues the pid from the given queue
	-int insert_proc(pid,queue): Inserts a pid to the given queue w.r.t.
	its priority
	-int delete_proc(pid,queue): Deletes a pid from the given queue
	-int checkque(queue) : Checks whether a queue is empty or not
	-int make_proc(address, name, type, prio, ..): Creates a process
	with the given starting address, type and priority (if
	realtime). A created process (thread) must be made waiting in
	its semaphore in its ready queue. The returned integer is the
	newly created PID.
	-pcbptr del_proc(queue): Deletes a process from the system. The pcb
	should be deallocated for future use.
	-int block_proc(pid,queue): Changes the state of the process to
	BLOCKED and calls the Scheduler.
	-int unblock_proc(pid,queue): Changes the state of the process to
	READY and calls the Scheduler.
C. Demo: You need to create your own scenario to demonstrate that the
scheduler works. You are requried to create at least three threads that
will simulate the processes to be scheduled by the scheduler thread,
which may call the functions:
- void *TaskA(void *anyParam);
- void *TaskB(void *anyParam);
- void *TaskC(void *anyParam);