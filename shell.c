#define  _POSIX_C_SOURCE 200809L

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<assert.h>

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }

void command(char* c) {
    char * sep = NULL;

    if((sep=strstr(c,"2>"))) { // 2
        *sep=0;
        char * fileout = sep+2;

        if(fileout[strlen(fileout) - 1  ] == ' '){ //cleaing blank spaces from string
             fileout[strlen(fileout) - 1 ] = '\0';
        }
       
         int write_fd ; 
         
        write_fd = open(fileout, O_WRONLY |  O_CREAT | O_TRUNC ,  0700) ; //giving all permissions

        if(write_fd == -1){return;} //returning if write_fd returns -1 (Fail Code)

        dup2(write_fd, 2);

    }

    if((sep=strstr(c,">"))) { // 1
        *sep=0;
        char * fileout = sep+1;
        
        if(fileout[strlen(fileout) - 1  ] == ' '){//cleaing blank spaces from string
             fileout[strlen(fileout) - 1 ] = '\0';
        }
       
        int write_fd ; 
        write_fd = open(fileout, O_WRONLY |  O_CREAT | O_TRUNC,  0700) ; //giving all permissions
        
        if(write_fd == -1){return;} //returning if write_fd returns -1 (Fail Code)

        dup2(write_fd, 1) ;
        
    }

    if((sep=strstr(c,"<"))) { // 0
        *sep=0;
        char * filein = sep+1;
          
        if(filein[strlen(filein) - 1  ] == ' '){//cleaing blank spaces from string
             filein[strlen(filein) - 1 ] = '\0';
        }
        
        int write_fd ; 
        
        write_fd = open(filein, O_RDONLY) ;     

        if(write_fd == -1){return;} //returning if write_fd returns -1 (Fail Code)
        dup2(write_fd, 0); 
    }
    
    char* program = strtok(c," ");
    
    char *args[16]= {program};

    if(args[0] == NULL){return;}

    for(int i = 1 ; i < 16; i++){

        args[i] = strtok(NULL," ");
        if(args[i] == NULL){break;}

}    

    if (!strcmp("echo",program)) {
       
    for(int i = 1 ; i < 16; i++){
          
            if(args[i] == NULL){return;} //change maybe
            printf("%s\n", args[i]) ; 
}

    } else {
       
        char * paths = getenv("PATH");   
        char* current_path = strtok(paths,":");
        char  program_with_path[100] ; 

        while(current_path != NULL){

             snprintf(program_with_path, 100, "%s/%s", current_path, program) ; 
             execv(program_with_path,args) ;

            current_path = strtok(NULL,":") ; 
        }
        
        execv(program,args) ;
           
        fprintf(stderr,"cs361sh: command not found: %s\n",program);
        fflush(stderr);
    }

}

void run(char*);

void pipeline(char* head, char* tail) {
    int fds[2] ; 
   
   if( pipe(fds) ==-1){return;}  

    int return_code; 
    int read_fd = fds[0];
    int write_fd = fds[1];
    int ret = fork() ; 
   
   if(ret != 0 ){ //parent process

    waitpid(ret, &return_code , 0 ); 
    dup2(read_fd, 0) ;
    //run(head) ; 
    close(read_fd) ;
    close(write_fd) ; 
    run(tail) ; 
    // dup2(read_fd, 0) ;    

   }else{ //child //tail goes in parent

        dup2(write_fd,1) ; //moving starderd output
        close(read_fd) ; 
        close(write_fd) ; //closing files
        run(head) ;
        exit(0) ;
        
   }

}

void sequence(char* head, char* tail) {

    run(head) ; 
    run(tail) ; 

}

void run(char *line) {
    char *sep = 0;


    if((sep=strstr(line,";"))) {
        *sep=0;
        sequence(line,sep+1);
    }
    else if((sep=strstr(line,"|"))) {
        *sep=0;
        pipeline(line,sep+1);
    }
    else {

       int ret = fork(); 
       int return_code; 
        if(ret !=0){

        waitpid(ret, &return_code , 0 ); 
        
        }else {

            command(line);
            exit(0) ; 
        
        }
      
    }
}
int main(int argc, char** argv) {
    char *line=0;
    size_t size=0;

    printf("Shell> ");
    fflush(stdout);

    int orig_in=dup(0);
    int orig_out=dup(1);
    int orig_err=dup(2);

    while(getline(&line,&size,stdin) > 0) {

        line[strlen(line)-1]='\0'; // remove the newline
        
        run(line);

        printf("Shell> ");
        fflush(stdout);
   }

    printf("\n");

    fflush(stdout);
    fflush(stderr);
    return 0;
}
