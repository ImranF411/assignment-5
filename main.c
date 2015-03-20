#define MAX_SIZE 2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*Authors: Michael Sabbagh, Imran Farooqui*/

typedef struct cnode{
	char ncommand[50] [50];
	int ccount;
	int fd[2];
	struct cnode *next;
	struct cnode *prev;
}cnode;

struct clist{
	cnode *head;
	int count;
};
typedef struct clist* CListPtr;

/*Built-in function table element*/
typedef struct builtins{
    char *name;
    int (*fnc)();
}fncEntry;

/*Creates a new list. Returns the pointer.*/
CListPtr CListCreate();
/*Destroys a list when given pointer.*/
void CListDestroy(CListPtr list);
/*Inserts command into a list.*/
int CListInsert(CListPtr list, char  command [50] [50], int index);
/*Removes a command from a list.*/
int CListRemove(CListPtr list);
/*Prints command list*/
void CPrintList(CListPtr list);
/*Forks and executes command*/
void forkAndKnife(char acommand[50][50], int index, int pfd[],int isFirst, int isFinal);
/*Parent process waits for command running process in here.
 *Parent prints exit status.*/
void waitingRoom();
/*built in function for cd*/
int cdFunc(int argc, char *argv[]);
/*built in function for exit*/
int exitFunc(int argc, char *argv[]);

int searchFuncTable(char *command[], int index);

int fillCommandList(char* to_parse);

void forkAndKnife2(cnode* cmdNode);

//char acommand [50][50];

char clcommand [50][50];

fncEntry funcTable[2];

CListPtr commandList;

int main()
{
    char* to_parse = (char*) calloc(MAX_SIZE,sizeof(char));

    fncEntry cdEntry = {"cd\0",&cdFunc};
    fncEntry exitEntry = {"exit\0",&exitFunc};

    funcTable[0] = cdEntry;
    funcTable[1] = exitEntry;

	if(isatty(0))
		printf("$>");

    	while(fgets(to_parse,MAX_SIZE,stdin) != NULL){
	    commandList = CListCreate();
            int index = 0, complete = 0, commandIndex = 0, aindex = 0, isFirst = 1, quoteCheck = 0, quoteCheck2 = 0;
            char myCommand [100];

            //Create pipe
            int fd[2];
            pipe(fd);

	    //Check if empty input
	    if(to_parse[0] == '\n'){
		printf("\n$>");
		CListDestroy(commandList);
		continue;
	    }

	    //Count the number of " and '
	    while(index < strlen(to_parse)){
		if(to_parse[index] == '\"'){
			quoteCheck++;
		}
		if(to_parse[index] == '\''){
			quoteCheck2++;
		}
		index++;
	    }

	    //Check if quotes are mismatched
        if((quoteCheck % 2) || (quoteCheck2 % 2)){
            printf("Error: mismatched quote\n$>");
            CListDestroy(commandList);
            continue;
	    }
	    index = 0;

   	    //Check if input is only spaces and/or tabs
	    while(to_parse[index] == ' ' || to_parse[index] == '\t' || to_parse[index] == '\n'){
		index++;
	    }
	    if(index == strlen(to_parse)){
		printf("\n$>");
		CListDestroy(commandList);
		continue;
            }

	    index = 0;


	    fillCommandList(to_parse);
            //Go through to_parse until we have processed all commands
            /*while(index < strlen(to_parse) && !complete){

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
                        strcpy(acommand[aindex], myCommand);
                        aindex++;
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
                        strcpy(acommand[aindex], myCommand);
                        aindex++;
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
                        strcpy(acommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                        continue;
                    }else{continue;}
                }

                //If we hit a pipe, entire argument is stored in list and ready for processing
                if(to_parse[index] == '|'){
                    index++;
                    if(strlen(myCommand) != 0){
                        myCommand[commandIndex] = '\0';
                        strcpy(acommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                    }
                    commandIndex = 0;
                    memcpy(myCommand, "\0", 100);

                    //forkAndKnife(acommand, aindex, fd, isFirst, 0); //Execute command
                    isFirst = 0;

                    /*Empty out string holding command
                    int i = 0;
                    while(i < aindex){
                        memcpy(acommand[i], "\0", 50);
                        i++;
                    }
                    aindex = 0;
                    continue;
                }
                if(to_parse[index] != '\n'){
                    myCommand[commandIndex] = to_parse[index];
                }
                commandIndex++;
                index++;

                //If we reach the end of the input command / list of commands
                if(index >= strlen(to_parse)){
                    if(strlen(myCommand) != 0){
                        myCommand[commandIndex-1] = '\0';
                        strcpy(acommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                    }
                    memcpy(acommand[aindex], "\0", 50);
                    //forkAndKnife(acommand, aindex, fd, isFirst, 1);

                    /*empty out string holding command
                    int i = 0;
                    while(i < aindex){
                        memcpy(acommand[i], "\0", 50);
                        i++;
                    }*/
		    //CPrintList(commandList);
		    cnode* exec = commandList->head;        //Start at command list's head
		    for(;exec != NULL; exec = exec->next){  //Loop through all of them
                pipe(exec->fd);                     //Set up pipe
                forkAndKnife2(exec);                //Execute command
                close(exec->fd[1]);                 //Close current write-to pipe, current process is done writing to here.
                if(exec->prev != NULL)              //Close previous read-from pipe, current process is done reading from here.
                    close(exec->prev->fd[0]);
                if(exec->next == NULL)              //Close reading pipe of last command.
                    close(exec->fd[0]);
		    }
		    waitingRoom();
		    CListDestroy(commandList);
                    complete = 1;
//                }
//            }

    		//forkAndKnife(acommand);
    		//fputs(to_parse,stdout);
    		//waitingRoom();			//Wait for child processes to die
        	printf("\n$>");
	}

    /*printf("Completed reading in. Press enter to exit.\n");
 *     gets(to_parse);*/
    return 0;
}


/*This function forks a child process and has it execute a command with arguments, passed in function call.
 * On successful fork, the parent waits for child process in function waitingRoom(int).
 * If the fork fails, the process exits with status 1.
 * If the execvp fails, the child process exits with status 1.
 */
void forkAndKnife(char acommand[50][50], int index, int pfd[],int isFirst, int isFinal){
	int pid;
	int i = 0;

	char* pcommand [50];

	while(i < index){
		pcommand[i] = acommand[i];
		i++;
	}

	pcommand[index] = NULL;

	if(searchFuncTable(pcommand, index) == 1)
        return;

	switch(pid = fork()){
	//Child process case
	case 0:
		//call execvp here because it searches path for command. We won't have to search it ourselves
		//Pipe work done here

		if(isFirst)
			close(pfd[0]);  //1st command does not need incoming end.
		else
			dup2(pfd[0],0);

		if(isFinal){
			close(pfd[1]); //Last command does not need outgoing end.
		}else{
			dup2(pfd[1],1);
		}
		execvp(pcommand[0], pcommand);
		perror("execvp");
		exit(1);

		break;
	//fork() fails case
	case -1:
		perror("fork");
		exit(1);
		break;
	//Parent process case
	default:
		break;
	}
	return;
}

void forkAndKnife2(cnode* cmdNode){ /*electric boogaloo*/
    int pid;
	int i = 0;
	int index = cmdNode->ccount;

	char* pcommand [50];

	while(i < index){
		pcommand[i] = cmdNode->ncommand[i];
		//acommand[i];
		i++;
	}

	pcommand[index] = NULL;

	if(searchFuncTable(pcommand, index) == 1)
        return;

	switch(pid = fork()){
	//Child process case
	case 0:
		//call execvp here because it searches path for command. We won't have to search it ourselves
		//Pipe work done here
		if(cmdNode->prev != NULL)//Is there a previous command?
            dup2(cmdNode->prev->fd[0],0); //If so, use its fd[0] for standard input.
        if(cmdNode->next != NULL)//Is there a next command?
            dup2(cmdNode->fd[1],1); //If so, use this node's fd[1] for standard output.
		execvp(pcommand[0], pcommand); //Execute command
		perror("execvp"); //Houston, we have a problem.
		exit(1);
		break;
	//fork() fails case
	case -1:
		perror("fork"); //In the words of President Madagascar "SHUT DOWN EVERYTHING"
		exit(1);
		break;
	//Parent process case
	default:
		break;          //Talk about lazy parenting...
	}
	return;
}

/*Parent process waits in here for child process.
 *Parent captures and prints s
 */
void waitingRoom(){

	int status,pid;
	//Parent loops on wait until wait returns -1, which is when all child processes have finished.
	while ((pid = wait(&status)) != -1)
		//Parent prints the exit status of each completed child process.
		fprintf(stderr, "Child process %d exits with %d.\n",pid, WEXITSTATUS(status));
	return;
}
/*Built in function for changing directory "cd"*/
int cdFunc(int argc, char *argv[]){
    char *dir;
    if(argc = 1){
            fprintf(stderr,"No directory specified. Going to home directory.\n");
        return chdir(getenv("HOME")); //Default to home environment.
    }
    if(argc = 2)
        dir = argv[1];
    if(argc > 2){
        fprintf(stderr,"Too many arguments. Staying in current directory.\n");
        return 1;
    }
    int ret = chdir(dir);
    if(ret == 0){
        fprintf(stderr,"%s",getcwd(NULL,0));
    }else{
        perror("chdir");
        return 1;
    }
    return 0;
}
/*built in function for exiting shell "exit"*/
int exitFunc(int argc, char *argv[]){
    long code = 0;
    if(argc != 1){
        code = strtol(argv[1], NULL, 10);
	}
    printf("Shell is exiting with code %ld\n", code);
    exit(code);
    return 0;
}

/*searches function table for a built in function*/
int searchFuncTable(char *command[], int index){
    int i = 0;
    //fprintf(stderr,"Built in function: %s\n",command[0]);
    for(; i<2;i++){     //Search function table
        if(strcmp(command[0],funcTable[i].name) == 0){  //If function found...
            int fnc = (*funcTable[i].fnc)(index,command);
            return 1;
        }
    }
    return 0;
}


int fillCommandList(char* to_parse){
            int index = 0, complete = 0, commandIndex = 0, aindex = 0;
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
                        strcpy(clcommand[aindex], myCommand);
                        aindex++;
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
                        strcpy(clcommand[aindex], myCommand);
                        aindex++;
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
                        strcpy(clcommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                        continue;
                    }else{continue;}
                }

                //If we hit a pipe, entire argument is stored in list and ready for processing
                if(to_parse[index] == '|'){
                    index++;
                    if(strlen(myCommand) != 0){
                        myCommand[commandIndex] = '\0';
                        strcpy(clcommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                    }
                    commandIndex = 0;
                    memcpy(myCommand, "\0", 100);
		    CListInsert(commandList, clcommand, aindex);
		    commandList->count++;
                            //forkAndKnife(acommand, aindex, fd, isFirst, 0); //Execute command


                    /*Empty out string holding command*/
                    int i = 0;
                    while(i < aindex){
                        memcpy(clcommand[i], "\0", 50);
                        i++;
                    }
                    aindex = 0;
                    continue;
                }
                if(to_parse[index] != '\n'){
                    myCommand[commandIndex] = to_parse[index];
                }
                commandIndex++;
                index++;


                //If we reach the end of the input command / list of commands
                if(index >= strlen(to_parse)){
                    if(strlen(myCommand) != 0){
                        myCommand[commandIndex-1] = '\0';
                        strcpy(clcommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                    }
                    memcpy(clcommand[aindex], "\0", 50);
                    //forkAndKnife(clcommand, aindex, fd, isFirst, 1);
		   CListInsert(commandList, clcommand, aindex);
      		   char clcommand [50][50];
		    commandList->count++;
             /*empty out string holding command*/
                    int i = 0;
                    while(i < aindex){
                        memcpy(clcommand[i], "\0", 50);
                        i++;
                    }
                    complete = 1;
                }
            }
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
int CListInsert(CListPtr list, char command [50] [50], int index){
	if(list == NULL ){
		printf("List or data are null in %s, line %d", __FILE__, __LINE__);
		return 0;
	}
	cnode* newNode = (cnode*)malloc(sizeof(cnode));
	int i = 0;
	while(i < index){
		strcpy(newNode->ncommand[i], command[i]);
		i++;
	}
	//newNode->ncommand[index] = "0";
	newNode->ccount = index;

	if(list->head == NULL){
		list->head = newNode;
		list->head->next = NULL;
		list->head->prev = NULL;
	}else{
		cnode * tmp = list->head;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = newNode;
		newNode->prev = tmp;
	}

	return 0;

}

/*Removes first node from LL.*/
int CListRemove(CListPtr list){
	if(list == NULL ){
		printf("List or data are null in %s, line %d", __FILE__, __LINE__);
		return 0;
	}
	if(list->head != NULL){
        list->head->next->prev = NULL;
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
	int i, j;
	j = 1;
	while(tmp != NULL){
		i = 0;
		printf("Command Number: %d\n", j);
		while(i < tmp->ccount){
			printf("%s-> ", tmp->ncommand[i]);
			i++;
		}

		printf("\n");
		j++;
		tmp = tmp->next;
	}
}
