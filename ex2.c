/* Imports */
#include <stdio.h>
#include <string.h>
#include<sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

/* Defines */
#define MAX_PROCESSES 100
#define CD_MAX_ARGUMENTS 2
#define HISTORY_MAX_ARGUMENTS 1
#define JOBS_MAX_ARGUMENTS 1
#define system_call void
#define CMD_SIZE 1024
#define bool int
#define True 1
#define False 0
#define TIMER 100
#define SYS_CALL_ERR "Error in system call"
#define FORK_ERR "Failed to fork"
#define BAD_DIR_ERR "Error: No such file or directory"
#define TOO_MANY_ARGS_ERR "Error: Too many arguments"

/* Global Variables */
struct Stack *jobs_stack;
char user[1024];
char jobs_init[1024];
char directory_log[1024];
char sys_call_err_log[1024];
char history_log[1024];
char jobs_log[1024];
pid_t processes_pid[MAX_PROCESSES];
char *processes_task[MAX_PROCESSES];
pid_t jobs_pid[MAX_PROCESSES];
char *jobs_task[MAX_PROCESSES];
bool cd_cmd = False, history_cmd = False, jobs_cmd = False, exit_cmd = False;
int status, process_index = 0, jobs_index = 0;
pid_t child_pid;
char user_input_buffer[CMD_SIZE];
char *args[CMD_SIZE];
char *command = NULL;

/* Global Functions */
struct Stack {
    pid_t top;
    unsigned capacity;
    pid_t array[100];
    char *name_array[100];
};
struct Tuple {
    pid_t pid;
    char *name;
};

struct Stack *createStack(unsigned capacity) {
    struct Stack *stack = (struct Stack *) malloc(sizeof(struct Stack));
    stack->capacity = capacity;
    stack->top = -1;
    return stack;
}

// Stack is full when top is equal to the last index
int isFull(struct Stack *stack) {
    return stack->top == stack->capacity - 1;
}

// Stack is empty when top is equal to -1
int isEmpty(struct Stack *stack) {
    return stack->top == -1;
}

// Function to add an item to stack.  It increases top by 1
void push(struct Stack *stack, int item, char *name) {
    if (isFull(stack))
        return;
    stack->array[stack->top + 1] = item;
    stack->name_array[stack->top + 1] = (char *) malloc(100 * sizeof(char));
    strcpy(stack->name_array[stack->top + 1], name);
    stack->top++;
    char cmd[100];
    sprintf(cmd, "echo %d > /home/$USER/dummyS/jobs_logs.txt", stack->top);
    system(cmd);

}

// Function to remove an item from stack.  It decreases top by 1
void pop(struct Stack *stack) {
    if (isEmpty(stack))
        return;
    stack->array[stack->top] = 0;
    stack->array[stack->top - 1];
    stack->name_array[stack->top] = NULL;
    stack->name_array[stack->top - 1];
    stack->top--;
    char cmd[100];
    sprintf(cmd, "echo %d > /home/$USER/dummyS/jobs_logs.txt", stack->top);
    system(cmd);

}

// Function to return the top from stack without removing it
struct Tuple *peek(struct Stack *stack) {
    struct Tuple *data = (struct Tuple *) malloc(sizeof(struct Tuple));
    if (isEmpty(stack))
        return data;

    data->pid = stack->array[stack->top];
    data->name = stack->name_array[stack->top];
    return data;
}

int q_run_in_background() {
    if (user_input_buffer[strlen(user_input_buffer) - 1] == '&' &&
        user_input_buffer[strlen(user_input_buffer) - 2] == ' ')
        return 1;
    return 0;
}

void remove_process() {
    process_index--;
    processes_pid[process_index % MAX_PROCESSES] = 0;
    processes_task[process_index % MAX_PROCESSES] = NULL;
}

void save_process() {
    processes_pid[process_index % MAX_PROCESSES] = child_pid;
    char *task = (char *) malloc(sizeof(char) * strlen(user_input_buffer));
    strcpy(task, user_input_buffer);
    processes_task[process_index % MAX_PROCESSES] = task;
    process_index++;
}

bool valid_arguments(int num) {
    int counter = 0;
    while (args[counter] != NULL) {
        counter++;
    }
    if (counter > num)
        return False;
    return True;
}

void print_error(char *str) {
    printf("\033[0;31m%s\n\033[0m", str);
    exit(15);
}

bool dir_exists(char *my_dir) {
    DIR *dir = opendir(my_dir);
    if (dir) {
        closedir(dir);
        return True;
    } else {
        return False;
    }
}

system_call cd() {
    if (!valid_arguments(CD_MAX_ARGUMENTS)) {
        print_error(TOO_MANY_ARGS_ERR);
    }
    FILE *fp;
    fp = fopen(directory_log, "rw");
    if (fp == NULL) {
        print_error("Cannot find directory_log.txt");
    } else if (strcmp(args[1], "~") == 0) {
        system("echo /home > /home/$USER/dummyS/directory_log.txt");
        system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
    } else if (strcmp(args[1], "/") == 0) {
        system("echo / > /home/$USER/dummyS/directory_log.txt");
        system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
    } else if (strcmp(args[1], "-") == 0) {
        FILE *f;
        f = fopen(directory_log, "r");
        if (f == NULL) {
            print_error("Cannot open directory_log");
        }
        char line[1024];
        int i = 0;
        while (fgets(line, sizeof(line), f)) {
            i++;
        }
        char *var;
        var = strtok(line, "\n");
        char sys_cmd[500];
        sprintf(sys_cmd, "echo %s > /home/$USER/dummyS/directory_log.txt", var);
        system(sys_cmd);
        system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
        fclose(f);
    } else if (strcmp(args[1], "..") == 0) {
        FILE *f;
        f = fopen(directory_log, "r");
        if (f == NULL) {
            print_error("Cannot open directory_log");
        }
        char line[1024];
        fgets(line, sizeof(line), f);
        char *var;
        var = strtok(line, "\n");
        int i = 1;
        unsigned long len = strlen(var);
        while (var[len - i] != '/' && len - i >= 0) {
            var[len - i] = '\0';
            i++;
        }
        if (len - i != 0) {
            var[len - i] = '\0';
        }
        char sys_cmd[500];
        sprintf(sys_cmd, "echo %s > /home/$USER/dummyS/directory_log.txt", var);
        system(sys_cmd);
        system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
        fclose(f);
    } else if (strcmp(args[1], ".") == 0) {
        system("echo $PWD > /home/$USER/dummyS/directory_log.txt");
        system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
    } else {
        char dirName[1024];
        strcpy(dirName, args[1]);
        if (dirName[0] == '/') {
            if (dir_exists(dirName)) {
                char sys_cmd[500];
                sprintf(sys_cmd, "echo %s > /home/$USER/dummyS/directory_log.txt", dirName);
                system(sys_cmd);
                system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
            } else {
                print_error(BAD_DIR_ERR);
            }
        } else {
            if (dir_exists(dirName)) {
                FILE *f;
                f = fopen(directory_log, "r");
                if (f == NULL) {
                    print_error("Cannot open directory_log");
                }
                char current_dir[1024];
                char *temp;
                fgets(current_dir, sizeof(current_dir), f);
                temp = strtok(current_dir, "\n");
                strcpy(current_dir, temp);
                if (strlen(current_dir) > 1) {
                    strcat(current_dir, "/");
                }
                strcat(current_dir, dirName);
                fclose(f);
                char sys_cmd[500];
                sprintf(sys_cmd, "echo %s > /home/$USER/dummyS/directory_log.txt", current_dir);
                system(sys_cmd);
                system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
            } else {
                print_error(BAD_DIR_ERR);
            }
        }
    }
    fclose(fp);
    exit(0);
}

system_call history(int x) {
    int i = 0;
    if (!valid_arguments(HISTORY_MAX_ARGUMENTS)) {
        print_error(TOO_MANY_ARGS_ERR);
    }
    while (processes_pid[i] != 0 && i < MAX_PROCESSES) {
        char *task = processes_task[i];
        if (!(task[strlen(task) - 1] == '&' && task[strlen(task) - 2] == ' ')) {
            printf("%d %s DONE\n", processes_pid[i], processes_task[i]);
        } else {
            bool found = False;
            FILE *fp;
            system("ps x > /home/$USER/dummyS/history_log.txt");
            fp = fopen(history_log, "r");
            if (fp == NULL) {
                print_error("Cannot open history_log.txt");
            }
            char line[1024];
            char word[1024];
            while (fgets(line, sizeof(line), fp)) {
                if (found)
                    break;
                sprintf(word, "%d", processes_pid[i]);
                if (strstr(line, word)) {
                    found = True;
                }
            }
            fclose(fp);
            if (found) {
                printf("%d %s RUNNING\n", processes_pid[i], processes_task[i]);
            } else {
                printf("%d %s DONE\n", processes_pid[i], processes_task[i]);
            }
        }
        i++;
    }
    if (x == 0) {
        printf("%d history RUNNING\n", getpid());
    } else {
        printf("%d history & RUNNING\n", getpid());
    }
    exit(0);
}


system_call jobs() {
    if (!valid_arguments(JOBS_MAX_ARGUMENTS)) {
        print_error(TOO_MANY_ARGS_ERR);
    }
    struct Stack *temp_stack = createStack(100);
    while (!isEmpty(jobs_stack)) {
        struct Tuple *item = peek(jobs_stack);
        char temp_for_print[100];
        strcpy(temp_for_print,item->name);
        char *name;
        name = strtok(temp_for_print,"&");
        printf("%d  %s\n", item->pid, name);
        pop(jobs_stack);
        push(temp_stack, item->pid, item->name);
    }
    system("ps x > /home/$USER/dummyS/history_log.txt");
    FILE *fp;
    while (!isEmpty(temp_stack)) {
        struct Tuple *item = peek(temp_stack);
        bool found = False;

        fp = fopen(history_log, "r");
        if (fp == NULL) {
            print_error("Cannot open history_log.txt");
        }
        char line[1024];
        char word[1024];
        while (fgets(line, sizeof(line), fp)) {
            if (found)
                break;
            sprintf(word, "%d", item->pid);
            if (strstr(line, word)) {
                found = True;
            }
        }
        fclose(fp);
        if (found) {
            push(jobs_stack, item->pid, item->name);
        }
        pop(temp_stack);
    }
    int i = 0;
    while (i<=jobs_stack->top) {
        char cmd[100];
        char name[100];
        strcpy(name,strtok(jobs_stack->name_array[i],"&"));
        sprintf(cmd, "echo %d %s >> /home/$USER/dummyS/jobs_init.txt", jobs_stack->array[i],
                name);
        system(cmd);
        i++;
    }
    exit(0);
}

bool child() {
    if (child_pid == 0)
        return True;
    return False;
}

bool failed_fork() {
    if (child_pid < 0)
        return True;
    return False;
}

bool parent() {
    if (child_pid > 0)
        return True;
    return False;
}

void parse(char *str) {
    char temp_buffer[CMD_SIZE];
    strcpy(temp_buffer, str);
    int i = 0;
    char *token;
    token = strtok(temp_buffer, " ");
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    command = args[0];
    if (strcmp("cd", command) == 0) {
        cd_cmd = True;
    } else if (strcmp("jobs", command) == 0) {
        jobs_cmd = True;
    } else if (strcmp("history", command) == 0) {
        history_cmd = True;
    } else if (strcmp("exit", command) == 0) {
        exit_cmd = True;
    }
    if (strcmp(args[i - 1], "&") == 0) {
        args[i - 1] = NULL;
    }
    i = 0;
    int j = 0;
    if (strcmp(command, "echo") == 0) {
        while (args[i] != NULL) {
            int h = 0;
            char *word = args[i];
            unsigned long len = strlen(word);
            char *new_word = (char *) malloc(sizeof(char) * len);
            for (j = 0; j < len; j++) {
                if (word[j] != '\"') {
                    new_word[h] = word[j];
                    h++;
                }
            }
            args[i] = new_word;
            i++;
        }
    }
}

void run_program_in_foreground() {
    child_pid = fork();
    if (parent()) {
        if (q_run_in_background()) {
            char n[1024];
            strcpy(n, user_input_buffer);
            push(jobs_stack, child_pid, n);
        }
        if (strcmp(command, "history") != 0 && strcmp(command, "jobs") != 0) {
            printf("%d\n", child_pid);
        }
        wait(&status);
        save_process();
        FILE *pFile;
        int c;
        pFile = fopen(sys_call_err_log, "r");
        if (pFile != NULL) {
            c = fgetc(pFile);
            if (c == 48) {
                remove_process();
                pop(jobs_stack);
                system("echo 1 > /home/$USER/dummyS/sys_call_err_log.txt");
            }
            fclose(pFile);
        }
    }
    if (failed_fork())
        print_error(FORK_ERR);
    else if (child()) {
        //usleep(TIMER);
        if (cd_cmd) {
            cd();
        } else if (jobs_cmd) {
            jobs();
        } else if (history_cmd) {
            history(0);
        } else if (exit_cmd) {
            exit(0);
        }
        execvp(command, args);
        system("echo 0 > /home/$USER/dummyS/sys_call_err_log.txt");
        print_error(SYS_CALL_ERR);
    }
    if(parent()&&strcmp(command,"jobs")==0){
        jobs_stack = createStack(100);
        FILE *fp;
        fp = fopen(jobs_init,"r");
        if(fp == NULL){
            print_error("Error opening jobs_init.txt");
        }
        pid_t t ;
        char line[1024];
        char *token;
        while(fgets(line,1024,fp)){
            char name[100];
            bool first = True;
            if(strcmp(line,"\n")==0){
                continue;
            }
            token = strtok(line," ");
            while(token != NULL){
                if(first){
                    first = False;
                    t = atoi(token);
                }else{
                    strcat(name, token);
                    strcat(name," ");
                }
                token = strtok(NULL," ");
            }
            strcpy(name,strtok(name,"\n"));
            push(jobs_stack,t,name);
            strcpy(name,"");
        }
        system("rm /home/$USER/dummyS/jobs_init.txt");
        system("touch /home/$USER/dummyS/jobs_init.txt");
    }
}

void run_program_in_background() {
    child_pid = fork();
    if (parent()) {
        char *n = user_input_buffer;
        push(jobs_stack, child_pid, n);
        if (strcmp(command, "history &") != 0 && strcmp(command, "jobs &") != 0) {
            printf("%d\n", child_pid);
        }
        save_process();
        //usleep(100000);
        FILE *pFile;
        int c;
        pFile = fopen(sys_call_err_log, "r");
        if (pFile != NULL) {
            c = fgetc(pFile);
            if (c == 48) {
                remove_process();
                pop(jobs_stack);
                system("echo 1 > /home/$USER/dummyS/sys_call_err_log.txt");
            }
            fclose(pFile);
        }
    }
    if (failed_fork())
        print_error(FORK_ERR);
    else if (child()) {
        if (cd_cmd) {
            cd();
        } else if (jobs_cmd) {
            jobs();
        } else if (history_cmd) {
            history(1);
        } else if (exit_cmd) {
            exit(0);
        }
        execvp(command, args);
        system("echo 0 > /home/$USER/dummyS/sys_call_err_log.txt");
        print_error(SYS_CALL_ERR);
    }
}

void clear() {
    cd_cmd = False;
    history_cmd = False;
    jobs_cmd = False;
    status = 0;
    FILE *fp;
    fp = fopen(jobs_log, "r");
    if (fp == NULL) {
        print_error("Error while opening jobs_logs.txt");
    }
    char temp[1024];
    fgets(temp, 1024, fp);
    jobs_stack->top = atoi(temp);
    fclose(fp);
    child_pid = 1;
    for (int i = 0; i < CMD_SIZE; i++) {
        if (user_input_buffer[i] == '\0')
            break;
        user_input_buffer[i] = '\0';
    }
    for (int i = 0; i < CMD_SIZE; i++) {
        if (args[i] == NULL)
            break;
        args[i] = NULL;
    }
}

void start_shell() {
    FILE *fp;
    fp = fopen(directory_log, "r");
    if (fp == NULL) {
        print_error("Cannot open directory_log");
    }
    char directory[1024];
    fgets(directory, 1024, fp);
    strcpy(directory, strtok(directory, "\n"));
    fclose(fp);
    chdir(directory);
    printf("\033[1;32m%s> \033[0m", directory);
    fgets(user_input_buffer, CMD_SIZE, stdin);
    parse(strtok(user_input_buffer, "\n"));
}

void get_user_name() {
    getlogin_r(user, 1024);
}

void build_log() {
    system("mkdir /home/$USER/dummyS");
    system("touch /home/$USER/dummyS/history_log.txt");
    system("touch /home/$USER/dummyS/sys_call_err_log.txt");
    system("touch /home/$USER/dummyS/directory_log.txt");
    system("touch /home/$USER/dummyS/jobs_logs.txt");
    system("touch /home/$USER/dummyS/jobs_init.txt");
    sprintf(directory_log, "/home/%s/dummyS/directory_log.txt", user);
    sprintf(history_log, "/home/%s/dummyS/history_log.txt", user);
    sprintf(sys_call_err_log, "/home/%s/dummyS/sys_call_err_log.txt", user);
    sprintf(jobs_log, "/home/%s/dummyS/jobs_logs.txt", user);
    sprintf(jobs_init, "/home/%s/dummyS/jobs_init.txt", user);

}

void remove_logs() {
    system("rm /home/$USER/dummyS/*");
    system("rmdir /home/$USER/dummyS");
}

/* Main Program */
int main(int argc, char *argv[]) {
    jobs_stack = createStack(100);
    get_user_name();
    build_log();
    system("echo $PWD > /home/$USER/dummyS/directory_log.txt");
    system("echo $PWD >> /home/$USER/dummyS/directory_log.txt");
    system("echo > /home/$USER/dummyS/history_log.txt ");
    system("echo 1 > /home/$USER/dummyS/sys_call_err_log.txt");
    system("echo -1 > /home/$USER/dummyS/jobs_logs.txt");
    while (True) {
        clear();
        //usleep(100000);

        start_shell();
        if (q_run_in_background() && strcmp(command, "sleep") == 0) {
            run_program_in_background();
        } else {
            run_program_in_foreground();
        }
        if (exit_cmd) {
            remove_logs();
            exit(0);
        }
    }
}
