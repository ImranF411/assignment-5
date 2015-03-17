#define MAX_SIZE 2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct cnode{
	char command[150];
	struct cnode *next;
}cnode;

struct clist{
	cnode *head;
};
typedef struct clist* CListPtr;

/*Creates a new list. Returns the pointer.*/
CListPtr CListCreate();
/*Destroys a list when given pointer.*/
void CListDestroy(CListPtr list);
/*Inserts command into a list.*/
int CListInsert(CListPtr list, char* command);
/*Removes a command from a list.*/
int CListRemove(CListPtr list);
/*Prints command list*/
void CPrintList(CListPtr list);


CListPtr command;

int main()
{
    char* to_parse = (char*) calloc(MAX_SIZE,sizeof(char));
    printf("Beginning shell.\n");
    printf("Enter an empty line to exit.\n");
    do{
		//create a list of arguments
		command = CListCreate();
		int index = 0, complete = 0, commandIndex = 0;
		char myCommand [100];

		//Go through to_parse until we have processed all commands
		while(index < strlen(to_parse) && !complete){
			//If we hit a " then get the full argument
			if(to_parse[index] == '\"'){
				index++;
				while((to_parse[index] != '\"') && (index < strlen(to_parse))){
					myCommand[commandIndex] = to_parse[index];
					commandIndex++;
					index++;
				}
				index++;

				if(strlen(myCommand) != 0){
					myCommand[commandIndex] = '\0';
					CListInsert(command, myCommand);
					memcpy(myCommand, "\0", 100);
					commandIndex = 0;
				}
				continue;
			}
			//If we hit a ' then get the full argument
			if(to_parse[index] == '\''){
				index++;
				while((to_parse[index] != '\'') && (index < strlen(to_parse))){
					myCommand[commandIndex] = to_parse[index];
					commandIndex++;
					index++;
				}
				index++;

				if(strlen(myCommand) != 0){
					myCommand[commandIndex] = '\0';
					CListInsert(command, myCommand);
					memcpy(myCommand, "\0", 100);
					commandIndex = 0;
				}
				continue;
			}
			//If we hit a space, store the argument in the list
			if(to_parse[index] == ' '){
				index++;
				if(strlen(myCommand) != 0){
					myCommand[commandIndex] = '\0';
					CListInsert(command, myCommand);
					memcpy(myCommand, "\0", 100);
					commandIndex = 0;
					continue;
				}
			}

			//If we hit a pipe, entire argument is stored in list and ready for processing
			if(to_parse[index] == '|'){
				index++;
				if(strlen(myCommand) != 0){
					myCommand[commandIndex] = '\0';
					CListInsert(command, myCommand);
					memcpy(myCommand, "\0", 100);
					commandIndex = 0;
				}
				commandIndex = 0;
				memcpy(myCommand, "\0", 100);
				CPrintList(command);
				while(CListRemove(command) != 0){
				   
				}
				continue;
			}
			if(to_parse[index] != '\n'){
				myCommand[commandIndex] = to_parse[index];
			}
			commandIndex++;
			index++;
			
			if(index >= strlen(to_parse)){
				if(strlen(myCommand) != 0){
					myCommand[commandIndex-1] = '\0';
					CListInsert(command, myCommand);
					memcpy(myCommand, "\0", 100);
					commandIndex = 0;
				}

				CPrintList(command);
				complete = 1;
			}
		}

    		fputs(to_parse,stdout);
        	printf("\n$>");
		//forkAndKnife(--INSERT COMMAND HERE--);
		
		CListDestroy(command);
	
    }while(fgets(to_parse,MAX_SIZE,stdin) != NULL && to_parse[0] != '\n');
    /*printf("Completed reading in. Press enter to exit.\n");
 *     gets(to_parse);*/
    return 0;
}


/*This function forks a child process and has it execute a command with arguments, passed in function call.
 * On successful fork, the parent waits for child process in function waitingRoom(int).
 * If the fork fails, the process exits with status 1.
 * If the execvp fails, the child process exits with status 1.
 */
void forkAndKnife(char argv[][]){ //Assuming command is in a string array. Adjust this as necessary.
	int pid;
	switch(pid = fork()){
	//Child process case
	case 0:
		//call execvp here because it searches path for command. We won't have to search it ourselves
		execvp(argv[0], argv);	//Adjust Here as well.
		perror("failed execvp");
		exit(1);
		break;
	//fork() fails case
	case -1:
		perror("failed fork");
		exit(1);
		break;
	//Parent process case
	default:
		ret = waitingRoom(pid);
		break;
	}
	return;
}

/*Parent process waits in here for child process.
 *Parent captures and prints s
 */
void waitingRoom(){

	int status,pid;
	//Parent loops on wait until wait returns -1, which is when all child processes have finished.
	whlie ((pid = wait(&status)) != -1)
		//Parent prints the exit status of each completed child process.
		fprintf(stderr, "Child process %d exits with %d.\n",pid, WEXITSTATUS(status));
	return;
}

/*Creates a new linked list.*/
CListPtr CListCreate(){
	CListPtr newList = malloc(sizeof(CListPtr));
	newList->head = NULL;
	return newList; 
}

/*Destroys a linked list.*/
void CListDestroy(CListPtr list){
	cnode* tmp;
	cnode* tmp2;
	tmp = list->head;

	while(tmp != NULL){
		tmp2 = tmp;
		tmp = tmp->next;
		free(tmp2);
	}
}


/*Inserts data to the front of the linked list.*/
int CListInsert(CListPtr list, char* command){
	if(list == NULL ){
		printf("List or data are null in %s, line %d", __FILE__, __LINE__);
		return 0;
	}
	cnode* newNode = (cnode*)malloc(sizeof(cnode));
	strcpy(newNode->command, command);
	newNode->next = list->head;
	list->head = newNode;	
	return 0;

}

/*Removes first node from LL.*/
int CListRemove(CListPtr list){
	if(list == NULL ){
		printf("List or data are null in %s, line %d", __FILE__, __LINE__);
		return 0;
	}
	if(list->head != NULL){
		list->head = list->head->next;
		return 1;
	}
	return 0;
}

void CPrintList(CListPtr list){
	if(list == NULL){
		printf("List is null in %s, line %d", __FILE__, __LINE__);
		return;
	}

	cnode* tmp;
	tmp = list->head;
	printf("Command List:");
	while(tmp != NULL){
		if(tmp->next == NULL){
			
			printf("%s \n", tmp->command);
			return;
		}
		printf("%s-> ", tmp->command);
		tmp = tmp->next;
	}
}
