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
unsigned int deliLock; //main lock
unsigned int makerCV; //maker condition variable



struct BoardListNode {
	unsigned int cashierNumber; //unsigned int for lock
	int sandwichNumber;
	struct BoardListNode* next;
};

struct CashierInfo {
	FILE* file_ptr;
	unsigned int id;
};

BoardListNode* startBoardList;
BoardListNode* endBoardList;


int checkSandwhich(BoardListNode* start, unsigned int ID) {
    BoardListNode* temp = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
    temp=start;

	while(temp->sandwichNumber!=1001){
		if(temp->cashierNumber==ID){
			return 1;
		}
		else{
			temp=temp->next;
		}
	}
	return 0;
}




//get sandwhich take it off the board 
void take_off() {
    BoardListNode* temp = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
    BoardListNode* removeTempNode = temp;
	temp->cashierNumber = 0; //************change from NULL
	temp->sandwichNumber = 0;
	temp->next = startBoardList;

	BoardListNode* removeNode;

	while(temp->sandwichNumber!=1001){
		if((prevMadeSandwich >= temp->next->sandwichNumber) && (prevMadeSandwich <= temp->next->next->sandwichNumber)){
			if ((abs(temp->next->sandwichNumber - prevMadeSandwich) <= abs(temp->next->next->sandwichNumber - prevMadeSandwich)) || (temp->next->next->sandwichNumber == 1001 && temp->next->sandwichNumber != -1)){
				removeNode = temp->next;
				temp->next = temp->next->next;
				break;
			}
			else{
				removeNode = temp->next->next;
				temp->next->next = temp->next->next->next;
				break;
			}
		}
		temp = temp->next;
	}

	unsigned int madeCashierID = removeNode->cashierNumber;
	int madeSandwichNumber = removeNode->sandwichNumber;

	prevMadeSandwich = madeSandwichNumber;
	numOrdersOnBoard--;
	
	cout << "READY: cashier " << madeCashierID << " sandwich " << madeSandwichNumber << std::endl;
	
	free(removeNode);
	free(removeTempNode);

	// Print Made Sandwich info here

	thread_signal(deliLock, madeCashierID);

}



int submitOrder(CashierInfo* info){
	// fprintf(stderr, "Beginning of submitOrder method \n");
	int fileReadResult;

	int done = fscanf(info->file_ptr, "%d", &fileReadResult);
	// fprintf(stderr, "File was read from \n");

	if(done == EOF){ //if there is no more sandwhiches to read, KILL IT
		numCashiers--; //decrement number of cashiers

		//update largePoss number (since num chashiers aka live threads is diff)
		if(numCashiers<maxOrdersOnBoard){
 			maxOrdersOnBoard = numCashiers;
 		}

		int killThread=1;
		return killThread;

 	}
 	else{ //more sandwhiches to read from this cashier so its NOT done
 		//add sandwhich to board
 		BoardListNode* newOrder = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
 		newOrder->cashierNumber = info->id;
 		newOrder->sandwichNumber = fileReadResult;

 		BoardListNode* temp = startBoardList;
 	
 		while(temp->sandwichNumber!=1001){ //last temp will be 1001 //check this through
 			if(temp->next->sandwichNumber>=fileReadResult){
	 			newOrder->next = temp->next;
	 			temp->next = newOrder;
	 			break;
	 		}
	 		temp = temp->next;
	 	}
 		
 	 	// fprintf(stderr, "Sandwich was added to board \n");
 		numOrdersOnBoard++; 
 		cout << "POSTED: cashier " << newOrder->cashierNumber << " sandwich " << newOrder->sandwichNumber << std::endl;
 		int killThread=0;
 		return killThread;
 	}
 }

// Cashier Method for adding orders
void cashier(void* arg){ //unsigned int for lock??

	// fprintf(stderr, "Cashier method starting \n");

	CashierInfo* myInfo = ((CashierInfo*) arg);

	while(1){
		thread_lock(deliLock);

		// unsigned int x = myInfo->id;
		// unsigned int y = checkSandwhich(startBoardList, myInfo->id);
		// fprintf(stderr, "%u ", x);
		// fprintf(stderr, "\n");
		// fprintf(stderr, "%u ", y);
		// fprintf(stderr, "\n");
		// fprintf(stderr, "%u ", numOrdersOnBoard);
		// fprintf(stderr, "\n");



		while(numOrdersOnBoard == maxOrdersOnBoard || checkSandwhich(startBoardList, myInfo->id)){
			//unsigned int x = myInfo->id;

			//fprintf(stderr, "%u ", x);
			fprintf(stderr, "\n");

			thread_wait(deliLock, myInfo->id);
		}
		//fprintf(stderr, "out of while loop \n");

		// fprintf(stderr, "Cashier Adding Order\n");

		int killThread = submitOrder(myInfo);
		if(killThread == 1){
			thread_signal(deliLock, makerCV); //signal maker so it can pick another cashier’s sandwhich
			cout << "Cashier thread exiting\n";
			thread_unlock(deliLock);
			return;

		}

		thread_signal(deliLock, makerCV);
		thread_unlock(deliLock);
	}
}


// Maker Method for making orders
void maker(void* arg) {

	// fprintf(stderr, "Beginning of maker method \n");
	char **argv = (char **)(arg);
	unsigned int counter = 0;
	for (int i = 2; i < numCashiers + 2; i++){
		//fprintf(stderr, "%s \n",((char*) arg)+i );
	//	FILE* specificCashierFile_ptr = fopen(((char*) arg)+i, "r");											// File reader corresponding to cashier
		FILE* specificCashierFile_ptr = fopen(argv[i], "r");
		unsigned int cashierID = counter;
		counter=counter+1;

		// Stores file reader and cashier number into a struct to pass as one argument
		CashierInfo* specificCashierInfo_ptr = ((struct CashierInfo*)(malloc(sizeof(struct CashierInfo))));
		specificCashierInfo_ptr->file_ptr = specificCashierFile_ptr;
		specificCashierInfo_ptr->id = cashierID;

		//fprintf(stderr, "Calling thread_create for single cashier \n");

		// Creates cashier thread
		if (thread_create((thread_startfunc_t) cashier, ((void*) specificCashierInfo_ptr))){
			cout << "thread_create failed\n";
			exit(1);
		}
	}

	while(1){
		thread_lock(deliLock);
    	//while the board isn't full or the number of orders 
    	//isn't at it's max then we wait for the board to be full 
   		//we give up the lock and wait for the makerCV to change
    	//so that we are woken up! 
    	while(numOrdersOnBoard!=maxOrdersOnBoard){
        	thread_wait(deliLock,makerCV);
    	}
    	//fprintf(stderr, "Maker Removing order\n");
    	//we fall out of the while loop with the lock. so we 
    	//have the lock and now we are going to take off a sandwhich and make it 
    	if(numCashiers==0){
    		free(startBoardList);
    		free(endBoardList);
    		thread_unlock(deliLock);
    		return;
    		//TO DO: KILL IT ALL, END OF THE PROGRAM
    	}else{
			take_off();
    	}

    	//job is done, unlock the thread
    	thread_unlock(deliLock);

		}

}

int main(int argc, char* argv[]){
	//******BRIAN TODO: ~~~~~INITILIZE HERE AND CREATE THREADS~~~~
			//*********
				///*********

	// Initializes global variables
	fprintf(stderr, "main \n");
	//fprintf(stderr, "Initializing Global Variables \n");
	maxOrdersOnBoard = atoi(argv[1]);
	numOrdersOnBoard = 0;
	numCashiers = argc - 2;
	prevMadeSandwich = -1;
	deliLock = 0;
	makerCV = (unsigned int)argc;					// Number we know is greater than number of cashiers

	// fprintf(stderr, "Initializing Boarld List \n");

	// Initializes first and last nodes of the board linked list
	startBoardList = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
	startBoardList->cashierNumber = -1;
	startBoardList->sandwichNumber = -1;
	endBoardList = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
	startBoardList->next = endBoardList;
	endBoardList->cashierNumber = 10001;
	endBoardList->sandwichNumber = 1001;
	endBoardList->next = NULL;


	// fprintf(stderr, "Calling thread_libinit with maker method \n");

	// Initiates maker thread with no argument
	if (thread_libinit((thread_startfunc_t) maker, ((void*) argv))){
		cout << "thread_libinit failed \n";
		exit(1);
	}
	fprintf(stderr, "last main \n");

}


