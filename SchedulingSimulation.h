#ifndef __SCHEDULING_SIMULATION_H__
#define __SCHEDULING_SIMULATION_H__

#define TRUE	1
#define FALSE	0



//Queue Data
typedef int Data;

typedef struct _node
{
	struct _node * next;
     Data data;
} Node;

typedef struct _lQueue
{
	Node * front;
	Node * rear;
} LQueue;

typedef LQueue Queue;

void queueInit(Queue * pq);
void taskImpl(int pick);
int checkque(Queue * pq);

void enqueueProc(Queue * pq, Data data);
Data dequeueProc(Queue * pq);
Data qPeek(Queue * pq);

#endif