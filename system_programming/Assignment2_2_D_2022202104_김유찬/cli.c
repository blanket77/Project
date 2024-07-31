///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/09 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-2 ( ftp server ) //
// Description : client에서 온 명령어를 FTP 명령어 변환하여 server에 보내고 //
//               server에서 온 출력 결과를 출력한다.
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
#include <string.h>    //string 관련 함수를 쓰기 위해서 해더 파일 포함
#define BUF_SIZE 256 //버퍼 크기를 256를 설정

int main(int argc, char *argv[]) 
{
    char buff[BUF_SIZE]; //데이터 송수신을 위한 버퍼
    int n; //잠시 담을 변수
    int sockfd; // 소켓 파일 디스크립터
    struct sockaddr_in serv_addr; //서버의 주소 정보를 저장할 구조체

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓을 생성한다.

    memset(&serv_addr, 0, sizeof(serv_addr)); // 서버 정보르 담음 구조체 초기화한다.
    serv_addr.sin_family = AF_INET; // 주소 체계를 인터넷 체계로
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 주소를 받아서 설정한다.
    serv_addr.sin_port = htons(atoi(argv[2])); // 포트 번호를 설정한다.
    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); // 서버에 연결한다. sock_fd로 주고 받는다.
    while(1) { 
        write(STDOUT_FILENO, "> ", 2); //프롬프트 출력
        read(STDIN_FILENO, buff, BUF_SIZE); //표준 입력으로부터 데이터를 읽는다.

        if (write(sockfd, buff, BUF_SIZE) > 0) { // 서버로 데이터 전송한다.
            if(read(sockfd, buff, BUF_SIZE) > 0) // 서버로부터 데이터를 받는다.
                printf("from server: %s", buff); // 수신된 데이터를 출력한다.
            else
                break; // 데이터 수신을 실패하면 루프를 종료한다.
        } else 
            break; // 데이터 전송 실패하면 루프를 종료한다.
    }
    close(sockfd); // 소켓을 닫고
    return 0; // 프로그램을 종료한다.
}
