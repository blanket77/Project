///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/30 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-3 ( ftp server ) //
// Description : server에 변환된 명령어 보냄. 그리고 data connection을 할 땐 그 때 마다 10001~30000의 임의의 포트로 함
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define MAX_BUF 1024

///////////////////////////////////////////////////////////////////////
// convert_addr_to_str //
// ================================================================= //
// Input: unsigned long ip_addr -> IP address to convert, //
// unsigned int port -> Port number to convert //
// (Input parameter Description) //
// Output: char* -> Formatted string representing the IP and port in //
// FTP command format //
// (Output parameter Description) //
// Purpose: Converts an IP address and port number into a string //
// formatted according to FTP command standards //
///////////////////////////////////////////////////////////////////////
char* convert_addr_to_str(unsigned long ip_addr, unsigned int port);

///////////////////////////////////////////////////////////////////////
// conv_cmd //
// ================================================================= //
// Input: char* buff -> Buffer containing the user input command, //
// char* cmd_buff -> Buffer to store the converted FTP command //
// (Input parameter Description) //
// Output: int - 1 success //
// 0 fail //
// (Output parameter Description) //
// Purpose: Converts a Unix-style command into an FTP command //
///////////////////////////////////////////////////////////////////////
int conv_cmd(char* buff,char* cmd_buff);

int count = 0;

int main(int argc, char **argv)
{
    int sockfd;
    char *hostport;
    struct sockaddr_in temp, servaddr;
    int listenfd, connfd;
    char buffer[1024];
    char command[100];
    char cmd_buff[MAX_BUF]= {'\0'};
    //sockf TCP로 만들어서 sockfd로 만들고 여기다가 클라이언트의 정보를 넣는다.
    if( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("can't create socket.\n");
        return -1;
    }

    bzero((char*)&temp, sizeof(temp)); // server_addr 구조체를 0으로 초기화
    temp.sin_family = AF_INET; // 주소 체계를 IPv4로 설정
    temp.sin_addr.s_addr = inet_addr(argv[1]); // IP 주소 설정
    temp.sin_port = htons(atoi(argv[2])); // 포트 번호를 네트워크 바이트 순서로 변환하여 설정

    // 서버 연결 시도
    if(connect(sockfd, (struct sockaddr*)&temp, sizeof(temp)) < 0){
        perror("Connect fail");
        close(sockfd);
        return 1;
    }

    //소켓이 열려있고 server와 연결이 되어있으면 밑에 코드 작동
    for (;;){
        write(STDOUT_FILENO, "> ", 2);
        memset(buffer, '\0', sizeof(buffer));
        memset(cmd_buff, '\0', sizeof(cmd_buff));
        //실행 파일을 제외한 입력을 띄어쓰기로 받는다.
        read(STDIN_FILENO, buffer, sizeof(buffer));
        for(int i = 0; i < MAX_BUF; i++){
            if(cmd_buff[i] == '\n') 
                cmd_buff[i] ='\0';
        } //개행문자가 있으면 지운다.

        if (conv_cmd(buffer, cmd_buff) < 0) //리눅스 명령어(buff)를 FTP(cmd_buff)로 바꿔준다. 
        {
            write(STDERR_FILENO, "conv_cmd() error!!\n", sizeof("conv_cmd() error!!\n"));
            exit(1);  //프로그램 종료
        }

        srand(time(NULL));
        
        /* make control connection and data connection */
        unsigned int data_port = 10001 + ( rand() % 20000 );

        // printf("%d\n",data_port);
        struct sockaddr_in servaddr, cliaddr;

        listenfd = socket(AF_INET, SOCK_STREAM, 0); //sock을 만든다.
        
        bzero((char*)&servaddr, sizeof(servaddr)); // 서버 주소 구조체를 초기화한다.
        servaddr.sin_family = AF_INET; //주소 체계를 인터넷 프로토콜로 설정
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP주소를 설정 
        servaddr.sin_port = htons(data_port);// 포트 번호를 입력으로 받아온다.


        if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){ //sock과 server 관련 정보를 합친다.
            printf("Server: Can't bind local address.\n");
            return 1;
        }

        int clilen = sizeof(cliaddr);
        listen(listenfd, 5);
        int client_len = sizeof(cliaddr);

        bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화
        hostport = convert_addr_to_str(temp.sin_addr.s_addr, servaddr.sin_port);
        printf("converting to %s\n", hostport);
        
        bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화
        snprintf(buffer, MAX_BUF, "PORT %s", hostport);
        write(sockfd, buffer, MAX_BUF); // PORT 명령어 client한테 보냄

        // //여기서 server처럼 행동 
        
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &client_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }
        bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화

        write(sockfd, cmd_buff, MAX_BUF); //command 전달
        count += read(sockfd, buffer, MAX_BUF); //buffer에 server 결과값 저장
        printf("%s", buffer);

        if (strncmp(cmd_buff, "NLST", 4) == 0){
            count += read(sockfd, buffer, MAX_BUF); //buffer에 server 결과값 저장
            printf("%s", buffer);
            count += read(sockfd, buffer, MAX_BUF); //buffer에 server 결과값 저장
            printf("%s", buffer);
            count += read(sockfd, buffer, MAX_BUF); //buffer에 server 결과값 저장
            printf("%s", buffer);
        }
        // write(STDOUT_FILENO, buffer, MAX_BUF); //출력
        else if (strncmp(cmd_buff, "QUIT", 4) == 0){

            count += read(sockfd, buffer, MAX_BUF); //buffer에 server 결과값 저장
            printf("%s", buffer);

            close(sockfd);
            close(connfd);
            close(listenfd);
            return 0;
        }
        else{}
        printf("OK. %d bytes is received\n", count);

        count = 0;
        close(connfd);
        close(listenfd);
    }
    close(sockfd);
    
    return 0;
}

char* convert_addr_to_str(unsigned long ip_addr, unsigned int port) {
    char *addr = malloc(30); // Allocate memory dynamically
    unsigned char bytes[4]; // IP 주소 바이트를 저장

    // 네트워크 바이트 순서의 IP 주소를 각 바이트로 분리
    bytes[3] = (ip_addr >> 24) & 0xFF; // 가장 상위 바이트
    bytes[2] = (ip_addr >> 16) & 0xFF;
    bytes[1] = (ip_addr >> 8) & 0xFF;
    bytes[0] = ip_addr & 0xFF; // 가장 하위 바이트

    // 포매팅된 문자열 생성
    sprintf(addr, "%d,%d,%d,%d,%d,%d", bytes[0], bytes[1], bytes[2], bytes[3], ntohs(port) / 256, ntohs(port) % 256);

    // 정적 버퍼의 주소 반환
    return addr;
}


int conv_cmd(char* buff,char* cmd_buff){
    char *ptr= strtok(buff, " "); //buff에 있는 리눅스 명령어를 FTP 명령어로 바꾼다.
    if(ptr == NULL)
        return -1;

    if( strncmp(ptr, "ls", (int)strlen("ls")) == 0){ //ls 명령어가 아니면 오류를 보내게 한다.
        strcpy(cmd_buff, "NLST"); //맞으면 ls부분을 NLST로 바꾼다.
    }
    else if(strncmp(ptr, "quit", (int)strlen("quit") ) == 0) {
        strcpy(cmd_buff, "QUIT"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else{
        return -1;
    }

    return 1;
}
