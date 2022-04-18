#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<sys/types.h>
#include<sys/wait.h>
#define list_max 100
#define clear() printf("\033[H\033[J")

void presentDir();
int readInput(char* line);
int parpipe(char* str, char** pipedString);
void Help();
int cmds_executed(char** token);
void executingargu(char** token);
void executingarguPiped(char** token, char** pipe_parsed);
void space_parse(char* str, char** token);
int stringProcessing(char* str, char** token, char** pipe_parsed);


//the function is to write the current directory
//here we use getcwd to get the directory
void presentDir(){
	char pwd[1024];
	char* user=getenv("USER");
	getcwd(pwd, sizeof(pwd));
	printf("\n%s@%s-ownshell:~%s",user,user,pwd);
}

//help gives the commands we can execute
void Help(){
	puts("\nCommands supported:"
		"\n$cd"//this the change directory
		"\n$exit"//exit the shell
		"\n$ls"// list of files
		"\n$pipe handling");return;
}

//this helps us to read the lines we give this shell
int readInput(char* line){
	char* temp;
//this readline function is in the readline library or header file
	temp = readline("$ ");
	if (strlen(temp) != 0){
		add_history(temp);
		strcpy(line, temp);
		return(0);
	} else{
		return(1);
	}
}



//this function handles the commands we execute
int cmds_executed(char** token){
	int n = 4;
	int switch_args = 0;
	char* list_cmds[n];
	char* user;
	for(int i=0;i<n;i++){
        if(i==0){
             list_cmds[0] = "exit";
        }else if(i==1){
            list_cmds[1] = "cd";
        }else if(i==2){
            list_cmds[2] = "help";
        }else{
            list_cmds[3] = "hi";
        }

    }
	for (int i=0;i<n;i++){
		if (strcmp(token[0], list_cmds[i]) == 0){
			switch_args = i + 1;
			break;
		}
	}

	switch (switch_args) {
	case 1:printf("\n");
	       exit(0);// exits when we run the exit command
	case 2:chdir(token[1]);//changes the directory
           return 1;
	case 3:Help();//show how many commands we can execute
		   return 1;
	case 4:user = getenv("USER");
		  printf("\nhi %s.\n ",user);
		  return 1;
	default:break;
	}

	return 0;
}

void space_parse(char* str, char** token){
	int i=0;
	while(i < list_max){
		token[i] = strsep(&str, " ");
		if (token[i] == NULL){
			break;
        }
		if (strlen(token[i]) == 0){
			i--;
        }
        i++;
	}
}
int parpipe(char* str, char** pipedString){
	int i=0;
	// this function is to the pipe function
	while(i < 2) {
		pipedString[i] = strsep(&str, "|");
		if (pipedString[i] == NULL){
			break;
		}
        i++;
	}
	if (pipedString[1] == NULL){
		return(0);
        }
	else {
		return(1);
	}
}

int stringProcessing(char* str, char** token, char** pipe_parsed){
	char* pipedString[2];
	int piped = 0;
	piped = parpipe(str, pipedString);
	if (piped){
		space_parse(pipedString[0], token);
		space_parse(pipedString[1], pipe_parsed);

	} else{
		space_parse(str, token);
	}

	if (cmds_executed(token)){
		return(0);
	} else{
		return(1 + piped);
	}
}


//here we creating a pipe between the processes
void executingarguPiped(char** token, char** pipe_parsed){
	int pifd[2];
// if the value pipe(pifd) is negative then initialization failed
    pid_t p1, p2;
	if (pipe(pifd) < 0){printf("\nPipe initialize not done");
		return;
	}
	p1 = fork();
	if(p1 < 0){printf("\nForking Failed");//negative number represents
	     //the failure of forking
		return;
	}
	//in attempt to write in the pipe we closed the read mode
	if(p1 == 0){close(pifd[0]);
		dup2(pifd[1], STDOUT_FILENO);
		close(pifd[1]);
		if (execvp(token[0], token) < 0) {
			printf("\nCommand 1 execution failed..");
			exit(0);
		}
		//here we are creating if upper condition is not true then
		//created another process by forking
	}else{p2 = fork();
		if (p2 < 0){
			printf("\nForking Failed");
			return;
		}
    //in attempt to write in the pipe we closed the read mode
		if(p2 == 0){close(pifd[1]);
			dup2(pifd[0], STDIN_FILENO);
			close(pifd[0]);
			if (execvp(pipe_parsed[0], pipe_parsed) < 0) {
                    //command is not executed
				printf("\nCommand 2 execution failed ..");
				exit(0);
			}
		}else{
		      wait(NULL);
		      return;
		}
	}
}

//executing arguments done here by spliting processes
void executingargu(char** token){
    // this here we creating new process
    //for executing the commands
	pid_t pid = fork();
	//done with the forking

	if(pid == -1){
		printf("\nForking Failed");// showing that we are failed in the forking process
		return;
	}else if(pid == 0){
		if (execvp(token[0], token) < 0) {
			printf("\nCommand execution failed ..");
		}
		exit(0);
	}else{
		wait(NULL);
		//the parent process is waiting for child processs to complete
		return;
	}
}

int main(){
	char linesInput[1000], *args_parsed[list_max];
	char* pipedParsedArgs[list_max];
	int flagofexection = 0;
    //loop for writing commands and executing comments
	while (1) {presentDir();
	// it read the the input lines
	//and split into tokens
		if(readInput(linesInput)){
			continue;
		}
		// this part is for excecution of the commands
		flagofexection = stringProcessing(linesInput,args_parsed, pipedParsedArgs);

		// execution of commands are and there pipes between the process
		if (flagofexection == 1){
                executingargu(args_parsed);
		}else if (flagofexection == 2){
		    executingarguPiped(args_parsed,pipedParsedArgs);
		}
	}
	return(0);
}

© 2022 GitHub, Inc.