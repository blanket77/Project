///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/21 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-1 ( ftp server ) //
// Description : client가 server에게 아이디, 비번을 보내고 맞는지 server에게 받아서
//               client쪽에서 출력한다.
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_BUF 1024
#define CONT_PORT 20001

///////////////////////////////////////////////////////////////////////
// log_in//
// ==================================================================
// Input: int sockfd -> socket descriptor
// Output: None
// Purpose: check user and password
///////////////////////////////////////////////////////////////////////
void log_in(int sockfd);

int main(int argc, char *argv[]){
    int sockfd, n, p_pid;
    struct sockaddr_in servaddr;
    
    //sockf TCP로 만들어서 sockfd로 만들고 여기다가 클라이언트의 정보를 넣는다.
    if( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("can't create socket.\n");
        return -1;
    }

    bzero((char*)&servaddr, sizeof(servaddr)); // server_addr 구조체를 0으로 초기화
    servaddr.sin_family = AF_INET; // 주소 체계를 IPv4로 설정
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // IP 주소 설정
    servaddr.sin_port = htons(atoi(argv[2])); // 포트 번호를 네트워크 바이트 순서로 변환하여 설정

    // 클라이언트 엔드포인트(sockfd)에 서버를 연결한다.
    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("can't connect.\n"); // 안되면 -1을 반환하므로 여기서 에러가 뜬다.
        return -1;
    }
    log_in(sockfd);
    close(sockfd);
    return 0;
}

void log_in(int sockfd){
    int n;
    char user[MAX_BUF], *passwd, buf[MAX_BUF];
    
    /* 코드 작성 (hint: Check if the ip is acceptable ) */
    n = read(sockfd, buf, MAX_BUF); //IP 신호가 잘 들어왔는지 확인
    buf[n] = '\0'; //마지막에 null문자 없을 수도 있기 때문에 쓴다.
    if(!strcmp(buf, "IP OK")){
        printf("** Client is connected **\n");
    }
    else{
        printf("** Connection refused **\n");
        close(sockfd);
        exit(0);
    }

    for(;;) {
        //buf를 초기화 한다.
        bzero(buf, sizeof(buf));
        write(STDOUT_FILENO, "Input Id : ", sizeof("Input Id : "));
        read(STDIN_FILENO, buf, MAX_BUF);//buf에 password를 적는
        //개행문자를 널문자로 바꾼다.
        for(int i = 0 ; i < MAX_BUF ; i++){
            if(buf[i] == '\n') {
                buf[i] = '\0';
                break;
            }
        }
        char user[20];
        strcpy(user, buf);

        write(sockfd, buf, MAX_BUF); //server에게 보낸다.
        //buf를 초기화한다.
        bzero(buf, sizeof(buf));
        //패스워드를 적으라고 한다.
        write(STDOUT_FILENO, "Input Password : ", sizeof("Input Password : "));
        read(STDIN_FILENO, buf, MAX_BUF);//buf에 password를 적는다
        //개행문자를 널 문자로 바꾼다.
        for(int i = 0 ; i < MAX_BUF ; i++){
            if(buf[i] == '\n') {
                buf[i] = '\0';
                break;
            }   
        }
        write(sockfd, buf, MAX_BUF); //server에게 보낸다.

        
        n = read(sockfd, buf, MAX_BUF);
        buf[n] = '\0'; //마지막에 null문자 없을 수도 있기 때문에 쓴다.

        if(!strcmp(buf, "OK")){ //한번 확인
            n = read(sockfd, buf, MAX_BUF); //온 데이터 확인
            buf[n] = '\0'; //마지막에 null문자 없을 수도 있기 때문에 쓴다.
            if(!strcmp(buf, "OK")) { //아이디, 비번 모두 맞다.
                n = read(sockfd, buf, MAX_BUF); //온 데이터 확인
                buf[n] = '\0'; //마지막에 null문자 없을 수도 있기 때문에 쓴다.
                printf("** User '%s' logged in **\n", user);
                close(sockfd);
                exit(0);
            }
            else if(!strcmp(buf, "FAIL")){ //틀렸다.
                printf("** Log-in failed **\n");
                //틀렸다는 신호를 보내야한다.
            }
            else{ // 3번 모두 틀렸다.
                printf("** Connection closed **\n");
                close(sockfd);
                exit(0);
            }
        }
    }
}