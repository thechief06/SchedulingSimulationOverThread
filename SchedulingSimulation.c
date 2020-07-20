#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include "SchedulingSimulation.h"

/* 
--------------------------------------------------------------
|                         CPU SCHEDULING                     |
|                 BIL 616 SYSTEM PROGRAMING COURSE           |
|                      Developed by gyurtalan                |
--------------------------------------------------------------
 */

// toplam process sayisi tanimlanir
#define TOTAL_PID 4

Queue ready_q, running_q;
int waiting_q[TOTAL_PID];
int newest_PID = 0;
int counter;

struct pcb
{
    int pid;
    int cpu_burst;
    int io_burst;
    int arr_time;
    int comp_time; //completion time
    int progress;
    int priority;
    sem_t sem;
    int semNext; // bu degiskeni bir semaphore in post islemi icin statik olusturmak ve atamak zorunda kaldim
    pthread_t thread;
    _Bool start;
    char taskId;
    int preemptive;
} * pcb_table[TOTAL_PID];

//  Queue default degerleri set edilir
void queueInit(Queue *pq)
{
    pq->front = NULL;
    pq->rear = NULL;
}

int checkque(Queue *pq)
{
    if (pq->front == NULL)
        return TRUE;
    else
        return FALSE;
}

void enqueueProc(Queue *pq, Data data)
{

    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->next = NULL;
    newNode->data = data;

    if (checkque(pq))
    {
        pq->front = newNode;
        pq->rear = newNode;
    }
    else
    {
        pq->rear->next = newNode;
        pq->rear = newNode;
    }
}

Data dequeueProc(Queue *pq)
{
    Node *delNode;
    Data retData;

    if (checkque(pq))
    {
        printf("Queue Memory Hatasi![dequeueProc]");
        exit(-1);
    }
    delNode = pq->front;
    retData = delNode->data;
    pq->front = pq->front->next;

    free(delNode);
    return retData;
}

Data qPeek(Queue *pq)
{
    if (checkque(pq))
    {
        printf("Queue Memory Hatasi![peek]");
        exit(-1);
    }
    return pq->front->data;
}
// ready_q, running_q ve waiting_q default degerleri atanir
void config()
{
    queueInit(&ready_q);
    queueInit(&running_q);
    for (int i = 0; i < TOTAL_PID; i++)
    {
        waiting_q[i] = -1;
    }
    printf("- Queue yaratma isi tamamlandi.\n\n");
}

// first come first served
void FCFS()
{
    printf("[FCFS Scheduling]\n");
    int i, pick, time;
    int idle_time = 0, check = 0;
    initalize();

    for (time = 1; check < TOTAL_PID; time++)
    { //Tum pcb_table ogeleri bittiginde sonlanir
        for (i = 0; i < TOTAL_PID; i++)
        {
            // process in arrivial time i geldiginde calismaya baslar
            // her bir process CPU BURST i time kadar calisir
            if (time == pcb_table[i]->arr_time)
            {
                enqueueProc(&ready_q, pcb_table[i]->pid);
            }
            if (waiting_q[i] > 0)
            {
                waiting_q[i]--;
            }
            if (waiting_q[i] == 0)
            {
                enqueueProc(&ready_q, i);
                waiting_q[i]--;
            }
        }
        if (!checkque(&ready_q) && checkque(&running_q))
        {
            pick = dequeueProc(&ready_q);
            enqueueProc(&running_q, pick);
            pcb_table[pick]->start = TRUE;
        }

        if (checkque(&running_q))
        {
            printf("TIME %d ~ %d\t: IDLE\n", time - 1, time);
            idle_time++;
        }
        else
        {
            if (pcb_table[pick]->start == TRUE)
            {
                pcb_table[pick]->start = FALSE;
            }

            // Run time da her bir process icin bir thread yaratilir
            if (pcb_table[pick]->progress == 0)
            {
                // kodun tum steplerini dinamik yaptim ancak
                // bir sonraki ready kuyruk elemanin bu asamada belirleyemedigim icin semaphore post edecegim elemani statik verdim
                int semNext = pcb_table[pick]->semNext;
                sem_init(&pcb_table[pick]->sem, 0, 1);
                sem_init(&pcb_table[semNext]->sem, 0, 0);

                sem_wait(&pcb_table[pick]->sem);
                pthread_create(&pcb_table[pick]->thread, NULL, (void *)&taskImpl, pick);
                pthread_join(pcb_table[pick]->thread, NULL);

                sem_post(&pcb_table[semNext]->sem);
            }
            printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);

            pcb_table[pick]->progress++;

            if (pcb_table[pick]->progress == pcb_table[pick]->cpu_burst)
            {
                check++;
                pcb_table[dequeueProc(&running_q)]->comp_time = time + 1;
            }
        }
    }

    printf("*****************************************************************************\n\n");
}

void preemtedPriority()
{
    printf("[Preemptive Priority Scheduling]\n");
    int i, pick, time, preem = 0;
    Queue temp;
    queueInit(&temp);
    int idle_time = 0;
    int check = 0;

    initalize();

    for (time = 1; check != TOTAL_PID; time++)
    {
        for (i = 0; i < TOTAL_PID; i++)
        {
            if (waiting_q[i] > 0)
            {
                waiting_q[i]--;
            }

            // process in arrivial time i geldiginde calismaya baslar
            // her bir process CPU BURST i time kadar calisir
            if (waiting_q[i] == 0 || time == pcb_table[i]->arr_time)
            {
                if (waiting_q[i] == 0)
                {
                    waiting_q[i]--;
                }
                // ilk kez queue alinir
                if (checkque(&ready_q) && checkque(&running_q))
                {
                    enqueueProc(&ready_q, pcb_table[i]->pid);
                }
                else if (!checkque(&running_q)) // suan calisan bir process var ise preemtion icin kontrol edilir
                {
                    // calisan process ile arrivial time i gelen processlerin priority degerleri kontrol edilir
                    // eger arrivial time i gelen process in priority degeri buyuk ise preemtion gerceklesir
                    if (pcb_table[i]->priority < pcb_table[qPeek(&running_q)]->priority)
                    {
                        preem = dequeueProc(&running_q);
                        while (!checkque(&ready_q))
                        {
                            enqueueProc(&temp, dequeueProc(&ready_q));
                        }
                        enqueueProc(&ready_q, preem);
                        while (!checkque(&temp))
                        {
                            enqueueProc(&ready_q, dequeueProc(&temp));
                        }
                        enqueueProc(&running_q, i);
                        if (pcb_table[preem]->preemptive == 1)
                        {
                            pcb_table[preem]->preemptive = 0;
                        }
                    }
                    else
                    {
                        if (checkque(&ready_q))
                            enqueueProc(&ready_q, i);
                        else
                        {
                            while (pcb_table[qPeek(&ready_q)]->priority < pcb_table[i]->priority)
                            {
                                enqueueProc(&temp, dequeueProc(&ready_q));
                                if (checkque(&ready_q))
                                    break;
                            }
                            enqueueProc(&temp, pcb_table[i]->pid);
                            while (!checkque(&ready_q))
                            {
                                enqueueProc(&temp, dequeueProc(&ready_q));
                            }
                            while (!checkque(&temp))
                            {
                                enqueueProc(&ready_q, dequeueProc(&temp));
                            }
                        }
                    }
                }
                else
                {
                    // eger calisan process yok ise, ready queue da siralama priority gore duzenlenir
                    while (pcb_table[qPeek(&ready_q)]->priority < pcb_table[i]->priority)
                    {
                        enqueueProc(&temp, dequeueProc(&ready_q));
                        if (checkque(&ready_q))
                            break;
                    }
                    enqueueProc(&temp, pcb_table[i]->pid);
                    while (!checkque(&ready_q))
                    {
                        enqueueProc(&temp, dequeueProc(&ready_q));
                    }
                    while (!checkque(&temp))
                    {
                        enqueueProc(&ready_q, dequeueProc(&temp));
                    }
                }
            }
        }
        if (!checkque(&running_q))
        {
            pick = qPeek(&running_q);
            pcb_table[pick]->start = TRUE;
        }

        if (!checkque(&ready_q) && checkque(&running_q))
        {
            pick = dequeueProc(&ready_q);
            enqueueProc(&running_q, pick);
            pcb_table[pick]->start = TRUE;
            pcb_table[pick]->preemptive = 1;
        }
        // eger time aninda hic bir process in arrivial time i gelmedi ise CPU bosta kalir
        if (checkque(&running_q))
        {
            printf("TIME %d ~ %d\t: IDLE\n", time - 1, time);
            idle_time++;
        }
        else
        {
            // arrivial time ve priority degeri ile artik process calismaya baslar
            if (pcb_table[pick]->start == TRUE)
            {
                pcb_table[pick]->start = FALSE;
            }

            // Run time da her bir process icin bir thread yaratilir
            if (pcb_table[pick]->progress == 0)
            {
                // kodun tum steplerini dinamik yaptim ancak
                // bir sonraki ready kuyruk elemanin bu asamada belirleyemedigim icin semaphore post edecegim elemani statik verdim
                int semNext = pcb_table[pick]->semNext;
                sem_init(&pcb_table[pick]->sem, 0, 1);
                sem_init(&pcb_table[semNext]->sem, 0, 0);

                sem_wait(&pcb_table[pick]->sem);
                pthread_create(&pcb_table[pick]->thread, NULL, (void *)&taskImpl, pick);
                pthread_join(pcb_table[pick]->thread, NULL);

                sem_post(&pcb_table[semNext]->sem);
            }
            printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
            pcb_table[pick]->progress++;

            if (pcb_table[pick]->progress == pcb_table[pick]->cpu_burst)
            {
                check++;
                pcb_table[dequeueProc(&running_q)]->comp_time = time + 1;
            }
        }
    }

    printf("*****************************************************************************\n\n");
}
// her bir task icin ayri ayri func yaratmak yerine tek bir func icinde tum tasklar implement edilir
void taskImpl(int pick)
{
    char taskId = pcb_table[pick]->taskId;
    printf("Thread(Task)%c: basladi...\n", taskId);

    printf("Thread(Task)%c: Counter Degeri: %d\n", taskId, counter);
    counter++;
    printf("Thread(Task)%c: Counter yeni Degeri: %d\n", taskId, counter);

    pthread_exit(0);
}

// pcb_table icin initialize degerler set edilir
void initalize()
{
    for (int i = 0; i < TOTAL_PID; i++)
    {
        waiting_q[i] = -1;
        pcb_table[i]->progress = 0;
        pcb_table[i]->comp_time = 0;
    }
}

void createTasks()
{
    for (int i = 0; i < TOTAL_PID; i++)
    {
        pcb_table[i] = (struct PID *)malloc(sizeof(struct pcb));
        if (i == 0)
        {
            pcb_table[newest_PID]->priority = 3;
            pcb_table[newest_PID]->taskId = 'A';
            pcb_table[newest_PID]->semNext = 3;
        }
        else if (i == 1)
        {
            pcb_table[newest_PID]->priority = 2;
            pcb_table[newest_PID]->taskId = 'B';
            pcb_table[newest_PID]->semNext = 2;
        }
        else if (i == 2)
        {
            pcb_table[newest_PID]->priority = 1;
            pcb_table[newest_PID]->taskId = 'C';
            pcb_table[newest_PID]->semNext = 0;
        }
        else if (i == 3)
        {
            pcb_table[newest_PID]->priority = 0;
            pcb_table[newest_PID]->taskId = 'D';
            pcb_table[newest_PID]->semNext = 1;
        }

        pcb_table[newest_PID]->pid = newest_PID;
        pcb_table[newest_PID]->start = FALSE;
        pcb_table[newest_PID]->progress = 0;
        pcb_table[newest_PID]->preemptive = 0;
        pcb_table[newest_PID]->cpu_burst = (int)(rand() % 8 + 2);
        pcb_table[newest_PID]->io_burst = (int)(rand() % 4 + 1);
        pcb_table[newest_PID]->arr_time = (int)(rand() % (4 * TOTAL_PID) + 1);

        newest_PID++;
    }
    pcb_table[(int)(rand() % TOTAL_PID)]->arr_time = 1;

    printf("----------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("pcb_table\ttask id\t\tpriorty\t\tstart\t\tarr time\tcpu burst\tio burst\n");
    printf("----------------------------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < TOTAL_PID; i++)
    {
        printf("P%d\t\tTask%c\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
               pcb_table[i]->pid, pcb_table[i]->taskId, pcb_table[i]->priority,pcb_table[i]->start,
               pcb_table[i]->arr_time, pcb_table[i]->cpu_burst, pcb_table[i]->io_burst);
    }
    printf("-----------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("- pcb_table yaratma isi tamamlandi.\n\n");
}

int main()
{
    createTasks();
    config();

    // kosulmak istenilen scheduling algoritmasi buradan belirlenebilir
    FCFS();
    preemtedPriority();

    return 0;
}
