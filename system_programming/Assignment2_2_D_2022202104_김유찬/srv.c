//////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/09 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-2 ( ftp server ) //
// Description : 1.client 온 문자열을 그대로 문자열에 보내준다. //
//               2.client와 연결되면 client에 ip, port를 출력해준다.
//               3. fork를 통해 1,2을 작업하는 각각의 프로세스를 만 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>   // 표준 입력/출력 함수 사용을 위한 헤더 파일
#include <stdlib.h>  // 표준 라이브러리 함수, 동적 할당, 프로세스 제어 함수 사용을 위한 헤더 파일
#include <string.h>  // 문자열 처리 함수 사용을 위한 헤더 파일
#include <unistd.h>  // 유닉스 표준 시스템 호출을 위한 헤더 파일 (예: read, write, close 등)
#include <arpa/inet.h> // 인터넷 주소 변환 함수 사용을 위한 헤더 파일
#include <sys/types.h>  // 시스템 데이터 타입 정의를 위한 헤더 파일
#include <sys/socket.h> // 소켓 프로그래밍 함수와 데이터 구조 사용을 위한 헤더 파일
#include <netinet/in.h> // 인터넷 주소 구조체 정의를 위한 헤더 파일
#include <sys/wait.h>  // 프로세스 제어 및 대기 함수 (wait) 사용을 위한 헤더 파일
#include <signal.h>   // 시그널 처리 함수 사용을 위한 헤더 파일

#define BUF_SIZE 256 // 버퍼 크기를 256으로 받는다.
char output[1024];  // 출력을 위한 버퍼
int length;         // sprintf로부터 반환된 문자열의 길이를 저장

///////////////////////////////////////////////////////////////////////
// client_info //
// ==================================================================
// Input: struct sockaddr_in* cliaddr -> Client's socket address information
// Output: int -> Returns 1 if successful, -1 if an error occurs
// Purpose: To retrieve and print the IP address and port number of the client
///////////////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in* cliaddr);

///////////////////////////////////////////////////////////////////////
// sh_chld //
// ==================================================================
// Input: int signum -> The signal number that triggered the handler (unused)
// Output: void -> This function does not return a value
// Purpose: To handle the SIGCHLD signal indicating a child process status change
///////////////////////////////////////////////////////////////////////
void sh_chld(int); // signal handler for SIGCHLD


///////////////////////////////////////////////////////////////////////
// sh_alrm //
// ==================================================================
// Input: int signum -> The signal number that triggered the handler (unused)
// Output: void -> This function does not return a value
// Purpose: To handle the SIGALRM signal by terminating the process
///////////////////////////////////////////////////////////////////////
void sh_alrm(int); // signal handler for SIGALRM

int main(int argc, char *argv[]) {
    char buff[BUF_SIZE]; // 데이터 송수신을 위한 버퍼
    int n; //읽거나 쓴 데이터의 바이트 수
    struct sockaddr_in server_addr, client_addr; // 서버와 클라이언트 주소 정보 구조체
    int server_fd, client_fd; // 서버 및 클라이언트 소켓 파일 디스크립터
    int len; // 주소 구조체의 크기
    int port; // 서버 포트 번호

    /* Applying signal handler(sh_alrm) for SIGALRM */
    if (signal(SIGALRM, sh_alrm) == SIG_ERR) {
        perror("Can't set signal handler");
        return 1;
    }

    /* Applying signal handler(sh_chld) for SIGCHLD */
    // 자식 프로세스가 종료되면 작동한다.
    if (signal(SIGCHLD, sh_chld) == SIG_ERR) {
        perror("Can't set signal handler");
        return 1;
    }

    server_fd = socket(PF_INET, SOCK_STREAM, 0); //서버 소켓(TCP)을 생성한다.

    memset(&server_addr, 0, sizeof(server_addr)); // 서버 주소 구조체를 초기화한다.
    server_addr.sin_family = AF_INET; //주소 체계를 인터넷 프로토콜로 설정
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP주소를 설정 
    server_addr.sin_port = htons(atoi(argv[1]));// 포트 번호를 입력으로 받아온다.

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //소켓에 주소를 바인딩한다.

    listen(server_fd, 5); // 동시에 5개의 클라이언트까지 연결을 기다린다.

    while(1) {
        pid_t pid;
        len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len); //클라이언트의 연결 요청을 수락한다.

        if(client_fd < 0){
            printf("Server: accept failed.\n");
            return 0;
        }


        /* 프로세스 소스 생성 (fork() 이용) client 연결이 들어올 때마다 연결*/ 
        pid = fork();//프로세스를 복제하다.
    
        if(pid < 0){// fork 실패 시 클라이언트 소켓을 닫는다.
            perror("fork 실패!!");
            close(client_fd); // 클라이언트 소켓 닫음
        }
        //부모 프로세스에서 하는일
        else if(pid != 0){
            //부모가 자식프로세스 pid를 갖고 있기 때문에 여기서 새로운 만들어진 Process PID를 출력한다.
			if (client_info(&client_addr) < 0) //client의 ip정보와 port번호를 출력한다.
				write(STDERR_FILENO, "client_info() err!!\n", sizeof("client_info() err!!\n"));
			    
			length = sprintf(output, "Child Process ID : %d\n", pid);
			write(STDOUT_FILENO, output, length);
            
			while (waitpid(-1, NULL, WNOHANG) > 0);  // 종료된 자식 프로세스 정리
        }
        else{
            //자식 프로세스에서 하는일  
            while(1){
                if((n = read(client_fd, buff, BUF_SIZE)) > 0){
					for(int i = 0 ; i < BUF_SIZE; i++){
						if(buff[i] == '\n'){
							buff[i+1] = '\0';
							break;
						}
					}
					// printf("%s\n",buff);
					// printf("hello");
            		//Quit이면
                    if(!strncmp(buff, "QUIT", strlen("QUIT"))){
                        length = sprintf(output,"Child Process(PID: %d) will be terminated.\n", getpid());
						write(STDOUT_FILENO, output, length);
                        //1초뒤에 프로세스 종료한다.
                        alarm(1);
                        close(client_fd); //클라이언트 소켓을 닫는다.
                        return 0;
                    }
                    else{
						// length = sprintf(output,"%s", buff);
                        // write(STDOUT_FILENO, output, length);
                        write(client_fd, buff, BUF_SIZE);
                    }
                }
            }
        }


        close(client_fd); //클라이언트 소켓을 닫는다.
    }

    return 0;
}

void sh_chld(int signum) {
    printf("Status of Child process was changed.\n");
    wait(NULL); //시스템에 종료된 상태의 자식 프로세스가 있다면, 해당 프로세스의 종료 상태를 회수하고, 프로세스 ID를 반환
}

void sh_alrm(int signum) {
    printf("Child Process (PID : %d) will be terminated.\n", getpid());
    exit(1); // 프로세스 종료
}

// 클라이언트 IP, PORT 정보를 출력하는 함수
int client_info(struct sockaddr_in* cliaddr){
    if(cliaddr->sin_family != AF_INET)
        return -1;
	length = sprintf(output, "==========Client info==========\n\n");
	write(STDOUT_FILENO, output, length);
    length = sprintf(output, "client IP: %s\n\n\n", inet_ntoa(cliaddr->sin_addr));
	write(STDOUT_FILENO, output, length);
    length = sprintf(output, "client port: %d\n\n", cliaddr->sin_port);
	write(STDOUT_FILENO, output, length);
	length = sprintf(output, "===============================\n");
	write(STDOUT_FILENO, output, length);
    return 1;
}