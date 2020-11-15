#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHMSIZE 128
#define SHMNAME "/sh_mem"
#define BUFSIZE 256
#define SOURCE_FILE "originalfile.txt"
#define TARGET_FILE "copy.txt"

int main(){
    int *val;
    struct timespec *tms;
    pid_t pid;
    void *shmaddr;
    int shmd;
    FILE *file;

    //공유 메모리 생성부분
    if((shmd = shm_open(SHMNAME, O_CREAT | O_RDWR, 0666)) == -1){
        perror("shm_open");
        exit(1);
    }
    if(ftruncate(shmd, SHMSIZE) != 0){
        perror("ftruncate");
        exit(1);
    }
    if((shmaddr = mmap(NULL, SHMSIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shmd, 0)) == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    //공유 메모리 생성부분

    //프로세스 생성
    pid = fork();

    if(pid == 0){
        //파일 열기
        if((file = fopen(SOURCE_FILE, "r")) == NULL){
            perror("fopen");
            exit(1);
        }
        char read_buf[BUFSIZE];//입출력 버퍼
        
        fgets(read_buf, BUFSIZE, file);
        strcpy((char *) shmaddr, read_buf);
        
        fclose(file);
    }
    else if (pid > 0){
        sleep(1);
        file = fopen(TARGET_FILE, "w");
        fputs(shmaddr, file);   
        
        if (shm_unlink(SHMNAME) == -1) {
            perror ("shm_unlink");
            exit (1);
        }
    }
    else{
        perror("fork");
        exit(1);
    }
    close(shmd);
    
    return 0;
}
