#include "asl.h"
#include "pcb.h"

/* Global Variables*/
HIDDEN semd_t semd_table[MAXPROC]; 
HIDDEN LIST_HEAD(semdFree_h);
HIDDEN LIST_HEAD(semd_h);

/* funzione di inizializzazione.
   si occupa di inizializzare la semd_table e le liste di semafori liberi o attivi
*/

void initASL()
{
	for (int index = 0; index < MAXPROC+1; index++)
	{
		semd_t semaphore;
		INIT_LIST_HEAD(&semaphore.s_next);
		semaphore.s_key = NULL;    
		INIT_LIST_HEAD(&semaphore.s_procQ);
		semd_table[index] = semaphore;
	}
	INIT_LIST_HEAD(&semdFree_h);
	for (int index = 0; index < MAXPROC; index++)
	{
		list_add_tail(&(semd_table[index].s_next), &semdFree_h);
	}
	INIT_LIST_HEAD(&semd_h);	
}

/* restituisce il puntatore a semd presente nella ASL la cui chiave è key,
   NULL se non è presente
*/

semd_t* getSemd(int *key)
{
	semd_t* semd = NULL;
	semd_t* pos = NULL;
	list_for_each_entry(pos,&semd_h,s_next)
		if (pos->s_key == key)
			semd = pos;
	return semd;		
}

/* aggiunge un PCB alla lista dei processi bloccati sul semaforo
	key: chiave del semaforo
	p: puntatore al PBC	
	return: TRUE se il SEMD richiesto non è disponibile in quanto sono terminati i semafori disponibili
*/	

int insertBlocked(int *key, pcb_t *p)
{
	semd_t *semd =  getSemd(key);
	if (semd == NULL)
	{
		if (list_empty(&semdFree_h))
			return TRUE;		
		else
		{
			//se il semaforo non è già stato utilizzato ne viene utilizzato uno dalla lista dei
			//semafori liberi e spostato nella ASL
		   	semd = container_of(semdFree_h.next,semd_t,s_next); 
			list_del(semdFree_h.next);
			semd->s_key = key;			
			INIT_LIST_HEAD(&semd->s_procQ);
			list_add_tail(&semd->s_next,&semd_h);
		}	
	}
	list_add(&p->p_next,&semd->s_procQ);
	p->p_semkey = key;
	return FALSE;	
}

/* rimuove il primo processo bloccato sulla coda dei processi del semaforo relativo
	key: chiave del semaforo
	
	return: puntatore al PCB che è stato rimosso; ritorna NULL se il semaforo indicato non è presente nella ASL
*/

pcb_t* removeBlocked(int *key)
{
	semd_t* semd = getSemd(key);
	pcb_t* pcb = NULL;
	if (semd != NULL)	
	{	
		//semd->s_procQ.next è il puntatore al campo list_head del primo processo in coda sul semaforo
		//per ottenere il puntatore alla struttura uso container_of
		pcb = container_of(semd->s_procQ.next,pcb_t,p_next);
		list_del(semd->s_procQ.next);
		pcb->p_semkey = key;
		//devo occuparmi anche di riportare il semaforo nella lista di quelli liberi se non ci sono piu'
		//processi bloccati su di esso
		if (list_empty(&semd->s_procQ)) 
		{
			list_del(&semd->s_next);
			semd->s_key = NULL;
			list_add(&semd->s_next,&semdFree_h);
		}
	}
	return pcb;
}

/* rimuove il processo puntato da p dalla coda dei processi del semaforo su cui è bloccato
	p: puntatore al processo da rimuovere
	return: il puntatore al processo rimosso; NULL se il processo non è stato trovato nella coda dei processi bloccati
		sul semaforo indicato nel campo p->semkey
*/

pcb_t* outBlocked(pcb_t *p)
{
	pcb_t *pcb = NULL;
	if (p != NULL)
	{
		semd_t *semd = getSemd(p->p_semkey);
		if (semd != NULL)	
		{
			struct list_head *pos;
			list_for_each(pos, &semd->s_procQ)    //ricerco il processo nella lista dei processi bloccati sul semaforo
				if (container_of(pos, pcb_t, p_next) == p) 
				{
					pcb = p;					
					break;
				}
			if (pcb != NULL)
			{
				__list_del(p->p_next.prev,p->p_next.next);
				pcb->p_semkey = semd->s_key;
				//dopo la rimozione del processo controllo se il semaforo è ancora utilizzato
				if (list_empty(&semd->s_procQ)) 
				{
					list_del(&semd->s_next);	
					semd->s_key = NULL;
					list_add(&semd->s_next,&semdFree_h);
				}
			}
		}
	}
	return pcb;
}

/* restutuisce il puntatore al PCB in testa alla coda dei processi del semaforo identificato da key senza rimuovere il processo
	key: chiave del semaforo
	
	return: processo in testa alla coda; NULL se il semaforo non è nella ASL e quindi non ha processi in coda
*/

pcb_t* headBlocked(int *key)
{
	semd_t *semd = getSemd(key);
	pcb_t* pcb = NULL;
	if (semd != NULL && !(list_empty(&semd->s_procQ)))	
		pcb = container_of(semd->s_procQ.next,pcb_t,p_next);  //operazione già presente nella funzione removeblocked
	return pcb;
}

/* rimuove il processo puntato da p dalla coda dei processi del semaforo su cui è bloccato e fa lo stesso su tutti 
   i processi dell'albero radicato in p
	p: puntatore al processo da rimuovere
*/

void outChildBlocked(pcb_t *p)
{
	if (p != NULL)
	{
		pcb_t *pcb = outBlocked(p);
		pcb_t *pcb_son = NULL;
		do {
			pcb_son = removeChild(pcb);
			outChildBlocked(pcb_son);
		} while (pcb_son != NULL);
	}
}