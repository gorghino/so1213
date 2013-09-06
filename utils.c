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
#include "p1test.h"

#define TRUE 1
#define FALSE 0
#define	MAX_CPUS 1


extern pcb_t *current_process[MAX_CPUS];
extern pcb_t *ready_queue[MAX_CPUS];
extern int process_count[MAX_CPUS];
extern int softBlock_count[MAX_CPUS];



/*insertPCBList inserisce il pcb puntato da pcb_elem nella lista puntata da pcblist_p*/
void insertPCBList(pcb_t **pcblist_p, pcb_t *pcb_elem){
	if(*pcblist_p == NULL){ 
		/*Caso empty*/
		*pcblist_p = pcb_elem;
		pcb_elem->p_next = NULL;
		return;
	}
	else if ((*pcblist_p)->p_next == NULL) { 
		/*Inserimento*/
		(*pcblist_p)->p_next = pcb_elem;
		pcb_elem->p_next = NULL;
	}
	else insertPCBList(&((*pcblist_p)->p_next), pcb_elem);
}

/*insertSEMList inserisce il semaforo puntato da sem_elem nella lista puntata da semlist_p*/
void insertSEMList(semd_t **semlist_p, semd_t *sem_elem){
	if(*semlist_p == NULL){
		/*Caso empty*/
		*semlist_p = sem_elem;
		sem_elem->s_next = NULL;
		return;
	}
	else if ((*semlist_p)->s_next == NULL){
		/*Inserimento*/
		(*semlist_p)->s_next = sem_elem;
		sem_elem->s_next = NULL;
	}
	else insertSEMList(&(*semlist_p)->s_next, sem_elem);
}

/*Controlla se la lista di pcb_t* puntata da pcblist_p è vuota*/
int emptyPCBList(pcb_t **pcblist_p){
	if(*pcblist_p == NULL)
		return TRUE;
	else
		return FALSE;
}

/*Controlla se la lista di semd_t* puntata da semlist_p è vuota*/
int emptySEMList(semd_t **semlist_p){
	if(*semlist_p == NULL)
		return TRUE;
	else
		return FALSE;
}

/*insertSibling aggiunge il puntatore al pcb P appena trova un pcb figlio senza fratello nella lista puntata da firstchild*/
void insertSibling(pcb_t *firstchild, pcb_t *p){
	if (firstchild->p_sib == NULL){
		firstchild->p_sib = p;

		p->p_parent = firstchild->p_parent; /*Ogni figlio punta al padre*/
		p->p_sib = NULL;
		}
	else insertSibling(firstchild->p_sib, p);
}

void P(int *key, pcb_t *process){
	semd_t *semd;
	if((semd = getSemd(key))!=NULL){
		*(semd->s_key)--;
		if(semd->s_key < 0){
			insertBlocked(key, process);
			softBlock_count[getPRID()]++;
		}
	}
}

void V(int *key, pcb_t *process){
	semd_t *semd;
	if((semd = getSemd(key))!=NULL){
		*(semd->s_key)++;
		if(semd->s_key >= 0){
			removeBlocked(key);
			softBlock_count[getPRID()]--;
		}
	}

}






//UTILS

/**
 * strreverse(puntatore a char, puntatore a char): rovescia una stringa
 * \param begin inizio stringa
 * \param end fine stringa
 */
void strreverse(char* begin, char* end) {
       
        char aux;
        while(end>begin)
                aux=*end, *end--=*begin, *begin++=aux;
       
}


/**
 * itoa(valore, puntatore della stringa, base in cui convertire) : converte un intero in una stringa nella base specificata
 * \param value intero da convertire
 * \param str indirizzo dove scrivere la stringa
 * \param base base in cui convertire l'intero
 */
void itoa(int value, char* str, int base) {
       
        static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        char* wstr=str;
        int sign;
       
        // Validate base
        if (base<2 || base>35){ *wstr='\0'; return; }
       
        // Take care of sign
        if ((sign=value) < 0) value = -value;
 
        // Conversion. Number is reversed.
        do *wstr++ = num[value%base]; while(value/=base);
        if(sign<0) *wstr++='-';
        *wstr='\0';
       
        // Reverse string
        strreverse(str,wstr-1);
} 


void finddevicenumber(bitmap, device_n) {
  device_n = 0;
  
  while (bitmap > 1) {
    device_n++;
    bitmap >>= 1;
  }
}
