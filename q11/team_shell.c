#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/types.h> 
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#define MAX_CMD_ARG 	10
#define MAX_CMD_LIST 	10
#define MAX_CMD_GRP	10

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

const char *prompt = "Ahn_Son"; // 팀 쉘 이름

char* cmdgrp[MAX_CMD_GRP];
char* cmdlist[MAX_CMD_LIST];
char* cmdargs[MAX_CMD_ARG];
char cmdline[BUFSIZ];

void fatal(char *str);
void parse_redirect(char* cmd);
void execute_cmd(char *cmdlist);
void execute_cmdline(char* cmdline);
void execute_cmdgrp(char* cmdgrp); 
void zombie_handler(int signo);

int ahnmkdir(char *s, const char *delimiters, char** argvp, int MAX_LIST); // mkdir : 안승주

struct sigaction act;  // 구조체 선언
static int status;
static int IS_BACKGROUND=0;

typedef struct { // 커맨드 구조체
    char* name;
    char* desc;
    int ( *func )( int argc, char* argv[] ); // 함수의 포인터 
} COMMAND;

int son_cd( int argc, char* argv[] ){   // cd : 손건 작성
    if( argc == 1 )
        chdir( getenv( "HOME" ) );  // getenv : 환경변수에 저장된 값을 읽어옴
    else if( argc == 2 ){
        if( chdir( argv[1] ) )
            printf( "Directory not found!!!\n" );
    }
    else
        printf( "USAGE: cd [dir]\n" );
    return TRUE;
}

int my_exit( int argc, char* argv[] ){   // 1번 exit으로 프로그램 마침.
    printf("End Shell!!!\n");
    exit(0);
    return TRUE;
}

static COMMAND builtin_cmds[] = // 커맨드  #include <unistd.h> 
{
    { "cd", "change directory", son_cd },
    { "exit", "exit this shell", my_exit },
    { "quit", "quit this shell", my_exit },
};

int main(int argc, char**argv)   // shell에서 signal의 사용
{
    int i;
    sigset_t set;

    sigfillset(&set);
    sigdelset(&set,SIGCHLD);
    sigprocmask(SIG_SETMASK,&set,NULL);

    act.sa_flags = SA_RESTART;
    sigemptyset(&act.sa_mask);
    act.sa_handler = zombie_handler;
    sigaction(SIGCHLD, &act, 0);

    while (1) {
        fputs(prompt, stdout);
        printf( "[%s] $>>> ", get_current_dir_name() ); 
        fgets(cmdline, BUFSIZ, stdin);
        cmdline[ strlen(cmdline) -1] ='\0'; 
        execute_cmdline(cmdline);
    }
    return 0;
}

void zombie_handler(int signo)  //  쉘 프로세스에서는 이 시그널을 받으면 핸들러로 돌려서 동료되지 않게끔 설정
{
    pid_t pid ;
    int stat ;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated normaly\n", pid) ;
}

void fatal(char *str)
{
    perror(str);
    exit(1);
}

int ahnmkdir(char *s, const char *delimiters, char** argvp, int MAX_LIST)  // mkdir
{
    int i = 0;
    int num_token = 0;
    char *newfolder = NULL;

    if( (s==NULL) || (delimiters==NULL) )
    {
        return -1;
    }

    newfolder = s+strspn(s, delimiters);

    argvp[num_token]=strtok(newfolder, delimiters);

    if(argvp[num_token] !=NULL)
        for(num_token=1; (argvp[num_token]=strtok(NULL, delimiters)) != NULL; num_token++)
        {
            if(num_token == (MAX_LIST-1)) return -1;
        }

    if( num_token > MAX_LIST) return -1;

    return num_token;
}

void parse_redirect(char* cmd) // 4번 재지향 파트
{
    char *arg;
    int mylength = strlen(cmd);
    int fd, i;

    for(i = mylength-1;i >= 0;i--)
    {
        switch(cmd[i])
        {
            case '<':  // fd가 0보다 크다면, 파일이 올바르게 열린 것 , 0644는 file의 소유자만 읽기/쓰기가 가능하며 그 외에는 읽기만 가능한 MODE.
                arg = strtok(&cmd[i+1], " \t");
                if( (fd = open(arg, O_CREAT | O_WRONLY, 0644)) < 0)
                    fatal("file open error\n");
                dup2(fd, STDIN_FILENO);
                close(fd);
                cmd[i] = '\0';
                break;
            case '>': // fd가 0보다 작다면, 파일이 올바르게 열린 것 , 0644는 file의 소유자만 읽기/쓰기가 가능하며 그 외에는 쓰기만 가능한 MODE.
                arg = strtok(&cmd[i+1], " \t");
                if( (fd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
                    fatal("file open error");
                dup2(fd, STDOUT_FILENO);
                close(fd);
                cmd[i] = '\0';
                break;
            default:break;
        }
    }

}

int run_background(char *cmd) // 2번 '&' 입력시 백그라운드로 실행
{
    int i;
    for(i=0; i < strlen(cmd); i++)
        if(cmd[i] == '&')
        {
            cmd[i] = ' ';
            return 1;
        }
    return 0;
}

void execute_cmd(char *cmdlist)
{
    parse_redirect(cmdlist);

    if(ahnmkdir(cmdlist, " \t", cmdargs, MAX_CMD_ARG) <= 0)
        fatal("ahnmkdir_cmdargs error");

    execvp(cmdargs[0], cmdargs);
    fatal("exec error");
}

void execute_cmdgrp(char *cmdgrp) // 4번 파이프 파트
{
    int i=0;
    int count = 0;
    int pfd[2];
    sigset_t set;

    setpgid(0,0);
    if(!IS_BACKGROUND)
        tcsetpgrp(STDIN_FILENO, getpid());

    sigfillset(&set);
    sigprocmask(SIG_UNBLOCK,&set,NULL);

    if((count = ahnmkdir(cmdgrp, "|", cmdlist, MAX_CMD_LIST)) <= 0)
        fatal("ahnmkdir_cmdgrp error");

    for(i=0; i<count-1; i++)
    {
        pipe(pfd);
        switch(fork())
        {
            case -1: fatal("fork error");
            case  0: close(pfd[0]);
                     dup2(pfd[1], STDOUT_FILENO);
                     execute_cmd(cmdlist[i]);
            default: close(pfd[1]);
                     dup2(pfd[0], STDIN_FILENO);
        }
    }
    execute_cmd(cmdlist[i]);

}

void execute_cmdline(char* cmdline)
{
    int count = 0;
    int i=0, j=0, pid;
    char* myvector[MAX_CMD_ARG];
    char mygrptemp[BUFSIZ];
    int num_token = 0;

    count = ahnmkdir(cmdline, ";", cmdgrp, MAX_CMD_GRP);

    for(i=0; i<count; ++i)
    {
        memcpy(mygrptemp, cmdgrp[i], strlen(cmdgrp[i]) + 1);
        num_token = ahnmkdir(cmdgrp[i], " \t", myvector, MAX_CMD_GRP);

        for( j = 0; j < sizeof( builtin_cmds ) / sizeof( COMMAND ); j++ ){	if( strcmp( builtin_cmds[j].name, myvector[0] ) == 0 ){
            builtin_cmds[j].func( num_token , myvector );
            return;
        }
        }

        IS_BACKGROUND = run_background(mygrptemp);

        switch(pid=fork())
        {
            case -1:
                fatal("fork error");
            case  0:
                execute_cmdgrp(mygrptemp);
            default:
                if(IS_BACKGROUND) break;
                waitpid(pid, NULL, 0);
                tcsetpgrp(STDIN_FILENO, getpgid(0));
                fflush(stdout);
        }
    }

}
int SetSignal(struct sigaction *def, sigset_t *mask, void(*handler)(int)) { // 3번 인터럽트키 파트
// SIGINT: Ctrl-C, SIGQUIT: Ctrl-Z
    struct sigaction catch;
    catch.sa_handler = handler;
    def->sa_handler = SIG_DFL;
    catch.sa_flags = 0;
    def->sa_flags = 0;

    if ((sigemptyset(&(def->sa_mask)) == -1) || (sigemptyset(&(catch.sa_mask)) == -1) || (sigaddset(&(catch.sa_mask), SIGINT) == -1) || (sigaddset(&(catch.sa_mask), SIGQUIT) == -1) || (sigaction(SIGINT, &catch, NULL) == -1) || (sigaction(SIGQUIT, &catch, NULL) == -1) || (sigemptyset(mask) == -1) || (sigaddset(mask, SIGINT) == -1) || (sigaddset(mask, SIGQUIT) == -1))

        return -1;
    return 0;
}
