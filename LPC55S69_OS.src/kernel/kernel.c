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
	// Mettre la tâche courante en état de sommeil
    tsk_running->status = TASK_SLEEPING;

    // Passer à la tâche suivante
    tsk_prev = tsk_running; // Sauvegarde la tâche précédente
    tsk_running = tsk_prev->next; // Change la tâche courante vers la suivante dans la liste

    // Définir l'état de la nouvelle tâche courante en cours d'exécution
    tsk_running->status = TASK_RUNNING;

    // Changer le contexte d'exécution pour passer à la nouvelle tâche courante
    sys_switch_ctx();

    // Obtenir le nombre de tâches en attente dans la liste "tsk_sleeping"
	int size = list_size(tsk_sleeping);
	for(int i = 0; i < size; ++i)
	{
		// Réduire le délai de chaque tâche en sommeil par la valeur de SYS_TICK
		tsk_sleeping->delay -= SYS_TICK;
		if (tsk_sleeping->delay <= 0)
		{
			tsk_sleeping->delay = 0;

			// Retirer la tâche de la liste des tâches en sommeil
			Task* tskDelay;
			tsk_sleeping = list_remove_head(tsk_sleeping,&tskDelay);

			// Changer son état pour la rendre prête à être exécutée
			tskDelay->status = TASK_READY;

			// Ajouter cette tâche à la fin de la liste des tâches prêtes
			tsk_running = list_insert_tail(tsk_running,tskDelay);
		}
		else{
			// Si le délai n'est pas écoulé, passer à la tâche suivante en sommeil
			tsk_sleeping = tsk_sleeping->next;
		}
		// Changer le contexte pour exécuter la tâche courante
		sys_switch_ctx();
	}
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
   // Définir l'état de la tâche courante en cours d'exécution
   tsk_running->status = TASK_RUNNING;

   // Changer le contexte pour passer à la tâche courante
   sys_switch_ctx();

   // Réinitialiser la priorité de base des interruptions
    __set_BASEPRI(0);

    // Configurer le SysTick pour générer une interruption toutes les 1 ms
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
	
	// Allouer le descripteur de tâche et la pile avec la taille calculée
    Task *new_task = (Task *)malloc(sizeof(Task) + size);

    // Vérifier si l'allocation a réussi
    if (!new_task) {
        return -1; // allocation échouée
    }

    // Initialiser un identifiant unique pour la nouvelle tâche en incrémentant `id`
    new_task->id = id++;
    new_task->status = TASK_READY; // tâche dans l'état READY

    // Configuration des limites de la pile de la tâche
    new_task->splim = (uint32_t*)(new_task+1);; // Limite inférieure de la pile (début de la pile)
    new_task->sp = new_task->splim+(size/4); // Limite supérieure de la pile (sommet de la pile pointe vers le haut)

    // Réserver l'espace pour le contexte initial (18 mots pour ARM Cortex-M)
    new_task->sp -= 18;

    // Initialiser la pile pour le contexte d'exécution de la tâche

    new_task->sp[0] = 0x1;                     // Registre CONTROL : mode non-privilégié
    new_task->sp[1] = 0xFFFFFFFD;              // Valeur de EXC_RETURN (mode Thread avec PSP)
    new_task->sp[15] = (uint32_t)task_kill;    // LR (Link Register) : pointeur vers `task_kill` pour nettoyage à la fin
    new_task->sp[16] = (uint32_t)func;         // PC (Program Counter) : point d'entrée de la fonction de la tâche
    new_task->sp[17] = 1 << 24;                // xPSR : mode Thumb (Bit 24 à 1)

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

    // Retirer la tâche courante de la liste des tâches prêtes et la sauvegarder dans `tskToKill`
    tsk_running = list_remove_head(tsk_running, &tskToKill);

    // Vérifier s'il y avait une tâche à supprimer
    if (tskToKill == NULL)
        return -1; // Aucun élément à supprimer

    // Libérer la mémoire de la tâche supprimée
    free(tskToKill);

    // Vérifier si la liste des tâches prêtes est maintenant vide
    if (tsk_running != NULL)
        tsk_running->status = TASK_RUNNING;

    // Changer le contexte pour passer à la tâche courante
    sys_switch_ctx();

	return 0;
}

/* sys_task_id
 *   returns id of task
 */
int32_t sys_task_id()
{
	// Vérifier si une tâche est actuellement en cours d'exécution
	if(tsk_running != NULL)
		return tsk_running->id; // Retourner l'ID de la tâche courante
    return -1; // Aucun ID disponible
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
	// Retirer la tâche courante de la liste des tâches prêtes et la sauvegarder dans `tsk_prev`
	tsk_running = list_remove_head(tsk_running, &tsk_prev);

	// Ajouter la tâche retirée à la liste des tâches en attente
	tsk_sleeping = list_insert_tail(tsk_sleeping, tsk_prev);

	// Définir le délai de la tâche en attente en millisecondes
	tsk_prev->delay = ms;

	tsk_prev->status = TASK_WAITING;
	tsk_running->status = TASK_RUNNING;

	// Changer le contexte pour passer à la tâche courante
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
	// Allouer un nouvel objet sémaphore
    Semaphore *sem_new = (Semaphore *)malloc(sizeof(Semaphore));

    // Vérifier si l'allocation mémoire a réussi
    if (sem_new == NULL) {
        return -1; //L'allocation a échoué
    }

    // Initialiser le compteur de sémaphore avec la valeur `init`
    sem_new->count = init;
    // Initialiser la liste des tâches en attente sur le sémaphore
    sem_new->waiting = NULL;

    // Retourner le sémaphore nouvellement créé
    return sem_new;
}

/* sys_sem_p
 *   take a token
 */
int32_t sys_sem_p(Semaphore * sem)
{
	 // Décrémenter le compteur de sémaphore pour prendre un jeton
	--sem->count;

	// Si le compteur est négatif, il n'y a plus de jeton disponible
    if (sem->count < 0) {
        Task *currentTask;

        // Retirer la tâche courante de la liste des tâches prêtes
		tsk_running = list_remove_head(tsk_running, &currentTask);

		// Ajouter la tâche courante à la liste des tâches en attente sur le sémaphore
		sem->waiting = list_insert_tail(sem->waiting, currentTask);

		// Changer l'état de la tâche en `TASK_WAITING` et de la prochaine tâche en `TASK_RUNNING`
		sem->waiting->status = TASK_WAITING;
		tsk_running->status = TASK_RUNNING;

		// Sauvegarder la tâche courante dans `tsk_prev` pour gestion de contexte
		tsk_prev = currentTask;

		// Changer le contexte pour passer à la tâche courante
		sys_switch_ctx();
    }

	return sem->count;
}

/* sys_sem_v
 *   release a token
 */
int32_t sys_sem_v(Semaphore * sem)
{
	// Incrémenter le compteur de sémaphore pour libérer un jeton
	++sem->count;

	// Si des tâches sont en attente sur le sémaphore
    if (sem->waiting != NULL) {
        Task *taskToUnblock;

        // Retirer la première tâche en attente de la liste du sémaphore
        sem->waiting=list_remove_head(sem->waiting,&taskToUnblock);

        // Sauvegarder la tâche courante dans `tsk_prev
        tsk_prev = tsk_running;

        // Insérer la tâche débloquée en tête de la liste des tâches prêtes
        tsk_running = list_insert_head(tsk_running, taskToUnblock);

        // Mettre la tâche débloquée en état `TASK_RUNNING` et l'ancienne tâche en état `TASK_READY`
        taskToUnblock->status = TASK_RUNNING;
        tsk_prev->status = TASK_READY;

        // Changer le contexte pour passer à la tâche courante
        sys_switch_ctx();
    }

	return sem->count;
}

