void take_off() {
    //BoardListNode* temp = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
    BoardListNode* temp = startBoardList->next;
    //BoardListNode* removeTempNode = temp;
	//temp->cashierNumber = 0; //************change from NULL
	//temp->sandwichNumber = 0;
	//temp->next = startBoardList;
	
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
