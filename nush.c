#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


#include "tokenize.h"

//check for semi
//	split on semi call left/right
//check for pipe
//	fork wait
//	split on pipe
//	make pipe
//	fork to exec left
//	fork to exec right
//	wait on both children
//check for && ||
//check for &(background)
//	fork exec
//	don't wait for child
//check for <,>
//	fork wait on parent
//	set up redirect
//	exec
//
//inspired by starter code from lecture
void
execute(char* cmd, char* args[], int length)
{

    
    //cd
        
    if (strcmp(args[0], "cd") == 0) {
        chdir(args[1]);
        return;
    }
    //check for ;
    //printf("command = %s\n", cmd);
    //printf("length = %d\n", length); 
    int i;
    //for (i = 0; i < length; ++i) {
    //    printf("%s\n", args[i]);
    //}
    for (i = 1; i < length; ++i) {
        //printf("a[%d] = %s\n", i, args[i]); 
        //printf("%d\n", sizeof(char*));
        if (strcmp(args[i], ";") == 0) {
            char** left = malloc(i * sizeof(char*));
            char** right = malloc((length - i - 1) * sizeof(char*));
            
            memcpy(left, args, i * sizeof(char*));
            memcpy(right, args + i + 1, (length - i) * sizeof(char*));
            //printf("%s\n", firstHalf[0]);
            execute(args[0], left, i); 
            //printf("%s\n", secondHalf[0]);
            execute(args[i + 1], right, length - i - 1);
            

            free(left);
            free(right);
            return; 

        }
    }
    int cpid; 
    //check for pipe
    for(i = 1; i < length - 1; ++i) {
        if (strcmp(args[i], "|") == 0) {
            int cpid;
            int cpid1;
            char** left = malloc(i * sizeof(char*));
            //printf("first size = %d\n", i);
            char** right = malloc((length - i - 1) * sizeof(char*));
            //printf("second size = %d\n", (length - i -1));
            memcpy(left, args, i * sizeof(char*));
            memcpy(right, args + i + 1, (length - i) * sizeof(char*));
            
             
            if((cpid = fork())) {
                int status;
                waitpid(cpid, &status, 0);
            }
            else {
                
                int pipe_fds[2];
                pipe(pipe_fds);
            
                int p_read = pipe_fds[0];
                int p_write = pipe_fds[1];
                if ((cpid1 = fork())) {
                    close(p_write);
                    int status;
                    waitpid(cpid1, &status, 0);

                    close(0); //stdin
                    dup(p_read);
                    execvp(right[0], right);
                    
                }
                else {
                    close(p_read);
                    close(1); //stdout
                    dup(p_write);
                    execvp(left[0], left);
                }
            }
            free(left);
            free(right);
            
            return;
        }
    }
    //&& ||
    int exit = 0;
    for (i = 1; i < length - 1; ++i) {
        if (strcmp(args[i], "||") == 0) { 
            char** left = malloc(i * sizeof(char*));
            char** right = malloc((length - i - 1) * sizeof(char*));
        
            memcpy(left, args, i * sizeof(char*));
            memcpy(right, args + i + 1, (length - i) * sizeof(char*));

             
            if(cpid = fork()) {
                int status;
                waitpid(cpid, &status, 0);
                exit = WEXITSTATUS(status);
            }
            else {
                execvp(left[0], left); 
            }
            
            if (exit != 0) {
                if(cpid = fork()) {
                    int status;
                    waitpid(cpid, &status, 0);
                }
                else {
                    execvp(right[0], right); 
                }
            }

            free(left);
            free(right);
            return;
        }
    //}
    //for (i = 1; i < length - 1; ++i) {
   
        if (strcmp(args[i], "&&") == 0) { 
            char** left = malloc(i * sizeof(char*));
            char** right = malloc((length - i - 1) * sizeof(char*));
        
            memcpy(left, args, i * sizeof(char*));
            memcpy(right, args + i + 1, (length - i) * sizeof(char*));

             
            if(cpid = fork()) {
                int status;
                waitpid(cpid, &status, 0);
                exit = WEXITSTATUS(status);
            }
            else {
                execvp(left[0], left); 
            }
            
            if (exit == 0) {
                if(cpid = fork()) {
                    int status;
                    waitpid(cpid, &status, 0);
                }
                else {
                    execvp(right[0], right); 
                }
            }

            free(left);
            free(right);
            return;
        }
    }

    for (i = 1; i < length; i++) {
        if (strcmp(args[i], "<") == 0) {
            char** left = malloc(i * sizeof(char*));
            memcpy(left, args, i * sizeof(char*)); 
             
            //printf("file = %s\n", args[i + 1]);

            //FILE* file = fopen(args[i + 1], "r");
            //int fd = fileno(file);
            //fclose(file);
            //printf("%d\n", fd);
            //close(0);//close stdin
            //printf("before open");
            //int fd = open("tests/sample.txt", O_RDONLY);
            //printf("after open");
            if (cpid = fork()) {
                int status;
                waitpid(cpid, &status, 0);

            }
            else {
	        int ii;
                for (ii = 0; ii < strlen(cmd); ++ii) {
                    if (cmd[ii] == ' ') {
                        cmd[ii] = 0;
                        break;
                    }
                }
                int fd = open(args[i + 1], O_RDONLY);
                //char* buf;
                //read(fd, buf, 10);
                //printf("%s\n", buf);
                close(0);
                dup(fd);
                close(fd);
                execvp(left[0], left);
            
            }
            free(left);
            
            return;
        }
        
        if (strcmp(args[i], ">") == 0) {
            char** left = malloc(i * sizeof(char*));
            memcpy(left, args, i * sizeof(char*));
	    //FILE* file = fopen(args[i + 1], "w");
	    //int fd = fileno(file);
            int fd = open(args[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
            //close(fd);
            //close(1);  //close stdout
            //fclose(file);
            if (cpid = fork()) {
                int status;
                waitpid(cpid, &status, 0);
            }
                    
            else {
   
	        int ii;
                for (ii = 0; ii < strlen(cmd); ++ii) {
                    if (cmd[ii] == ' ') {
                        cmd[ii] = 0;
                        break;
                    }
                }
                close(1);
                dup(fd);
                execvp(left[0], left);
            }
            close(fd); 
            
            free(left);
            
            return;
        }
        
    }

 
    //background (&)
    int contains = 0;
  
    for (i = 1; i < length; ++i) {
        if (strcmp(args[i], "&") == 0) {
            char** args2 = malloc(i * sizeof(char*));
            memcpy(args2, args, i * sizeof(char*));
        //memcpy(args2, args + i - 1, (length - i) * sizeof(char*));
            //args = args2;
            //printf("length = %d\n", length);
            //args = realloc(args, i * sizeof(char*));
            //int i;
            //for (i = 0; i < (length); ++i) {
            //    printf("args2[%d] = %s\n", i, args2[i]);
            //}
            int cpid;
            if(cpid = fork()) {
                //int status;
                //waitpid(cpid, &status, 0);       
            }
            else {
                //fflush(stdout);
                execvp(cmd, args2);
            }
            free(args2);
            return;
        }
    }
 
    //normal operation
    
    if ((cpid = fork())) {

        int status;
        waitpid(cpid, &status, 0);
    }
    else {
	
	int ii;
        for (ii = 0; ii < strlen(cmd); ++ii) {
            if (cmd[ii] == ' ') {
                cmd[ii] = 0;
                break;
            }
        }


        execvp(cmd, args);
    }
}

int
main(int argc, char* argv[])
{ 
    


    if (argc == 1) {
        char cmd[256];
        while(1) {
            printf("nush$ ");
            fflush(stdout);
            char* rv = fgets(cmd, 256, stdin);
            //if (!rv) {
            //    continue;
            //}
            svec* sv = tokenize(rv);
       
            
            //for (ii = 0; ii < sv->size; ++ii) {
            //    printf("%s\n", sv->data[ii]);
            //}	           
            if(sv->size > 0 && strcmp(sv->data[0], "exit") == 0) {
                break;
            }
            //svec_push_back(sv, 0);
            execute(sv->data[0], sv->data, sv->size); 
            free_svec(sv);
        }
        //free(cmd);
        //free_svec(sv);
        //free(rv);
          
    }
    else {
        char temp[100];
        FILE* script = fopen(argv[1], "r");
	
        while (fgets(temp, 100, script) != NULL) {
            svec* sv = tokenize(temp);
            execute(sv->data[0], sv->data, sv->size);
            free_svec(sv);
        }
        fclose(script);
    }
	

    return 0;
}
