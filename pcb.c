/*  Copyright (C) 2013 Aguiari Davide, Giacò Jacopo, Trotta Marco
 *  Authors: Aguiari Davide davide.aguiari@studio.unibo.it
 *			 Giacò Jacopo jacopo.giaco@studio.unibo.it
 					Trotta Marco marco.trotta2@studio.unibo.it
 							
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "utils.h"
#include "pcb.e"
#include "libumps.h"
#include "p1test.h"


pcb_t pcb_table[MAXPROC]; /*Tabella dei PCB*/
pcb_t *pcbfree_h = NULL; /*Puntatore alla testa della lista dei PCB liberi*/


/************ Funzioni per gestire le liste di PCB ************/


/*Inizializza la pcbFree in modo da contenere tutti gli elementi della pcb_table.*/
void initPcbs(void){
	initPcbs_rec(0);
}
	
void initPcbs_rec(int count){
	if (count>=MAXPROC) /*0...MAXPROC-1*/
		return;

	insertPCBList(&pcbfree_h, &pcb_table[count]);
	count++;
	initPcbs_rec(count);
}

/*Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree)*/	
void freePcb(pcb_t *p){
	insertPCBList(&pcbfree_h, p);
	return;
}


/*Restituisce NULL se la pcbFree e’ vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i 
campi (NULL/0) e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void){
	if(emptyPCBList(&pcbfree_h)){
		return NULL;
	}
	else{
		pcb_t *ptemp = pcbfree_h;
		pcbfree_h = pcbfree_h->p_next;

		/*Annullo i puntatori*/
		ptemp->p_next = NULL;
		ptemp->p_parent = NULL;
		ptemp->p_first_child = NULL;
		ptemp->p_sib = NULL;
		ptemp->p_semkey = NULL; /*Il processo non è ancora fermo su un semaforo*/

		return ptemp;	
	}
}	

/************ Funzioni per gestire le code di PCB ************/

/*Inserisce l’elemento puntato da p nella coda dei processi puntata da head. 
L’inserimento avviene tenendo conto della priorita’ di ciascun pcb.
La coda dei processi è ordinata in base alla priorita’ dei PCB, in ordine decrescente 
(L’elemento di testa e’ l’elemento con la priorita’ piu’ alta).*/
void insertProcQ(pcb_t **head, pcb_t* p){
	if ((*head) == NULL){
		(*head) = p; /*la coda è vuota, p è il primo elemento*/
		}
	else if(p->priority < (*head)->priority && head != NULL ){	
			insertProcQ(&((*head)->p_next), p);
			}
	else if(p->priority > (*head)->priority && head != NULL){
		/*Inserisco il puntatore di pcb_t*/
		p->p_next = (*head);
		(*head) = p;
	}
	/*Casi uguali, scorro fino all'ultimo uguale e lo inserisco*/
	else{
		/*Lo inserisco dopo, evito Starvation*/
		pcb_t *last_equal = getLast(head, p->priority); 
		p->p_next = last_equal->p_next;
		last_equal->p_next = p;
	}
}

/*getLast ritorna l'ultimo pcb in coda con priorità priority*/
pcb_t *getLast(pcb_t **head, int priority){
	if( (*head)->p_next == NULL  || ((*head)->p_next)->priority != priority)
		return (*head);
	else
		return getLast(&((*head)->p_next), priority);

}

/*Restituisce l’elemento di testa della coda dei processi puntata da head, SENZA RIMUOVERLO. 
Ritorna NULL se la coda non ha elementi.*/
pcb_t* headProcQ(pcb_t* head){
	if (!head)
		return NULL;
	else
		return head;
}


/*Rimuove il primo elemento dalla coda dei processi puntata da head. 
Ritorna NULL se la coda e’ vuota. 
Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.*/
pcb_t* removeProcQ(pcb_t** head){
	if ((*head) == NULL)
		return NULL;
	else{
		/*Rimuovo e aggiorno i puntatori*/
		pcb_t *temp = (*head);
		(*head) = (*head)->p_next;
		temp->p_next = NULL;
		return temp;
		}
}



/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head. 
Se p non e’ presente nella coda, restituisce NULL.*/
pcb_t* outProcQ(pcb_t** head, pcb_t *p){
	if (head == NULL)
		return NULL;

	else if((*head) == p)
		return removeProcQ(head);

	else
		return outProcQ(&((*head)->p_next), p);
}



/*Richiama la funzione fun per ogni elemento della lista puntata da head*/
void forallProcQ(struct pcb_t *head, void fun(struct pcb_t *pcb, void *), void *arg){
	if (head == NULL)
		return;
	else{
		fun(head, arg);
		forallProcQ(head->p_next, fun, arg);
	}
}


/************ Funzioni per gestire gli alberi di PCB ************/

/*Inserisce il PCB puntato da p come figlio del PCB puntato da parent*/
void insertChild(pcb_t *parent, pcb_t *p){
	if(parent->p_first_child == NULL){
		/*Se parent non ha figli, p è il primo figlio e inizio della lista dei figli*/
		parent->p_first_child = p;
		p->p_parent = parent;
		p->p_sib = NULL;
		}
	else
		insertSibling(parent->p_first_child, p); /*Se parent punta già a una lista di figli, insertSibling() si preoccupa di inserire p in coda*/
}

/*Rimuove il primo figlio del PCB puntato da p. 
Se p non ha figli, restituisce NULL.*/
pcb_t* removeChild(pcb_t *p){
	if(p->p_first_child == NULL)
		return NULL;
	else{
		pcb_t *temp = p->p_first_child;
		p->p_first_child = temp->p_sib; /*Aggiorno firstchild di p con il nuovo sibling*/
		temp->p_sib = NULL;
		temp->p_parent = NULL;
		return temp;	
	}
}

/*Rimuove il PCB puntato da p dalla lista dei figli del padre. 
Se il PCB puntato da p non ha un padre, restuisce NULL. 
Altrimenti restituisce l’elemento rimosso (cioe’ p).*/
pcb_t* outChild(pcb_t* p){
	pcb_t *list_child = (p->p_parent)->p_first_child; /*Faccio puntare a list_child, la lista dei figli del parent di p*/

	if(p->p_parent == NULL)
		return NULL;

	else if(list_child == p) /*Se p è il primo figlio del suo parent, mi torna utile removeChild(), che lo elimina*/
		return removeChild(p->p_parent);
	else
		return outChild_rec(list_child, p);
}

pcb_t* outChild_rec(pcb_t *list_child, pcb_t *p){
	if(list_child->p_sib == NULL)
		return NULL; /*p non è presente nella lista*/

	else if(list_child->p_sib == p){
		/*Rimuovo p dalla lista e aggiorno i puntatori*/
		pcb_t *removed = list_child->p_sib;
		list_child->p_sib = removed->p_sib;

		removed->p_sib = NULL;
		removed->p_parent = NULL;
		return removed;
	}	
	else
		return outChild_rec(list_child->p_sib, p);
}

