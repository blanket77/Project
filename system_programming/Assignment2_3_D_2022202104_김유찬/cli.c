///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/16 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-3 ( ftp server ) //
// Description : server에 변환된 명령어 보냄, Ctnl+C 입력되면 프로세스 종료
///////////////////////////////////////////////////////////////////////

#include <stdio.h>     // 표준 입력/출력 관련 헤더 파일 포함
#include <stdlib.h>    // 표준 라이브러리 함수 사용을 위한 헤더 파일 포함
#include <unistd.h>    // 유닉스 표준 함수 정의를 위한 헤더 파일 포함
#include <arpa/inet.h> // 인터넷 주소 관련 함수 사용을 위한 헤더 파일 포함
#include <sys/types.h> // 시스템 콜을 사용하기 위한 다양한 데이터 타입 정의
#include <sys/socket.h> // 소켓 함수와 데이터 구조체 정의를 위한 헤더 파일 포함
#include <netinet/in.h> // 인터넷 주소 체계 정의를 위한 헤더 파일 포함
#include <sys/wait.h>   // 프로세스 제어 관련 함수를 위한 헤더 파일 포함
#include <signal.h>     // 시그널 처리 관련 함수를 위한 헤더 파일 포함
#include <string.h>
#define MAX_BUFF 10000
#define RCV_BUFF 10000

void sh_sigint(int); // signal handler for SIGALRM

int sockfd; // 소켓 파일 디스크립터

///////////////////////////////////////////////////////////////////////
// conv_cmd //
// ==================================================================
// Input: char* buff -> Buffer containing the original command from the client
//        char* cmd_buff -> Buffer where the converted command should be stored
// Output: int -> Returns 1 if the command conversion was successful, -1 if an error occurred
// Purpose: To convert client commands into a format that the server can process more effectively,
//          such as converting Linux system commands into FTP commands
///////////////////////////////////////////////////////////////////////
int conv_cmd(char* buff, char* cmd_buff);

///////////////////////////////////////////////////////////////////////
// process_result //
// ==================================================================
// Input: char* rcv_buff -> Buffer containing the server's response to the command
// Output: None
// Purpose: To process and potentially modify the server's response before it is sent back to the client,
//          such as formatting the output or filtering results
///////////////////////////////////////////////////////////////////////
void process_result(char* rcv_buff);


int main(int argc, char **argv)
{
    char buff[MAX_BUFF] = {'\0'}, cmd_buff[MAX_BUFF]= {'\0'}, rcv_buff[RCV_BUFF] = {'\0'}; //입력에 담은 buffer, buffer에 담긴거 변환해서 담는 cmd_buffer, server에게 보낼 rcv_buff
    int n; // 숫자를 잠시 담을 변수
    struct sockaddr_in server_addr; //소켓 
    char* haddr = argv[1];
    int portno = atoi(argv[2]);
    char message[100];
    int str_len;

    if (signal(SIGINT, sh_sigint) == SIG_ERR) {
        perror("Can't set signal handler");
        return 1;
    }

    //sockf TCP로 만들어서 sockfd로 만들고 여기다가 클라이언트의 정보를 넣는다.
    if( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("can't create socket.\n");
        return -1;
    }

    bzero(rcv_buff, sizeof(rcv_buff)); // rcv_buff 버퍼를 0으로 초기화
    bzero((char*)&server_addr, sizeof(server_addr)); // server_addr 구조체를 0으로 초기화
    server_addr.sin_family = AF_INET; // 주소 체계를 IPv4로 설정
    server_addr.sin_addr.s_addr = inet_addr(haddr); // IP 주소 설정
    server_addr.sin_port = htons(portno); // 포트 번호를 네트워크 바이트 순서로 변환하여 설정

    // 클라이언트 엔드포인트(sockfd)에 서버를 연결한다.
    if( connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("can't connect.\n"); // 안되면 -1을 반환하므로 여기서 에러가 뜬다.
        return -1;
    }

    write(STDOUT_FILENO, "> ", 2);
    //소켓이 열려있고 server와 연결이 되어있으면 밑에 코드 작동
    for (;;){
        memset(buff, '\0', sizeof(buff));
        memset(cmd_buff, '\0', sizeof(cmd_buff));
        //실행 파일을 제외한 입력을 띄어쓰기로 받는다.
        read(STDIN_FILENO, buff, sizeof(buff));
        for(int i = 0; i < MAX_BUFF; i++){
			if(cmd_buff[i] == '\n') 
                cmd_buff[i] ='\0';
		} //개행문자가 있으면 지운다.
           
        if (conv_cmd(buff, cmd_buff) < 0) //리눅스 명령어(buff)를 FTP(cmd_buff)로 바꿔준다. 
        {
            write(STDERR_FILENO, "conv_cmd() error!!\n", sizeof("conv_cmd() error!!\n"));
            exit(1);  //프로그램 종료
        }
        for(int i = 0; i < MAX_BUFF; i++){
			if(cmd_buff[i] == '\n') buff[i] ='\0';
		} //개행문자가 있으면 지운다.
        n = strlen(cmd_buff);

        // printf("%s, %d\n", cmd_buff, n);

        //서버에게 변환된 FTP 명령어(cmd_buff)를 클라이언트한테 보낸다.
        if (write(sockfd, cmd_buff, n) != n) //개행문자 때문
        {
            write(STDERR_FILENO, "write() error!!\n", sizeof("write() error!!\n"));
            exit(1);  //프로그램 종료
        }
        //서버에서 온 내용을 받는다.
        if ((n = read(sockfd, rcv_buff, sizeof(rcv_buff))) < 0)
        {
            write(STDERR_FILENO, "read() error\n", sizeof("read() error\n"));
            exit(1);  //프로그램 종료
        }
        rcv_buff[n] = '\0';
        if (!strcmp(rcv_buff, "QUIT"))
        {
            // write(STDOUT_FILENO, "Program quit!!\n", sizeof("Program quit!!\n"));
            exit(1);  //프로그램 종료
        }
        else if (!strcmp(rcv_buff, "Q"))
        {
            exit(1);  //프로그램 종료
        }
        
        process_result(rcv_buff);//ls을 결과를 물을 출력한다. 
        /*display ls (including options) command result */
        write(STDOUT_FILENO, "> ", 2);
    }
    close(sockfd);
    return 0;
}

int conv_cmd(char* buff,char* cmd_buff){
    char *ptr= strtok(buff, " "); //buff에 있는 리눅스 명령어를 FTP 명령어로 바꾼다.
    if(ptr == NULL)
        return -1;

    if( strncmp(ptr, "ls", (int)strlen("ls")) == 0){ //ls 명령어가 아니면 오류를 보내게 한다.
        strcpy(cmd_buff, "NLST"); //맞으면 ls부분을 NLST로 바꾼다.
    }

    else if(strncmp(ptr, "dir", (int)strlen("dir") ) == 0) {
        strcpy(cmd_buff, "LIST"); //맞으면 dir를 LIST로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "pwd", (int)strlen("pwd") ) == 0) {
        strcpy(cmd_buff, "PWD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "cd ..", (int)strlen("cd ..") ) == 0) {
        ptr = strtok(NULL, " ");
        strcpy(cmd_buff, "CDUP"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "cd", (int)strlen("cd") ) == 0) {
        strcpy(cmd_buff, "CWD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "mkdir", (int)strlen("mkdir") ) == 0) {
        strcpy(cmd_buff, "MKD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "delete", (int)strlen("delete") ) == 0) {
        strcpy(cmd_buff, "DELE"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "rmdir", (int)strlen("rmdir") ) == 0) {
        strcpy(cmd_buff, "RMD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "rename", (int)strlen("rename") ) == 0) {
        strcpy(cmd_buff, "RNFR "); //맞으면 quit를 QUIT로 바꾼다.
        ptr = strtok(NULL, " ");
        strcat(cmd_buff, ptr);
        strcat(cmd_buff, " ");
        strcat(cmd_buff, "RNTO ");
        ptr = strtok(NULL, " ");
        strcat(cmd_buff, ptr);
        //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "quit", (int)strlen("quit") ) == 0) {
        strcpy(cmd_buff, "QUIT"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else{
        return -1;
    }
    while(ptr != NULL){
        ptr = strtok(NULL, " ");
        if (ptr == NULL) //널문자를 만나면 빠져나온다.
            break;
        
        strcat(cmd_buff, " "); // 한칸 띄우고
        strcat(cmd_buff, ptr); // 그 다음 문자열을 넣는다.
    }
    return 1;
}

//서버에게 받은 rcv_buff 내용을 출력하는 함수
void process_result(char* rcv_buff){
    //100씩 끊어서 출력한다. 왜냐면 100씩 끊어서 서버에서 보냈기 때문이다.
    for(int i = 0 ; i < RCV_BUFF; i+=100){
        write(STDOUT_FILENO, rcv_buff+i , 100);
    }
}
//Cntl+C를 누르면 시그널 발생
void sh_sigint(int signum){
    //client한테 QUIT를 보낸다.
    write(sockfd, "QUIT", 4);
   close(sockfd);
    exit(1);
}