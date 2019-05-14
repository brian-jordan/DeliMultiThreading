#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

using namespace std;


int maxOrdersOnBoard;
int numOrdersOnBoard;
int prevMadeSandwich;
int numCashiers;
// unsigned int* cashierIDs_ptr;
unsigned int deliLock; //main lock
// unsigned int cashierID; //cashier condition variable
unsigned int makerCV; //maker condition variable
int killThread;


typedef struct boardListNode {
	unsigned int cashierNumber; //unsigned int for lock
	int sandwichNumber;
	struct boardListNode* next;
}boardListNode_t;		//for naming purposes

typedef struct cashierInfo {
	FILE* file_ptr;
	unsigned int id;
}cashierInfo_t;

static boardListNode_t* startBoardList = NULL;
static boardListNode_t* endBoardList = NULL;



void threadMaker(void* arg){
// Creates maker thread
		if (thread_create((thread_startfunc_t) maker, ((void*) 0))){
			std::cout << "thread_create maker failed\n";
		}

// Initializes cashier threads with cashier info struct
	for (int i = 2; i < numCashiers + 2; i++){
		
		FILE* specificCashierFile_ptr = fopen(((char*) arg)+i, "r");											// File reader corresponding to cashier
		unsigned int cashierID = i - 1;
	//	unsigned int* specificCashierID_ptr = cashierIDs_ptr + cashierID;										
	//	specificCashierID_ptr* = cashierID;

		// Stores file reader and cashier number into a struct to pass as one argument
		CashierInfo* specificCashierInfo_ptr = ((struct CashierInfo*)(malloc(sizeof(struct CashierInfo))));
		specificCashierInfo_ptr->file_ptr = specificCashierFile_ptr;
		specificCashierInfo_ptr->id = cashierID;

		// Creates cashier thread
		if (thread_create((thread_startfunc_t) cashier, ((void*) specificCashierInfo_ptr))){
			std::cout << "thread_create failed\n";
		}
	}

}

int main(int argc, char* argv[]){
	//******BRIAN TODO: ~~~~~INITILIZE HERE AND CREATE THREADS~~~~
			//*********
				///*********

	// Initializes global variables
	printf("hello");
	maxOrdersOnBoard = atoi(argv[1]);
	numOrdersOnBoard = 0;
	numCashiers = argc - 2;
	prevMadeSandwich = -1;
	deliLock = 0;
	printf("hello");
	makerCV = argc;	
	printf("hello2");			// Number we know is greater than number of cashiers

	// Initializes array of cashierID condition variables
	// cashierIDs_ptr = ((unsigned int*)(malloc(sizeof(unsigned int) * (numCashiers + 1))));

	// Initializes first and last nodes of the board linked list
	startBoardList = ((boardListNode_t*)(malloc(sizeof(boardListNode_t*))));
	startBoardList->cashierNumber = 0;
	startBoardList->sandwichNumber = -1;
	endBoardList = ((boardListNode_t*)(malloc(sizeof(boardListNode_t*))));
	startBoardList->next = endBoardList;
	endBoardList->cashierNumber = 0;
	endBoardList->sandwichNumber = 1001;
	endBoardList->next = NULL;

	printf("bye");

	// Initiates maker thread with no argument
	if (thread_libinit((thread_startfunc_t) threadMaker, (void*) argv)){
		std::cout << "thread_libinit failed \n";
		exit(1);
	}

	
	//******BRIAN TODO: ~~~~~DEMALLOC HERE THE WHOLE BOARD LIST~~~~
			//*********
				///*********
	// Frees Array of Cashier Control Variables
	// for (int i = 0; i < argc; i++){
	// 	unsigned int* freeCashierID = cashierIDs_ptr;
	// 	free(freeCashierID);
	// 	cashierIDs_ptr + 1;
	// }

	// Frees original two nodes of board list
	// free(startBoardList);
	// free(endBoardList);

}

