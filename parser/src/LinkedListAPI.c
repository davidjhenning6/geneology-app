 /************************
 * @file LinkedListAPI.c
 * @author Dave Henning
 * @date January 2018
 * Some code taken from prof Dennis Nikitenko example code
 *
************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include"LinkedListAPI.h"

List initializeList(char* (*printFunction)(void* toBePrinted),void (*deleteFunction)(void* toBeDeleted),int (*compareFunction)(const void* first,const void* second)){
	List list;
	list.head = NULL;
	list.tail = NULL;
	list.length = 0;
	list.deleteData = deleteFunction;
	list.compare = compareFunction;
	list.printData = printFunction;
	return list;
}


/**Function for creating a node for the linked list. 
* This node contains abstracted (void *) data as well as previous and next
* pointers to connect to other nodes in the list
*@pre data should be of same size of void pointer on the users machine to avoid size conflicts. data must be valid.
*data must be cast to void pointer before being added.
*@post data is valid to be added to a linked list
*@return On success returns a node that can be added to a linked list. On failure, returns NULL.
*@param data - is a void * pointer to any data type.  Data must be allocated on the heap.
**/
Node* initializeNode(void* data){
	Node *temp = (Node*)malloc(sizeof(Node));
	temp->previous = NULL;
	temp->next = NULL;
	temp->data = data; 
	return temp;
}


/**Inserts a Node at the front of a linked list.  List metadata is updated
* so that head and tail pointers are correct.
*@pre 'List' type must exist and be used in order to keep track of the linked list.
*@param list pointer to the dummy head of the list
*@param toBeAdded a pointer to data that is to be added to the linked list
**/
void insertFront(List* list, void* toBeAdded){
	if (list == NULL || toBeAdded == NULL){
		return;
	}
	Node *node = initializeNode(toBeAdded);
	if(list->head == NULL){
		node->next = NULL;
		list->tail = node;
	}
	else{
		node->next = list->head;
		list->head->previous = node;
	}
	list->length = list->length + 1;
	list->head = node;
	node->previous = NULL;
	return;
}


/**Inserts a Node at the back of a linked list. 
*List metadata is updated so that head and tail pointers are correct.
*@pre 'List' type must exist and be used in order to keep track of the linked list.
*@param list pointer to the dummy head of the list
*@param toBeAdded a pointer to data that is to be added to the linked list
**/
void insertBack(List* list, void* toBeAdded){
	if (list == NULL || toBeAdded == NULL){
		return;
	}
	Node *node = initializeNode(toBeAdded);
	if(list->head == NULL){
		list->head = node;
		list->tail = node;
		node->next = NULL;
		node->previous = NULL;
	}
	else{
		list->tail->next = node;
		node->previous = list->tail;
		node->next = NULL;
		list->tail = node;
	}
	list->length = list->length + 1;
	return;
}


/** Clears the contents linked list, freeing all memory asspociated with these contents.
* uses the supplied function pointer to release allocated memory for the data
*@pre 'List' type must exist and be used in order to keep track of the linked list.
*@param list pointer to the List-type dummy node
**/
void clearList(List* list){
	if (list == NULL){
		return;
	}
	if (list->head == NULL && list->tail == NULL){
		return;
	}
	Node* tmp;
	while (list->head != NULL){
		list->deleteData(list->head->data);
		tmp = list->head;
		list->head = list->head->next;
		free(tmp);
	}
	list->head = NULL;
	list->tail = NULL;
	list->length = 0;

}


/** Uses the comparison function pointer to place the element in the 
* appropriate position in the list.
* should be used as the only insert function if a sorted list is required.  
*@pre List exists and has memory allocated to it. Node to be added is valid.
*@post The node to be added will be placed immediately before or after the first occurrence of a related node
*@param list a pointer to the dummy head of the list containing function pointers for delete and compare, as well 
as a pointer to the first and last element of the list.
*@param toBeAdded a pointer to data that is to be added to the linked list
**/
void insertSorted(List* list, void* toBeAdded){
	if (list == NULL || toBeAdded == NULL){
		return;
	}
	if (list->head == NULL){
		insertBack(list, toBeAdded);
		return;
	}
	if (list->compare(toBeAdded, list->head->data) <= 0){
		insertFront(list, toBeAdded);
		return;
	}
	if (list->compare(toBeAdded, list->tail->data) > 0){
		insertBack(list, toBeAdded);
		return;
	}
	Node* currNode = list->head;
	while (currNode != NULL){
		if (list->compare(toBeAdded, currNode->data) <= 0){
		
			char* currDescr = list->printData(currNode->data); 
			char* newDescr = list->printData(toBeAdded); 
		
			//printf("Inserting %s before %s\n", newDescr, currDescr);

			free(currDescr);
			free(newDescr);
		
			Node* newNode = initializeNode(toBeAdded);
			newNode->next = currNode;
			newNode->previous = currNode->previous;
			currNode->previous->next = newNode;
			currNode->previous = newNode;
			list->length = list->length + 1;
			return;
		}
		currNode = currNode->next;
	}
	return;
}


/** Removes data from from the list, deletes the node and frees the memory,
 * changes pointer values of surrounding nodes to maintain list structure.
 * returns the data 
 * You can assume that the list contains no duplicates
 *@pre List must exist and have memory allocated to it
 *@post toBeDeleted will have its memory freed if it exists in the list.
 *@param list pointer to the dummy head of the list containing deleteFunction function pointer
 *@param toBeDeleted pointer to data that is to be removed from the list
 *@return on success: void * pointer to data  on failure: NULL
 **/
void* deleteDataFromList(List* list, void* toBeDeleted){
	if (list == NULL || toBeDeleted == NULL){
		return NULL;
	}
	Node* tmp = list->head;
	while(tmp != NULL){
		if (list->compare(toBeDeleted, tmp->data) == 0){
			//Unlink the node
			Node* delNode = tmp;
			if (tmp->previous != NULL){
				tmp->previous->next = delNode->next;
			}else{
				list->head = delNode->next;
			}
			if (tmp->next != NULL){
				tmp->next->previous = delNode->previous;
			}else{
				list->tail = delNode->previous;
			}
			void* data = delNode->data;
			free(delNode);
			list->length = list->length - 1;
			return data;
		}else{
			tmp = tmp->next;
		}
	}
	return NULL;
}


/**Returns a pointer to the data at the front of the list. Does not alter list structure.
 *@pre The list exists and has memory allocated to it
 *@param the list struct
 *@return pointer to the data located at the head of the list
 **/
void* getFromFront(List list){
	if(list.head == NULL){
		return NULL;
	}
	return list.head->data;
}


/**Returns a pointer to the data at the back of the list. Does not alter list structure.
 *@pre The list exists and has memory allocated to it
 *@param the list struct
 *@return pointer to the data located at the tail of the list
 **/
void* getFromBack(List list){
	if(list.tail == NULL){
		return NULL;
	}
	return list.tail->data;
}


/**Returns a string that contains a string representation of
the list traversed from  head to tail. Utilize the list's printData function pointer to create the string.
returned string must be freed by the calling function.
 *@pre List must exist, but does not have to have elements.
 *@param list Pointer to linked list dummy head.
 *@return on success: char * to string representation of list (must be freed after use).  on failure: NULL
 **/
char* toString(List list){
	ListIterator iter = createIterator(list);
	char* str;
		
	str = (char*)malloc(sizeof(char));
	strcpy(str, "");
	
	void* elem;
	while( (elem = nextElement(&iter)) != NULL){
		char* currDescr = list.printData(elem);
		int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc(str, newLen);
		strcat(str, "\n");
		strcat(str, currDescr);	
		free(currDescr);
	}
	return str;
}


/** Function for creating an iterator for the linked list. 
 * This node contains abstracted (void *) data as well as previous and next
 * pointers to connect to other nodes in the list
 *@pre List exists and is valid
 *@post List remains unchanged.  The iterator has been allocated and points to the head of the list.
 *@return The newly created iterator object.
 *@param list - a pointer to the list to iterate over.
**/
ListIterator createIterator(List list){
	ListIterator iterate;
	iterate.current = list.head;
	return iterate;
}


/** Function that returns the next element of the list through the iterator. 
* This function returns the data at head of the list the first time it is called after.
* the iterator was created. Every subsequent call returns the data associated with the next element.
* Returns NULL once the end of the iterator is reached.
*@pre List exists and is valid.  Iterator exists and is valid.
*@post List remains unchanged.  The iterator points to the next element on the list.
*@return The data associated with the list element that the iterator pointed to when the function was called.
*@param iter - an iterator to a list.
**/
void* nextElement(ListIterator* iter){
	if(iter->current == NULL){
		return NULL;
	}
	void *node = iter->current->data;
	iter->current = iter->current->next;
	return node;
}


/**Returns the number of elements in the list.
 *@pre List must exist, but does not have to have elements.
 *@param list - the list struct.
 *@return on success: number of eleemnts in the list (0 or more).  on failure: -1 (e.g. list not initlized correctly)
 **/
int getLength(List list){
	//if(list == NULL)){
	//	return(-1);
	//}
	return(list.length);
}


/** Function that searches for an element in the list using a comparator function.
 * If an element is found, a pointer to the data of that element is returned
 * Returns NULL if the element is not found.
 *@pre List exists and is valid.  Comparator function has been provided.
 *@post List remains unchanged.
 *@return The data associated with the list element that matches the search criteria.  If element is not found, return NULL.
 *@param list - a list sruct
 *@param customCompare - a pointer to comparator fuction for customizing the search
 *@param searchRecord - a pointer to search data, which contains seach criteria
 *Note: while the arguments of compare() and searchRecord are all void, it is assumed that records they point to are
 *      all of the same type - just like arguments to the compare() function in the List struct
 **/
void* findElement(List list, bool (*customCompare)(const void* first,const void* second), const void* searchRecord){
	if(list.head==NULL){
		return NULL;
	}
	if(searchRecord==NULL){
		return NULL;
	}
	if(customCompare==NULL){
		return NULL;
	}
	ListIterator iter = createIterator(list);
	if(iter.current==NULL){
		return NULL;
	}
	void* data = nextElement(&iter);
	do{
		if(customCompare(data, searchRecord)){
			return data;
		}
	}while((data = nextElement(&iter))!=NULL);
	return NULL;
}

