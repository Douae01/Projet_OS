#include <stdlib.h>
#include "kernel.h"
#include "list.h"

#ifndef NULL
#define NULL 0
#endif


/*****************************************************************************
 * Global variables
 *****************************************************************************/

static uint32_t id=1;
Task * tsk_running = NULL;	/* pointer to ready task list : the first
                                     node is the running task descriptor */
Task * tsk_prev = NULL;
Task * tsk_sleeping = NULL;	/* pointer to sleeping task list */

/*****************************************************************************
 * SVC dispatch
 *****************************************************************************/
/* sys_add
 *   test function
 */
int sys_add(int a, int b)
{
    return a+b;
}

/* syscall_dispatch
 *   dispatch syscalls
 *   n      : syscall number
 *   args[] : array of the parameters (4 max)
 */
int32_t svc_dispatch(uint32_t n, uint32_t args[])
{
	int32_t result;
    switch(n) {
    	case 0:
    		result = sys_add((int)args[0], (int)args[1]);
    		break;
    	case 1:
    		result = (int32_t *)malloc((size_t)args[0]);
    		break;
    	case 2:
    		free((void*)args[0]);
    		result = 0;
    		break;
    	case 3:
    		result = sys_os_start();
    		break;
    	case 4:
    		result = sys_task_new((TaskCode)args[0],(uint32_t)args[1]);
    		break;
    	case 5:
    		result = sys_task_id();
    		break;
    	case 6:
    		result = sys_task_wait((uint32_t)args[0]);
    		break;
    	case 7:
    		result = sys_task_kill();
    		break;
    	case 8:
    		result = sys_sem_new((int32_t)args[0]);
    		break;
    	case 9:
    		result = sys_sem_p((Semaphore*)args[0]);
    		break;
    	case 10:
    		result = sys_sem_v((Semaphore*)args[0]);
    		break;
    	default:
    		result = -1;
    		break;
    }
    return result;
}

void sys_switch_ctx()
{
	SCB->ICSR |= 1<<28; // set PendSV to pending
}
/*****************************************************************************
 * Round robin algorithm
 *****************************************************************************/
#define SYS_TICK 10	// system tick in ms

uint32_t sys_tick_cnt=0;

/* tick_cb
 *   system tick callback: task switching, ...
 */
void sys_tick_cb()
{
    tsk_running->status = TASK_SLEEPING;

    // Passer à la tâche suivante
    tsk_prev = tsk_running;
    tsk_running = tsk_prev->next;

    tsk_running->status = TASK_RUNNING;
    sys_switch_ctx();

	int size = list_size(tsk_sleeping);
	for(int i = 0; i < size; ++i)
	{
		tsk_sleeping->delay -= SYS_TICK;
		if (tsk_sleeping->delay <= 0)
		{
			tsk_sleeping->delay = 0;

			Task* tskDelay;
			tsk_sleeping = list_remove_head(tsk_sleeping,&tskDelay);
			tskDelay->status = TASK_READY;
			tsk_running = list_insert_tail(tsk_running,tskDelay);
		}
		else
			tsk_sleeping = tsk_sleeping->next;

		sys_switch_ctx();
	}
    //list_display(tsk_running);  // Afficher seulement si la tâche passe en exécution
}

void SysTick_Handler(void)
{
	sys_tick_cnt++;

	if (sys_tick_cnt == SYS_TICK) {
		sys_tick_cnt = 0;
		sys_tick_cb();
	}
}

/*****************************************************************************
 * General OS handling functions
 *****************************************************************************/

/* sys_os_start
 *   start the first created task
 */
int32_t sys_os_start()
{
   tsk_running->status = TASK_RUNNING;
   sys_switch_ctx();

    // Reset BASEPRI
    __set_BASEPRI(0);

	// Set systick reload value to generate 1ms interrupt
    SysTick_Config(SystemCoreClock / 1000U);

    return 0;
}

/*****************************************************************************
 * Task handling functions
 *****************************************************************************/
void task_kill();

/* sys_task_new
 *   create a new task :
 *   func      : task code to be run
 *   stacksize : task stack size
 *
 *   Stack frame:
 *      |    xPSR    |
 *      |     PC     |
 *      |     LR     |
 *      |     R12    |    ^
 *      |     R3     |    ^
 *      |     R2     |    | @++
 *      |     R1     |
 *      |     R0     |
 *      +------------+
 *      |     R11    |
 *      |     R10    |
 *      |     R9     |
 *      |     R8     |
 *      |     R7     |
 *      |     R6     |
 *      |     R5     |
 *      |     R4     |
 *      +------------+
 *      | EXC_RETURN |
 *      |   CONTROL  | <- sp
 *      +------------+
 */
int32_t sys_task_new(TaskCode func, uint32_t stacksize)
{
	// get a stack with size multiple of 8 bytes
	uint32_t size = stacksize>96 ? 8*(((stacksize-1)/8)+1) : 96;
	
    // Allouer le descripteur de tâche et la pile
    Task *new_task = (Task *)malloc(sizeof(Task) + size);

    if (!new_task) {
        return -1; // allocation échouée
    }

    // Initialiser le champ `id` unique à partir de la variable globale `id`
    new_task->id = id++;
    new_task->status = TASK_READY; // tâche dans l'état READY

    // Configuration de la pile=
    new_task->splim = (uint32_t*)(new_task+1);; // Limite inférieure de la pile (début de la pile)
    new_task->sp = new_task->splim+(size/4); // Limite supérieure de la pile (sommet de la pile)

    // Réserver l'espace pour le contexte initial (18 mots pour ARM Cortex-M)
    new_task->sp -= 18;

    // Initialiser la pile pour le contexte d'exécution de la tâche

    new_task->sp[0] = 0x1;                     // Registre CONTROL : mode non-privilégié
    new_task->sp[1] = 0xFFFFFFFD;              // Valeur de EXC_RETURN (mode Thread avec PSP)
    new_task->sp[15] = (uint32_t)task_kill;    // LR (Link Register) : adresse de retour, pointant vers task_kill pour nettoyage
    new_task->sp[16] = (uint32_t)func;         // PC (Program Counter) : point d'entrée de la fonction de la tâche
    new_task->sp[17] = 1 << 24;                // xPSR : mode Thumb (bit 24)

    // Insérer la nouvelle tâche dans la liste circulaire des tâches prêtes
    tsk_running = list_insert_tail(tsk_running, new_task);
    if (tsk_running == NULL) // Vérifier si l'insertion a échoué
    {
        if (new_task != NULL) // Libérer la mémoire allouée si l'insertion a échoué
            free(new_task);
        return -1;
    }

    // Retourner l'identifiant de la tâche
    return new_task->id;
}


/* sys_task_kill
 *   kill oneself
 */
int32_t sys_task_kill()
{
    Task *tskToKill;

    tsk_running = list_remove_head(tsk_running, &tskToKill);

    // Check if there was a task to kill
    if (tskToKill == NULL)
        return -1;
    free(tskToKill);

    // Check if the task list is now empty
    if (tsk_running != NULL)
        tsk_running->status = TASK_RUNNING;

    // Switch context
    sys_switch_ctx();

	return 0;
}

/* sys_task_id
 *   returns id of task
 */
int32_t sys_task_id()
{
	if(tsk_running != NULL)
		return tsk_running->id;
    return -1;
}


/* sys_task_yield
 *   run scheduler to switch to another task
 */
int32_t sys_task_yield()
{

    return -1;
}

/* task_wait
 *   suspend the current task until timeout
 */
int32_t sys_task_wait(uint32_t ms)
{
	tsk_running = list_remove_head(tsk_running, &tsk_prev);
	tsk_sleeping = list_insert_tail(tsk_sleeping, tsk_prev);
	tsk_prev->delay = ms;
	tsk_prev->status = TASK_WAITING;
	tsk_running->status = TASK_RUNNING;
	sys_switch_ctx();

    return -1;
}


/*****************************************************************************
 * Semaphore handling functions
 *****************************************************************************/

/* sys_sem_new
 *   create a semaphore
 *   init    : initial value
 */
Semaphore * sys_sem_new(int32_t init)
{
    Semaphore *sem_new = (Semaphore *)malloc(sizeof(Semaphore));
    if (sem_new == NULL) {
        return -1;
    }

    sem_new->count = init;
    sem_new->waiting = NULL;

    return sem_new;
}

/* sys_sem_p
 *   take a token
 */
int32_t sys_sem_p(Semaphore * sem)
{
	--sem->count;

    if (sem->count < 0) {
        Task *currentTask;

		tsk_running = list_remove_head(tsk_running, &currentTask);

		sem->waiting = list_insert_tail(sem->waiting, currentTask);
		sem->waiting->status = TASK_WAITING;
		tsk_running->status = TASK_RUNNING;

		tsk_prev = currentTask;
		sys_switch_ctx();
    }

	return sem->count;
}

/* sys_sem_v
 *   release a token
 */
int32_t sys_sem_v(Semaphore * sem)
{
	++sem->count;

    if (sem->waiting != NULL) {
        Task *taskToUnblock;
        sem->waiting=list_remove_head(sem->waiting,&taskToUnblock);

        tsk_prev = tsk_running;

        tsk_running = list_insert_head(tsk_running, taskToUnblock);
        taskToUnblock->status = TASK_RUNNING;
        tsk_prev->status = TASK_READY;

        sys_switch_ctx();
    }

	return sem->count;
}
