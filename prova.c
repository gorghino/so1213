
#include "listx.h"
#include "stdlib.h"
#include "stdio.h"

// Elemento della lista
typedef struct list_item {
	int item;
	struct list_head list;
} list_item_t;



int main() {

	list_item_t* elem1;
	list_item_t* elem2;
	
	// Allocazione di memoria per 2 elementi di tipo list_item_t 
	elem1=(list_item_t*) malloc(sizeof(list_item_t));
	elem2=(list_item_t*) malloc(sizeof(list_item_t));
	
	elem1->item=3;
	elem2->item=5;

	// Elemento sentinella
	struct list_head head;
	
	//Inizializzazione dell'elemento sentinella
	INIT_LIST_HEAD(&head);
	
	// Test di lista vuota
	printf("Lista vuota? %d \n",list_empty(&head));	

	// Aggiunta di due elementi alla lista
	list_add(&(elem1->list),&head);	
	list_add(&(elem2->list),&head);	
	
	// Test di lista vuota
	printf("Lista vuota? %d \n",list_empty(&head));	

	// Scorrimento di Lista: Metodo 1
	list_item_t* pos;
	list_for_each_entry(pos,&head,list) {
		printf("Elemento corrente: %d \n",pos->item);
	}
	
	// Scorrimento di Lista: Metodo 2
	struct list_head* pos2;
	list_for_each(pos2,&head) {
		list_item_t* elem=container_of(pos2,list_item_t,list);
		printf("Elemento corrente: %d \n",elem->item);

	} 
	

	return 1;
}
