#include <stdlib.h>
#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/*User input command control functions declaration*/
void lsh_loop(void);
char* lsh_read_line(void);
char** lsh_split_line(char *line);

/*Command execution function decalration*/
int lsh_execute(char** args);

void free(char* line);
void free(char** args);

/*Built in commands function declaration*/

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*List of built in commands and corresponding functions*/

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*buitin_func[]) (char*) {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

/*main function : */

int main(int argc, char**argv){

    lsh_loop();

    return EXIT_SUCCESS;

}

/*Function defenitions*/
/*User input command control functions defenition*/

void lsh_loop(void){

    char *line;
    char **args;
    int status;

    do{

        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);

    } while(status);


}

char  *lsh_read_line(void){

    int bufsize = LSH_RL_BUFSIZE;
    int position  = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "lsh : allocation error\n");
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
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer) {
                fprintf(stderr, "lsh : allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

    }
}

//Implement the previous part simply using getline ! : Note to dev!

char **lsh_split_line(char *line) {

    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if(!tokens) {
        fprintf(stderr, "lsh :allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position>=bufsize){
            bufsize+=LSH_TOK_BUFSIZE;
            tokens = realloc(tokens,bufsize*sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "lsh : allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL,LSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int lsh_launcher(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid==0) {
        //Child process
        if(execvp(args[0],args) == -1){
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if(pid<0){
        //fork failure
        perror("lsh");
    } else {
        //Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}






  
  