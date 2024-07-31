///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/30 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-2 ( ftp server ) //
// Description : NLST, QUIT에 대한 결과를 실행하고 그 결과를 client에 준다.
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#define MAX_BUF 1024

char buffer[1024];

unsigned int port_num;
///////////////////////////////////////////////////////////////////////
// convert_str_to_addr //
// ================================================================= //
// Input: char* str -> String containing the IP address and port, //
// unsigned int* port -> Pointer to store the extracted port number //
// (Input parameter Description) //
// Output: char* -> Pointer to the IP address part of the string //
// (Output parameter Description) //
// Purpose: Converts a string formatted as "IP,port" back into //
// separate IP address and port number components //
///////////////////////////////////////////////////////////////////////
char* convert_str_to_addr(char *str, unsigned int *port);

///////////////////////////////////////////////////////////////////////
// handle_client //
// ================================================================= //
// Input: int connfd -> File descriptor for the connected client socket //
// (Input parameter Description) //
// Output: void //
// (Output parameter Description) //
// Purpose: Handles client requests; processes data received from //
// the client connected through the socket represented by connfd //
///////////////////////////////////////////////////////////////////////
void handle_client(int connfd) {
    int data_sock;
    struct sockaddr_in client_addr;
    bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화

    while (read(connfd, buffer, MAX_BUF) > 0) {
        // printf("%d\n", port_num);
        printf("%s\n", buffer);
        
        if (strncmp(buffer, "PORT", 4) == 0) {
            convert_str_to_addr(buffer, &port_num);
            int data_port;
            
            // PORT command가 잘됐고 server와 client에도 보낸다.
            printf("200 PORT command performed successfully.\n");
            write(connfd, "200 PORT command performed successfully.\n", MAX_BUF);

            // Setting up data connection
            data_sock = socket(AF_INET, SOCK_STREAM, 0);
            bzero((char*)&client_addr, sizeof(client_addr)); // 서버 주소 구조체를 초기화한다.
            client_addr.sin_family = AF_INET; //주소 체계를 인터넷 프로토콜로 설정
            client_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP주소를 설정 
            client_addr.sin_port = htons(port_num);

            int addr_len = sizeof(client_addr);
            if (connect(data_sock, (struct sockaddr *)&client_addr, addr_len) < 0) {
                perror("Data connection failed");
                // write(STDOUT_FILENO, "11\n", 3);
                printf("550 Failed to access.\n");
                write(connfd, "550 Failed to access.\n", MAX_BUF);
                break;
            }
            bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화

        }
        // FTP 명령어가 NLST일 때 실행되는 코드
        else if (strncmp(buffer, "NLST", 4) == 0) {
            printf("150 Opening data connection for directory list.\n");
            write(connfd, "150 Opening data connection for directory list.\n", MAX_BUF);

            DIR *dir = opendir("."); //현대 디렉토리를 연다.
            struct dirent **dirp = 0;
            char tmp[200]; 
    
            bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화
            if (!dir) {
                write(data_sock, "디렉토리 열기 실패\n", 26);
                return;
            }
            // 현재 디렉토리에 있는 파일을 아스키코드 기준으로 오름차순을 하겠다.
            int n = scandir(".", &dirp, NULL, alphasort);
            for (int i = 0; i < n; i++) {
                if(dirp[i]->d_name[0] == '.' ) continue;
				bzero((char*)&tmp, 200); // buffer 0으로 초기화
                snprintf(tmp, MAX_BUF, "%s\n", dirp[i]->d_name); // 개행문자까지 포함해서 tmp에 넣고
                strcat(buffer, tmp); //tmp를 buffer에 하나씩 이어붙인다.
			}
            
            write(connfd, buffer, MAX_BUF);
            closedir(dir);
            
            printf("226 Complete transmission.\n");
            write(connfd, "226 Complete transmission.\n", MAX_BUF);
        } 
        // FTP 명령어가 QUIT일 때 실행되는 코드
        else if (strncmp(buffer, "QUIT", 4) == 0) {
            printf("221 Goodbye.\n");
            write(connfd, "221 Goodbye.\n", MAX_BUF);
            close(connfd);
            break;
        }
        else{}
        bzero((char*)&buffer, MAX_BUF); // buffer 0으로 초기화
    }
    close(connfd);
}


int main(int argc, char **argv)
{
    int listenfd, connfd;
    char *host_ip;
    char temp[25];
    struct sockaddr_in server_addr, cliaddr;
    listenfd = socket(PF_INET, SOCK_STREAM, 0); //서버 소켓(TCP)을 생성한다.

    memset(&server_addr, 0, sizeof(server_addr)); // 서버 주소 구조체를 초기화한다.
    server_addr.sin_family = AF_INET; //주소 체계를 인터넷 프로토콜로 설정
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP주소를 설정 
    server_addr.sin_port = htons(atoi(argv[1]));// 포트 번호를 입력으로 받아온다.

    // 서버 주소를 소켓에 바인딩
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed"); // 오류 메시지 출력
        close(listenfd); // 소켓 닫기
        return 1; // 프로그램 종료
    }


    int clilen = sizeof(cliaddr);
    listen(listenfd, 5);
    
    
    // port를 만들어서 보내기connfd
    while (1) {
        int client_len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &client_len); // client를 받아드린다.
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }
        handle_client(connfd);
    }

    close(listenfd);
    return 0;
}

char* convert_str_to_addr(char *str, unsigned int *port) {
    char *addr = malloc(30); // Allocate memory dynamically
    int ip[4]; // IP 주소의 각 바이트를 저장할 배열
    int p1, p2; // 포트 번호를 구성하는 두 부분

    // sscanf를 사용하여 str에서 IP 주소와 포트 번호 부분을 추출
    if (sscanf(str, "PORT %d,%d,%d,%d,%d,%d", &ip[0], &ip[1], &ip[2], &ip[3], &p1, &p2) != 6) {
        fprintf(stderr, "Error: Invalid input string format\n");
        return NULL; // 형식이 잘못되었을 경우 오류 메시지를 출력하고 NULL 반환
    }

    // IP 주소를 점 표기법으로 포맷팅
    sprintf(addr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    // 포트 번호 계산 (p1 * 256 + p2)
    *port = p1 * 256 + p2;

    return addr; // 포맷된 IP 주소 반환
}