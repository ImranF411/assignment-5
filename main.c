/* This shell program made by:
 * Imran Farooqui
 * Michael Sabbagh
 * */

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
int CListInsert(CListPtr list, char* command);
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

int searchFuncTable(char *command[]);

char acommand [50][50];

fncEntry funcTable[2];

int main()
{
    char* to_parse = (char*) calloc(MAX_SIZE,sizeof(char));

    fncEntry cdEntry = {"cd",&cdFunc};
    fncEntry exitEntry = {"exit",&exitFunc};

    funcTable[0] = cdEntry;
    funcTable[1] = exitEntry;

	if(isatty(0))
		printf("$>");

    	while(fgets(to_parse,MAX_SIZE,stdin) != NULL && to_parse[0] != '\n'){
            int index = 0, complete = 0, commandIndex = 0, aindex = 0, isFirst = 1;
            char myCommand [100];

            //Create pipe
            int fd[2];
            pipe(fd);

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
                    }
                }

                //If we hit a pipe, entire argument is stored in list and ready for processing
                //Process where CPrintList is called
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

                            forkAndKnife(acommand, aindex, fd, isFirst, 0); //Execute command
                    isFirst = 0;

                    /*Empty out string holding command*/
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
                //Process final command where CPrintList is called
                if(index >= strlen(to_parse)){
                    if(strlen(myCommand) != 0){
                        myCommand[commandIndex-1] = '\0';
                        strcpy(acommand[aindex], myCommand);
                        aindex++;
                        memcpy(myCommand, "\0", 100);
                        commandIndex = 0;
                    }
                    memcpy(acommand[aindex], "\0", 50);
                    forkAndKnife(acommand, aindex, fd, isFirst, 1);

                    /*empty out string holding command*/
                    int i = 0;
                    while(i < aindex){
                        memcpy(acommand[i], "\0", 50);
                        i++;
                    }
                    complete = 1;
                }
            }

    		//forkAndKnife(acommand);
    		//fputs(to_parse,stdout);
            close(fd[0]); close(fd[1]);	//close pipes.
    		waitingRoom();			//Wait for child processes to die
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

	if(searchFuncTable(pcommand) == 1)
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
		perror("execlp");
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

int cdFunc(int argc, char *argv[]){
    /*check number of elements*/
  //  if(argc > 3){
        fprintf(stderr,"cd: number of arguments\n%d\n",argc);
    //     return;
    //}
    char* dir = getenv("HOME"); //Default to home environment.
    if(argc > 2)
        dir = argv[1];
    int ret = chdir(dir);
    if(ret == 0){
        fprintf(stderr,"%s",getcwd(NULL,0));
    }else{
        perror("chdir");
        return 1;
    }
    return 0;
}

int exitFunc(int argc, char *argv[]){
    long int code = 0;
    if(argc != 1)
        code = strtol(argv[1],NULL,10);
    exit(code);
    return 0;
}

int searchFuncTable(char *command[]){
    int i = 0;
    fprintf(stderr,"Built in function: %s\n",command[0]);
    for(; i<2;i++){     //Search function table
        if(strcmp(command[0],funcTable[i].name) == 0){  //If function found...
            int argc = sizeof(command)/sizeof(command[0]);
            int a = 0;
            for(;a < argc;a++){
                fprintf(stderr,"%s\n",command[a]);
            }
            int fnc = (*funcTable[i].fnc)(argc,command);
            return 1;
        }
    }
    return 0;
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
	if(list->head == NULL){
		list->head = newNode;
	}else{
		cnode * tmp = list->head;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = newNode;
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
