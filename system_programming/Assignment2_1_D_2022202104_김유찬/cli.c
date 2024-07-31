///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/02 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 ( ftp server ) //
// Description : client에서 온 명령어를 FTP 명령어 변환하여 server에 보내고 //
//               server에서 온 출력 결과를 출력한다.
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_BUFF 10000
#define RCV_BUFF 10000

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
    int sockfd; //클라이언트 네트워크 엔드포인트 파일 디스크립터
    char buff[MAX_BUFF] = {'\0'}, cmd_buff[MAX_BUFF]= {'\0'}, rcv_buff[RCV_BUFF] = {'\0'}; //입력에 담은 buffer, buffer에 담긴거 변환해서 담는 cmd_buffer, server에게 보낼 rcv_buff
    int n; // 숫자를 잠시 담을 변수
    struct sockaddr_in server_addr; //소켓 
    char* haddr = argv[1];
    int portno = atoi(argv[2]);
    char message[100];
    int str_len;

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
            write(STDOUT_FILENO, "Program quit!!\n", sizeof("Program quit!!\n"));
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
        while(ptr != NULL){
            ptr = strtok(NULL, " ");
            if (ptr == NULL) //널문자를 만나면 빠져나온다.
                break;
            
            strcat(cmd_buff, " "); // 한칸 띄우고
            strcat(cmd_buff, ptr); // 그 다음 문자열을 넣는다.
        }
        return 1;
    }

    else if(strncmp(ptr, "quit", (int)strlen("quit") ) == 0) {
        strcpy(cmd_buff, "QUIT"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
        while(ptr != NULL){
            ptr = strtok(NULL, " ");
            if (ptr == NULL) //널문자를 만나면 빠져나온다.
                break;
            
            strcat(cmd_buff, " "); // 한칸 띄우고
            strcat(cmd_buff, ptr); // 그 다음 문자열을 넣는다.
        }
        return 1;
    }
    else{
        return -1;
    }
    return -1;
}

//서버에게 받은 rcv_buff 내용을 출력하는 함수
void process_result(char* rcv_buff){
    //100씩 끊어서 출력한다. 왜냐면 100씩 끊어서 서버에서 보냈기 때문이다.
    for(int i = 0 ; i < RCV_BUFF; i+=100){
        write(STDOUT_FILENO, rcv_buff+i , 100);
    }
}