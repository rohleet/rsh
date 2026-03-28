#include <stdlib.h>
#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>
#include <limits.h>
#include <pwd.h>


#define RSH_RL_BUFSIZE 1024
#define RSH_TOK_BUFSIZE 64
#define RSH_TOK_DELIM " \t\r\n\a"

/*User input command control functions declaration*/
void rsh_loop(void);
char* rsh_read_line(void);
char** rsh_split_line(char *line);

/*Command execution function decalration*/
int rsh_execute(char** args);

/*Built in commands function declaration*/
int rsh_cd(char **args);
int rsh_help(char **args);
int rsh_exit(char **args);
int rsh_pwd(char **args);
int rsh_echo(char **args);
int rsh_type(char **args);
int rsh_export(char **args);


/*List of built in commands and corresponding functions*/
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "pwd",
    "echo",
    "type",
    "export"
};

char *builtin_type[] = {
    "directory management",
    "command help",
    "shell termination",
    "directory management",
    "printing",
    "type command itself"
};

/*Function pointers*/
int (*builtin_func[]) (char **) = {
    &rsh_cd,
    &rsh_help,
    &rsh_exit,
    &rsh_pwd,
    &rsh_echo,
    &rsh_type,
    &rsh_export
};

/*Builtin command counter*/
int rsh_num_builtins();

/*Prompt features*/

void dir_prompt_printer();
void username_prompt_printer();
void hostname_prompt_printer();

int prompt_printer();

/*main function : */
int main(int argc, char**argv){

    rsh_loop();

    return EXIT_SUCCESS;

}

/*Function defenitions*/
/*User input command control functions defenition*/
void rsh_loop(void){

    char *line;
    char **args;
    int status;

    do{

        prompt_printer();
        line = rsh_read_line();
        args = rsh_split_line(line);
        status = rsh_execute(args);

        free(line);
        free(args);

    } while(status);


}

char  *rsh_read_line(void){

    int bufsize = RSH_RL_BUFSIZE;
    int position  = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "rsh : allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {

        c = getchar();

        if(c == EOF || c =='\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize) {
            bufsize += RSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer) {
                fprintf(stderr, "rsh : allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

    }
}

//Implement the previous part simply using getline ! : Note to dev!
char **rsh_split_line(char *line) {

    int bufsize = RSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if(!tokens) {
        fprintf(stderr, "rsh :allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, RSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position>=bufsize){
            bufsize+=RSH_TOK_BUFSIZE;
            tokens = realloc(tokens,bufsize*sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "rsh : allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL,RSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int rsh_launcher(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid==0) {
        //Child process
        if(execvp(args[0],args) == -1){
            perror("rsh");
        }
        exit(EXIT_FAILURE);
    } else if(pid<0){
        //fork failure
        perror("rsh");
    } else {
        //Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*Bultin command counter defenition*/
int rsh_num_builtins() {
    return(sizeof(builtin_str)/sizeof(char *));
}

/*Built in commands function defenition*/
int rsh_cd(char **args) {

    if (args[1]==NULL) {
        fprintf(stderr,"rsh:expected argument to \"cd\"\n");
    } else {

        if (chdir(args[1]) != 0) {
            perror("rsh");
        }

    }
    return 1;
}

int rsh_help(char **args) {

    int i;

    printf("The basicC_shell\n");
    printf("Type the program names arguments and hit enter\n");
    printf("The following are nuilt in:\n");

    for(i=0;i<rsh_num_builtins();i++){
        printf(" %s\n",builtin_str[i]);
    }

    return 1;
}

int rsh_exit(char **args) {
    return 0;
}

int rsh_pwd(char **args) {

    if(args[1] != NULL) {
        
        fprintf(stderr,"Usage : pwd\n");

    } else {

        char buf[1024];

        if ((getcwd(buf,sizeof(buf))) != NULL) {
            printf("%s\n",buf);
        } else {
            perror("rsh");
        }

    }

    return 1;
}

int rsh_echo(char **args) {

    if(args[1] == NULL) {
        fprintf(stderr,"rsh: expected argument to echo\n");
    } else {

        int i=1;

        while(args[i]!=NULL) {
            printf("%s ",args[i++]);
        }
        printf("\n");
    }

    return 1;
}

int rsh_type(char **args) {

    if (args[1]==NULL) {
        fprintf(stderr,"rsh : argument expected\n");
        return -1;
    } 

    bool command_found = false;

    for(int i=0;i<rsh_num_builtins();i++) {
        if(strcmp(args[1],builtin_str[i]) == 0) {
            printf("%s\n",builtin_type[i]);
            command_found = true;
            break;
        }
    }

    if(!command_found) {
        fprintf(stderr,"Command not found\n");
        return -1;
    }

    return 1;
}

int rsh_export(char **args) {
    
    if(args[1] == NULL || args[2]==NULL) {
        fprintf(stderr,"rsh:expected argument\n");
    } else {

        if(setenv(args[1],args[2],0) != 1) {
            perror("rsh\n");
        }
    }

    return 1;
}


/*Execute function*/
int rsh_execute(char **args) {

    if (args[0] == NULL) {
        return 1;
    }

    for (int i=0;i<rsh_num_builtins();i++) {

        if (strcmp(args[0],builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }

    }

    return rsh_launcher(args);
}

/*Prompt features implementation*/
int prompt_printer() {
    username_prompt_printer();
    printf("@");
    hostname_prompt_printer();
    printf("::");
    dir_prompt_printer();
    printf("~$");
}

void dir_prompt_printer() {
    char buf[1024];

    if ((getcwd(buf,sizeof(buf))) != NULL) {
        printf("%s",buf);
    }
}

void username_prompt_printer() {

    struct passwd *pw = getpwuid(geteuid());

    if (pw) {
        printf("%s", pw->pw_name);
    }
}

void hostname_prompt_printer(){
    char buf[1024];

    if(gethostname(buf, 1024) == 0) {
        printf("%s",buf);
    }
}
  