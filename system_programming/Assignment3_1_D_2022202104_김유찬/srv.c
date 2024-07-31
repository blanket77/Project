///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/21 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 ( ftp server ) //
// Description : client한테 id와 pawd를 받고 서버에 있는 passwd에 내용인지 확인하고
//               허용된 IP인지도 access.txt에 있는 걸 보고 확인하고 허용시킨다.
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
/* 필요한 header file 선언 => 헸음*/
#define MAX_BUF 1024
#define MAX_LINE 256

///////////////////////////////////////////////////////////////////////
// client_info
// ==================================================================
// Input: struct sockaddr_in* cliaddr -> pointer to client address structure
// Output: int -> returns status (0 for success, -1 for failure)
// Purpose: Retrieves and processes client information
///////////////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in* cliaddr);

///////////////////////////////////////////////////////////////////////
// user_match
// ==================================================================
// Input: char *user, char *passwd -> username and password strings
// Output: int -> returns match result (1 for match, 0 for no match)
// Purpose: Checks if the provided user and password match stored credentials
///////////////////////////////////////////////////////////////////////
int user_match(char *user, char *passwd);

///////////////////////////////////////////////////////////////////////
// log_auth
// ==================================================================
// Input: int connfd -> connection file descriptor
// Output: int -> returns authentication result (1 for success, 0 for failure)
// Purpose: Performs authentication process
///////////////////////////////////////////////////////////////////////
int log_auth(int connfd);

int main(int argc, char *argv[]) {
	int listenfd, connfd;
	struct sockaddr_in servaddr, cliaddr;
	FILE *fp_checkIP; // FILE stream to check client’s IP
	// 코드 작성 => 완성  
	bzero((char*)&servaddr, sizeof(servaddr)); // 서버 주소 구조체를 초기화한다.
    servaddr.sin_family = AF_INET; //주소 체계를 인터넷 프로토콜로 설정
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP주소를 설정 
    servaddr.sin_port = htons(atoi(argv[1]));// 포트 번호를 입력으로 받아온다.

	listenfd = socket(AF_INET, SOCK_STREAM, 0); //sock을 만든다.

	if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){ //sock과 server 관련 정보를 합친다.
        printf("Server: Can't bind local address.\n");
        return 0;
    }

	int clilen = sizeof(cliaddr);
	listen(listenfd, 5); // 최대 5개의 클라이언트를 받을 수 있다.
	for(;;) {
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen); //client 받아 들인다.
	
		if (client_info(&cliaddr) < 0){ //client의 ip정보와 port번호를 출력한다.
            printf("** not client **\n");
			break;
		}
		/* 코드 작성 (hint: Client의 IP가 접근 가능한지 확인) */
		char check_ip[20]; 
		strcpy(check_ip, inet_ntoa(cliaddr.sin_addr)); // IP 주소를 문자열로 변환하고 check_ip에 복사합니다.

		FILE *fp;
		char line[MAX_LINE]; // Buffer line
		fp = fopen("access.txt", "r");
		/* 코드 작성 (hint: 인증 성공 시 return 1, 인증 실패 시 return 0 */
		if (fp == NULL) {
			printf("file can not open!!\n");
		}

		int check = 0;//확인 4번이면 맞다. 
		//한줄 한줄 읽는다.
		while (fgets(line, MAX_LINE, fp)) {
			check = 0; //한줄 읽고 시작할 때 0으로 초기화
			for(int i = 0 ; i < MAX_LINE ; i++){
				if(line[i] == '\n'){
					line[i] = '\0';
					break;
				}
			}
			char* ptr1, *tmp1, *ptr2, *tmp2;

			ptr1 = strtok_r(line, ".", &tmp1); //ip부분마다 파싱
			ptr2 = strtok_r(check_ip, ".", &tmp2); //ip부분마다 파싱

			while(ptr2 != NULL){
				if(!strcmp(ptr1, "*") || !strcmp(ptr1, ptr2)) //만약 확인되면
					check++; 
				else break;
				ptr1 = strtok_r(NULL, ".", &tmp1); //access 파일 ip부분마다 파싱
				ptr2 = strtok_r(NULL, ".", &tmp2); //client ip부분마다 파싱
			}
			if(check == 4){
				write(connfd, "IP OK", MAX_BUF); // 허용된 IP라고 OK 신호를 보낸다.
				printf("** Client is connected **\n");
				break;
			}
		}
		if(check != 4){
			write(connfd, "IP Fail", MAX_BUF); // 허용된 IP가 아니라고 Fail 신호를 보낸다.
			printf("** It is NOT authenticated client **\n");
			continue;
			close(connfd);
		}
		fclose(fp); //닫는다.
		

		if(log_auth(connfd) == 0) { // if 3 times fail (ok : 1, fail : 0)
			printf("** Fail to log-in **\n");
			close(connfd);
			continue;
		}
		printf("** Success to log-in **\n");
		close(connfd);
	}
}

//회원 인증한다. => 완성
int log_auth(int connfd){
	char user[MAX_BUF], passwd[MAX_BUF];
	int n, count=1;
	while(1) {
		/* 코드 작성 (hint: username과 password를 client로부터 받는다) => 했음*/ 
		printf("** User is trying to log-in (%d/3) **\n", count);
		read(connfd, user, MAX_BUF); // client한테 user를 받아온다.
		read(connfd, passwd, MAX_BUF); // client한테 passwd를 받아온다.
		
		write(connfd, "OK", MAX_BUF); // OK 신호를 보낸다.
		
		//user가 맞는 확인한다.
		if((n = user_match(user, passwd)) == 1){ //맞다.
			write(connfd, "OK", MAX_BUF); // OK 신호를 보낸다.
			break; //맞기 때문에 while문에 빠져나와서 1를 반한하게 한다.
		}
		else if(n == 0){ // 아니다.
			if(count >= 3) { //3번 이상 틀리면
				write(connfd, "FAILFAIL", MAX_BUF);  // 더 이상 안된다고 로그인 안된다고 신호를 보낸다.
				return 0; // 실패했다고 0을 반환한다.
			}
			printf("** Log-in failed **\n");
			write(connfd, "FAIL", MAX_BUF); //실패했다고 client한테 보낸다.
			count++; //한번 더 센다.
			continue; //반복문 돈다.
		}
	}
	return 1; //로그인 비번이 맞다
}
//user 아이디, 비번이 모두 맞으면 신호 1을 보내고 아니면 0을 보낸다. => 완성
int user_match(char *user, char *passwd){
	FILE *fp;
	struct passwd *pw;
	char line[MAX_LINE]; // Buffer line
    
	fp = fopen("passwd", "r");
	/* 코드 작성 (hint: 인증 성공 시 return 1, 인증 실패 시 return 0 */
	if (fp == NULL) {
		printf("file can not open!!\n");
        return 0;
    }

    //한줄 한줄 읽는다.
    while (fgets(line, MAX_LINE, fp)) {
		char* ptr1;char* ptr2;
		ptr1 = strtok(line, ":"); //아이디 파싱
		ptr2 = strtok(NULL, ":"); //비밀번호 파싱
		
		if(!strcmp(ptr1, user) && !strcmp(ptr2, passwd)){
			fclose(fp);
			return 1; //회원정보와 매칭된게 있기 때문에 맞다는 신호를 보낸다.
		}
	}
	fclose(fp);
	return 0; //파일을 다 검색해도 없기 때문에 회원정보와 매칭된게 없다는 신호를 보낸다.
}

// 클라이언트 IP, PORT 정보를 출력하는 함수 => 완성
int client_info(struct sockaddr_in* cliaddr){
	printf("** Client is trying to connect **\n");
    if(cliaddr->sin_family != AF_INET)
        return -1;
	printf(" - IP:\t  %s\n", inet_ntoa(cliaddr->sin_addr)); //Client IP 출력한다.
	printf(" - Port:  %d\n", cliaddr->sin_port); //Client Port를 출력한다.
	
    return 1;
}