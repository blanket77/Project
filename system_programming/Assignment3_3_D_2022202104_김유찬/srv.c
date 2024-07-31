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
#include <fcntl.h>

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
#include <signal.h>     // 시그널 처리 관련 함수를 위한 헤더 파일 포함
/* 필요한 header file 선언 => 헸음*/
#define MAX_BUF 30000
#define MAX_LINE 256
#define SEND_BUFF 30000
#define SEND_SIZE 100 //받을 수 있는 문자열 갯수

struct tm *t1;
time_t time1;
char filename[100];
int n;
char output[1024];  // 출력을 위한 버퍼
int bin_sign = 1;

unsigned int port_num;
int length = 0;         // sprintf로부터 반환된 문자열의 길이를 저장
char temp[MAX_BUF]; // 임시 데이터 저장을 위한 버퍼
#define MAX_BUFF 30000
#define SEND_BUFF 30000
#define SEND_SIZE 100 //받을 수 있는 문자열 갯수

int data_sock;
struct sockaddr_in client_addr;
char buf[MAX_BUF];
char user[60], passwd[60];
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
int log_auth(int connfd, struct sockaddr_in cliaddr);

///////////////////////////////////////////////////////////////////////
// cmd_process //
// ==================================================================
// Input: char* buff -> Buffer containing the command received from the client
//        char* result_buff -> Buffer where the result of the command is stored
// Output: int -> Returns 1 if the command was processed successfully,
//                -1 if an error occurred, or if the command is invalid
// Purpose: To process commands received from the client, such as listing directory contents
//          or handling specific server commands like QUIT
///////////////////////////////////////////////////////////////////////

int cmd_process(char* buff, char* result_buff, char* message_buff);
///////////////////////////////////////////////////////////////////////
// print_file_information //
// ==================================================================
// Input: struct dirent *dirp -> Directory entry to be printed
//           struct stat *buf -> File status information          
// Output: None 
// Purpose: To print detailed information about a directory or file
///////////////////////////////////////////////////////////////////////
void print_file_information(struct dirent *, struct stat *, char* , int );


void sh_sigint(int); // signal handler for SIGALRM
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

void log_to_file(const char *log_msg) {
    int log_fd = open("logfile", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (log_fd < 0) {
        perror("Failed to open log file");
        return;
    }

    write(log_fd, log_msg, strlen(log_msg));
    write(log_fd, "\n", 1);
    close(log_fd);
}

int listenfd, connfd;
int main(int argc, char *argv[]) {

	if (signal(SIGINT, sh_sigint) == SIG_ERR) {
        perror("Can't set signal handler");
        return 1;
    }

    char result_buff[SEND_BUFF]; // 데이터 수신 및 결과 전송을 위한 버퍼
    char message_buff[SEND_BUFF];

	struct sockaddr_in servaddr, cliaddr;
	FILE *fp_checkIP; // FILE stream to cMAX_BUFFheck client’s IP
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

	char time_str[100];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
	bzero(buf, sizeof(buf));

	sprintf(buf, "%s[%s:%d] Server is started\n", time_str, inet_ntoa(servaddr.sin_addr),servaddr.sin_port);
	log_to_file(buf);

    FILE *file;
    // 파일을 이어 쓰기로 연다 (존재하지 않으면 새 파일을 생성한다)
    file = fopen("motd", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
    char buffer[80];
    time_t rawtime;
    struct tm *timeinfo;

    // 현재 시간을 얻는다
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // 표준 시간대 설정
    char tz[] = "KST";
    strftime(buffer, sizeof(buffer), "%a %b %d %T", timeinfo);
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), " KST %d", timeinfo->tm_year + 1900);
    // 데이터를 파일에 쓴다
    if (fprintf(file, "220 sswlab.kw.ac.kr FTP server (version myftp [1.0] %s) ready.\n", buffer) < 0) {
        perror("Failed to write to file");
        fclose(file);
        return 1;
    }
    // 파일을 닫는다
    fclose(file);

	int clilen = sizeof(cliaddr);
	listen(listenfd, 5); // 최대 5개의 클라이언트를 받을 수 있다.
	for(;;) {
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen); //client 받아 들인다.
	
		if (client_info(&cliaddr) < 0){ //clMAX_BUFFient의 ip정보와 port번호를 출력한다.
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

			ptr1 = strtok_r(line, ".", &tmp1); //ip부분마다 파싱#define SEND_SIZE 100 //받을 수 있는 문자열 갯수
			ptr2 = strtok_r(check_ip, ".", &tmp2); //ip부분마다 파싱

			while(ptr2 != NULL){
				if(!strcmp(ptr1, "*") || !strcmp(ptr1, ptr2)) //만약 확인되면
					check++; 
				else break;
				ptr1 = strtok_r(NULL, ".", &tmp1); //access 파일 ip부분마다 파싱
				ptr2 = strtok_r(NULL, ".", &tmp2); //client ip부분마다 파싱
			}
			if(check == 4){
				write(connfd, "IP OK\0", MAX_BUF); // 허용된 IP라고 OK 신호를 보낸다.
                bzero(buf, MAX_BUF);
                file = fopen("motd", "r");
                if (file == NULL) {
                    perror("Failed to open file");
                    return 1;
                }

                // 첫 번째 줄을 읽는다
                if (fgets(buf, sizeof(buf), file) != NULL) {
                } else {
                    // 첫 번째 줄을 읽지 못했을 때 (파일이 비어있거나 오류 발생)
                    printf("Failed to read the first line or the file is empty.\n");
                }
                printf("%s\n", buf);
                // 파일을 닫는다
                fclose(file);
                
                write(connfd, buf, MAX_BUF);
				break;
			}
		}
		if(check != 4){
			write(connfd, "IP Fail", MAX_BUF); // 허용된 IP가 아니라고 Fail 신호를 보낸다.
            bzero(buf, MAX_BUF);
            sprintf(buf, "431 This client can't access. Close the session.\n");
            write(connfd, buf, MAX_BUF);
            write(STDOUT_FILENO, buf, MAX_BUF);
			close(connfd);
		}
		fclose(fp); //닫는다.
	

		if(log_auth(connfd, cliaddr) == 0) { // if 3 times fail (ok : 1, fail : 0)
			close(connfd);
			continue;
		}

        while(1){
            memset(buf, '\0', sizeof(buf));
            memset(result_buff, '\0', sizeof(result_buff));
            memset(temp, '\0', sizeof(temp));
            bzero(message_buff, SEND_BUFF);
            n = read(connfd, buf, MAX_BUFF); //connfd에 있는 내용 buff에 적는다. 변환된 명령어를 받는다.
            
            buf[n] = '\0'; //끝에는 null문자를 넣는다.
            // write(STDOUT_FILENO, buf, MAX_BUF);

            for(int i = 0; i < MAX_BUFF; i++){
                if(buf[i] == '\n') 
                    buf[i] ='\0';
            }
			char command[MAX_BUF+1000];

            strcpy(temp , buf); // buff의 데이터는 strtok땜에 바뀌기 때문에 temp에 저장한다.
            write(STDOUT_FILENO, buf, MAX_BUF);
            write(STDOUT_FILENO, "\n", 1);
			char time_str[100];
			time_t now = time(NULL);
			struct tm *t = localtime(&now);

			strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
			bzero(command, sizeof(command));

			sprintf(command, "%s[%s:%d] %s| %s\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user, buf);
			log_to_file(command);

            if ( n = cmd_process(buf, result_buff, message_buff) < 0) //client에서 온 FTP명령어를 실행한다.
            {
                write(STDERR_FILENO, "cmd_process() err!!\n", sizeof("cmd_process() err!!\n"));

                write(connfd, "Q", sizeof(result_buff)); // client한테 큐를 보낸다.
                // write(connfd, "QUIT", sizeof("QUIT"));
                break;
            }
			write(STDOUT_FILENO, message_buff, SEND_BUFF);
			write(connfd, message_buff, SEND_BUFF); //result_buff에 있는 내용을 client에게 보낸다.
			log_to_file(message_buff);
			

            if(!strncmp(temp, "NLST", 4) || !strncmp(temp, "LIST", 4)){
                bzero(message_buff, sizeof(message_buff));
                strcpy(message_buff, "150 Opening data connection for directory list.\n");
				write(STDOUT_FILENO, message_buff, SEND_BUFF);
				log_to_file(message_buff);
                write(connfd, message_buff, SEND_BUFF); //result_buff에 있는 내용을 client에게 보낸다.
                write(connfd, result_buff, MAX_BUF); //result_buff에 있는 내용을 client에게 보낸다.
            }
			if(!strncmp(temp, "RETR", 4) ){
				bzero(message_buff, sizeof(message_buff));
				
				if(bin_sign == 1)
               		sprintf(message_buff, "150 Opening binary mode data connection for %s.\n", filename);
				else
					sprintf(message_buff, "150 Opening ascii mode data connection for %s.\n", filename);
				
				write(STDOUT_FILENO, message_buff, SEND_BUFF);
				log_to_file(message_buff);
                write(connfd, message_buff, SEND_BUFF); //result_buff에 있는 내용을 client에게 보낸다.
                
				
				write(connfd, result_buff, MAX_BUF); //result_buff에 있는 내용을 client에게 보낸다.
				
			}
			if(!strncmp(temp, "STOR", 4)){
				bzero(message_buff, sizeof(message_buff));
			
				if(bin_sign == 1)
               		sprintf(message_buff, "150 Opening binary mode data connection for %s.\n", filename);
				else
					sprintf(message_buff, "150 Opening ascii mode data connection for %s.\n", filename);
				
				write(STDOUT_FILENO, message_buff, SEND_BUFF);
				log_to_file(message_buff);
                write(connfd, message_buff, SEND_BUFF); //result_buff에 있는 내용을 client에게 보낸다.
                
			}

            if(!strncmp(temp, "NLST", 4) || !strncmp(temp, "RETR", 4) || !strncmp(temp, "STOR", 4)|| !strncmp(temp, "LIST", 4)){
                bzero(message_buff, sizeof(message_buff));
                strcpy(message_buff, "226 Complete transmission.\n");
				write(STDOUT_FILENO, message_buff, SEND_BUFF);
				log_to_file(message_buff);
                write(connfd, message_buff, SEND_BUFF); //result_buff에 있는 내용을 client에게 보낸다.
            }
            
            if (!strcmp(temp, "QUIT"))
            {
				char time_str[100];
				time_t now = time(NULL);
				struct tm *t2 = localtime(&now);
				strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t2);
				bzero(buf, sizeof(buf));

   				time_t time2 = mktime(t2);

				sprintf(buf, "%s[%s:%d] %s LOG_OUT [total service time : %.0fsec]\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user, difftime(time2, time1));
				log_to_file(buf);
                close(connfd); //보낼 파일을 닫는다.
                break;
            }

        }

		close(connfd);
	}
}

//회원 인증한다. => 완성
int log_auth(int connfd, struct sockaddr_in cliaddr){
	
	int n, count=1;
	while(1) {
		/* 코드 작성 (hint: username과 password를 client로부터 받는다) => 했음*/ 
		printf("** User is trying to log-in (%d/3) **\n", count);
		read(connfd, user, 60); // client한테 user를 받아온다.
        
		
        if(user_match(user, passwd) == 2){
            bzero(buf, MAX_BUF);
		    write(connfd, "ID OK", MAX_BUF); // OK 신호를 보낸다.            
		    
            bzero(buf, MAX_BUF);
            sprintf(buf, "331 Password is required for %s.\n", user);
            write(connfd, buf, MAX_BUF);
            write(STDOUT_FILENO, buf, MAX_BUF);
            read(connfd, passwd, 60); // client한테 user를 받아온다.
            //user가 맞는 확인한다.

            if((n = user_match(user, passwd)) == 1){ //맞다.
                write(connfd, "OK", MAX_BUF); // OK 신호를 보낸다.
                bzero(buf, MAX_BUF);
                sprintf(buf, "230 User %s logged in.\n", user);
                write(connfd, buf, MAX_BUF);
                write(STDOUT_FILENO, buf, MAX_BUF);

				char time_str[100];
				time_t now = time(NULL);
				t1 = localtime(&now);
				time1 = mktime(t1);

				strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t1);
				bzero(buf, sizeof(buf));

				sprintf(buf, "%s[%s:%d] %s LOG_IN\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user);
				log_to_file(buf);

                break; //맞기 때문에 while문에 빠져나와서 1를 반한하게 한다.
            }
            else if(n == 2){ // 아니다.
                if(count >= 3) { //3번 이상 틀리면
                    write(connfd, "FAILFAIL", MAX_BUF);  // 더 이상 안된다고 로그인 안된다고 신호를 보낸다.
                    bzero(buf, MAX_BUF);
                    sprintf(buf, "530 Failed to log-in\n");
					
					char time_str[100];
					time_t now = time(NULL);
					struct tm *t = localtime(&now);
					strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
					bzero(buf, sizeof(buf));

					sprintf(buf, "%s[%s:%d] %s LOG_FAIL\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user);
					log_to_file(buf);

                    write(connfd, buf, MAX_BUF);
                    write(STDOUT_FILENO, buf, MAX_BUF);
                    return 0; // 실패했다고 0을 반환한다.
                }
                write(connfd, "FAIL", MAX_BUF); //실패했다고 client한테 보낸다.
                bzero(buf, MAX_BUF);
                sprintf(buf, "430 Invalid username or password\n");
                write(connfd, buf, MAX_BUF);
                write(STDOUT_FILENO, buf, MAX_BUF);
                count++; //한번 더 센다.
				
				char time_str[100];
				time_t now = time(NULL);
				struct tm *t = localtime(&now);
				strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
				bzero(buf, sizeof(buf));

				sprintf(buf, "%s[%s:%d] %s LOG_FAIL\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user);
				log_to_file(buf);
                
            }
        }
        else{
            if(count >= 3) { //3번 이상 틀리면
                write(connfd, "FAILFAIL", MAX_BUF);  // 더 이상 안된다고 로그인 안된다고 신호를 보낸다.
                bzero(buf, MAX_BUF);
                sprintf(buf, "530 Failed to log-in\n");

				char time_str[100];
				time_t now = time(NULL);
				struct tm *t = localtime(&now);
				strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
				bzero(buf, sizeof(buf));

				sprintf(buf, "%s[%s:%d] %s LOG_FAIL\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user);
				log_to_file(buf);

                write(connfd, buf, MAX_BUF);
                write(STDOUT_FILENO, buf, MAX_BUF);
                return 0; // 실패했다고 0을 반환한다.
            }
            bzero(buf, MAX_BUF);
		    write(connfd, "ID NO", MAX_BUF); // OK 신호를 보낸다.            

			char time_str[100];
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
			strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
			bzero(buf, sizeof(buf));

			sprintf(buf, "%s[%s:%d] %s LOG_FAIL\n", time_str, inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port, user);
			log_to_file(buf);

            bzero(buf, MAX_BUF);
            sprintf(buf, "430 Invalid username or password\n");
            write(connfd, buf, MAX_BUF);
            write(STDOUT_FILENO, buf, MAX_BUF);
            count++;
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
        else if(!strcmp(ptr1, user) && strcmp(ptr2, passwd)){
            return 2; //아이디 하나만 같다.
        }
        else{}
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

// FTP 명령어를 실행한다.
int cmd_process(char* buff, char* result_buff, char* message_buff){
	optind = 0;//외부전역함수를 초기화한다.
    char *buffer; //임시로 받아놓는다.
    strcpy(buffer, buff);

    int numStrings = SEND_SIZE;
    char **argv = malloc(numStrings * sizeof(char*)); // 문자열 포인터 배열을 동적 할당
    for (int i = 0; i < numStrings; i++) {
        argv[i] = malloc(256 * sizeof(char)); // 각 문자열에 256 바이트 할당
    }	
    argv[2][0] = '\0';
    argv[3][0] = '\0';
    
    int argc = 0; //인자 갯수, 
    char *ptr= strtok(buff, " "); 
    if(ptr == NULL)
        return -1;

    strcpy(argv[0], ptr);

    int i = 1;
	while(ptr != NULL){
        argc++;
        ptr = strtok(NULL, " "); //혹시 개행문자가 끝에 있을 때를 대비해서 넣었다.
        if (ptr == NULL) //널문자를 만나면 빠져나온다.
            break;
        strcpy(argv[i], ptr);
		i++;
	}
    argv[i][0] = '\0'; // 다음 인자 널문자로 만든다. getopt를 함수를 쓸 때 그 다음에 인자가 들어갈 수 있어서

    opterr = 0; // getopt 함수의 에러 메시지 출력 설정, 0이면 출력 안 함
    int c = 0;
    if(strncmp(argv[0], "QUIT",4)==0){ // 첫 번째 인수가 "QUIT"인 경우
        while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?': // 옵션 문자가 잘못된 경우
                	length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc > 1){ // 인수가 너무 많은 경우
        	length = sprintf(output, "Error: argument is not required\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
        strcpy(message_buff, "221 Goodbye\n");
		return 1;
	}
    else if (strncmp(argv[0], "PORT", 4) == 0) {
        convert_str_to_addr(buffer, &port_num);
        int data_port;
        
        // PORT command가 잘됐고 server와 client에도 보낸다.
        strcpy(message_buff, "200 PORT command performed successfully.\n");

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
            strcpy(message_buff, "550 Failed to access.\n");
            return -1;
        }
        bzero(buf, MAX_BUF); // buffer 0으로 초기화
        return 1;
    }

    else if(strcmp(argv[0], "NLST")==0){
		
		int aflag = 0, lflag = 0;
        while ((c = getopt (argc, argv, "al")) != -1){
            switch (c){
				case 'a':
					aflag++;
					break;
				case 'l':
					lflag++;
					break;
				case '?':
                    length = sprintf(output, "Error: invalid option\n\n"); //다른 옵션이 나오면 출력된다.
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		int index = optind; //외부 전역함수를 함부로 바꾸면 안 되기 때문에 index라는 변수에 저장했다. 또한 이제부터는 옵션을 고려하지 않기 때문에 optind를 굳이 안 써도 된다.
		index++;
        // printf("aflag %d, lflag %d\n", aflag, lflag);
		// printf("%d, %s %s non %s\n",argc, argv[0], argv[1], argv[2]);
		// printf("%d %d\n", optind, argc);
		argv[index+1][0] = '\0'; //혹시나 뒤에 쓰레기 값이 있을 수도 있어서
		if(argc != optind){
			length = sprintf(output, "only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
            
            
			return 1;
		}
		if(aflag == 0 && lflag ==0){ // 아무런 옵션이 없는 겨우
			struct stat buf; 

			DIR *dp = NULL;
			struct dirent **dirp = 0;
			int n = 0;
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index-1][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				stat(argv[index], &buf); //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.
				int count = 0;
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.

				int idx = 0;
				int result_idx = 0;
				char temp[100] = {'\0',};

				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					if(count == 4){
						length = sprintf(temp + idx, "\n");
						idx += length;
						strncpy(result_buff+result_idx, temp, sizeof(temp));
						idx = 0;
						result_idx += 100; //result 인덱스 100 증가했다.
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(temp + idx, "%s/     ", dirp[i]->d_name);	
						idx += length;
					}
					else{
						length = sprintf(temp + idx, "%s     ", dirp[i]->d_name);
						idx += length;
					}
				}
				length = sprintf(temp + idx, "\n");			
				idx += length;
				strncpy(result_buff+result_idx, temp, sizeof(temp));
        		closedir(dp);
                
                
				return 0;
			}

			dp = opendir(argv[index]); //디렉토리 주소를 가져온다. 실패하면 null을 반환
			int isFile = stat(argv[index], &buf)+1; //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.

			//디렉토리이지만 실행권한이 없으면 실행이 되지 않는다.
			if(S_ISDIR(buf.st_mode) && !(buf.st_mode & S_IXUSR)){
				length = sprintf(output, "Error: cannot access\n\n");
				write(STDOUT_FILENO, output, length);
				closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
			}

			//애초에 파일이 없으면 디렉토리가 아니다. 
			//파일이 있지만 디렉토리가 아니면 디렉토리가 아니다.
			else if(!isFile || (isFile && !S_ISDIR(buf.st_mode))) {
				length = sprintf(output, "Error: No such file or directory\n\n");
				write(STDOUT_FILENO, output, length);
			}
			//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
			else{
				// length = sprintf(output, "%s\n", argv[index]);		
				n = scandir(argv[index], &dirp, NULL, alphasort);
				int count = 0;
				// 정렬된 파일 이름을 출력한다.
				int idx = 0;
				int result_idx = 0;
				char temp[100] = {'\0',};

				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					if(count == 4){
						length = sprintf(temp + idx, "\n");
						idx += length;
						strncpy(result_buff+result_idx, temp, sizeof(temp));
						idx = 0;
						result_idx += 100; //result 인덱스 100 증가했다.
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(temp + idx, "%s/     ", dirp[i]->d_name);	
						idx += length;
					}
					else{
						length = sprintf(temp + idx, "%s     ", dirp[i]->d_name);
						idx += length;
					}
				}
				length = sprintf(temp + idx, "\n");			
				idx += length;
				strncpy(result_buff+result_idx, temp, sizeof(temp));
        		closedir(dp);
                
        		return 0;
    		}
            
			return 0;
		}
		else if(aflag != 0 && lflag ==0){ // a옵션만 있는 경우
			struct stat buf;

			DIR *dp = NULL;
			struct dirent **dirp = 0;
			int n = 0;
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index-1][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				stat(argv[index], &buf); //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.
				int count = 0;
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				int idx = 0;
				int result_idx = 0;
				char temp[100] = {'\0',};

				for (int i = 0; i < n; i++) {
					if(count == 4){
						length = sprintf(temp + idx, "\n");
						idx += length;
						strncpy(result_buff+result_idx, temp, sizeof(temp));
						idx = 0;
						result_idx += 100; //result 인덱스 100 증가했다.
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(temp + idx, "%s/     ", dirp[i]->d_name);	
						idx += length;
					}
					else{
						length = sprintf(temp + idx, "%s     ", dirp[i]->d_name);
						idx += length;
					}
				}
				length = sprintf(temp + idx, "\n");			
				idx += length;
				strncpy(result_buff+result_idx, temp, sizeof(temp));
        		closedir(dp);
                
				return 0;
			}

			dp = opendir(argv[index]); //디렉토리 주소를 가져온다. 실패하면 null을 반환
			int isFile = stat(argv[index], &buf)+1; //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.

			//디렉토리이지만 실행권한이 없으면 실행이 되지 않는다.
			if(S_ISDIR(buf.st_mode) && !(buf.st_mode & S_IXUSR)){
				length = sprintf(output, "Error: cannot access\n\n");
				write(STDOUT_FILENO, output, length);
				closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
			}

			//애초에 파일이 없으면 디렉토리가 아니다. 
			//파일이 있지만 디렉토리가 아니면 디렉토리가 아니다.
			else if(!isFile || (isFile && !S_ISDIR(buf.st_mode))) {
				length = sprintf(output, "Error: No such file or directory\n\n");
				write(STDOUT_FILENO, output, length);
			}
			//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
			else{
				// length = sprintf(output, "%s\n", argv[index])sset;		
				n = scandir(argv[index], &dirp, NULL, alphasort);
				int count = 0;
				// 정렬된 파일 이름을 출력한다.
				int idx = 0;
				int result_idx = 0;
				char temp[100] = {'\0',};

				for (int i = 0; i < n; i++) {
					if(count == 4){
						length = sprintf(temp + idx, "\n");
						idx += length;
						strncpy(result_buff+result_idx, temp, sizeof(temp));
						idx = 0;
						result_idx += 100; //result 인덱스 100 증가했다.
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(temp + idx, "%s/     ", dirp[i]->d_name);	
						idx += length;
					}
					else{
						length = sprintf(temp + idx, "%s     ", dirp[i]->d_name);
						idx += length;
					}
				}
				length = sprintf(temp + idx, "\n");			
				idx += length;
				strncpy(result_buff+result_idx, temp, sizeof(temp));
        		closedir(dp);
                
        		return 0;
    		}
			
			return 0;
		}
		else if(aflag == 0 && lflag !=0){ //l옵셥만 있는 경우
			struct stat buf;

			DIR *dp = NULL;
			struct dirent **dirp = 0;
			int n = 0;
			char path[1024]; 

			char *directory = argv[index];
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index-1][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				directory = ".";
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf,  result_buff, 100*i);
				}		
        		closedir(dp);
                
				return 0;
			}

			dp = opendir(argv[index]); //디렉토리 주소를 가져온다. 실패하면 null을 반환
			int isFile = stat(argv[index], &buf)+1; //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.

			//디렉토리이지만 실행권한이 없으면 실행이 되지 않는다.
			if(S_ISDIR(buf.st_mode) && !(buf.st_mode & S_IXUSR)){
				length = sprintf(output, "Error: cannot access\n\n");
				write(STDOUT_FILENO, output, length);
				closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
			}

			//애초에 파일이 없으면 디렉토리가 아니다. 
			//파일이 있지만 디렉토리가 아니면 디렉토리가 아니다.
			else if(!isFile || (isFile && !S_ISDIR(buf.st_mode))) {
				length = sprintf(output, "Error: No such file or directory\n\n");
				write(STDOUT_FILENO, output, length);
			}
			
			//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
			else{
				length = sprintf(output, "%s\n", argv[index]);
				write(STDOUT_FILENO, output, length);
				n = scandir(argv[index], &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					// length = sprintf(output, "%s\n", path);
					stat(path, &buf);
					print_file_information(dirp[i], &buf,  result_buff, 100*i);
				}		
        		closedir(dp);
                
				return 0;
			}
            
			return 0;
		}
		else{ // a,l 옵션 모두 있는 경우
			struct stat buf;
			
			DIR *dp = NULL;
			struct dirent **dirp = 0;
			int n = 0;
			char path[1024]; 
			// printf("index = %d\n", index);
			char *directory = argv[index];
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index-1][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				directory = ".";
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf, result_buff, 100*i);
				}
				for (int i = 0; i < numStrings; i++) {
					free(argv[i]);  // 각 문자열에 대한 메모리 해제
				}
				free(argv);  // 문자열 포인터 배열에 대한 메모리 해제
                
				return 0;
			}

			dp = opendir(argv[index]); //디렉토리 주소를 가져온다. 실패하면 null을 반환
			int isFile = stat(argv[index], &buf)+1; //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.

			//디렉토리이지만 실행권한이 없으면 실행이 되지 않는다.
			if(S_ISDIR(buf.st_mode) && !(buf.st_mode & S_IXUSR)){
				length = sprintf(output, "Error: cannot access\n\n");
				write(STDOUT_FILENO, output, length);
				closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
			}

			//애초에 파일이 없으면 디렉토리가 아니다. 
			//파일이 있지만 디렉토리가 아니면 디렉토리가 아니다.
			else if(!isFile || (isFile && !S_ISDIR(buf.st_mode))) {
				length = sprintf(output, "Error: No such file or directory\n\n");
				write(STDOUT_FILENO, output, length);
			}
			//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
			else{
				length = sprintf(output, "%s\n", argv[index]);
				write(STDOUT_FILENO, output, length);
				n = scandir(argv[index], &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf,  result_buff, 100*i);
				}		
        		closedir(dp);
                
				return 0;
			}
            
			return -1;
		}
	}

    else if(strcmp(argv[0], "CWD")==0){ // "CWD" 명령 처리
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc > 2){
			length = sprintf(output, "Error: only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		if(chdir(argv[1]) == 0){ // 디렉토리 변경 시도
			char* buffer;
			buffer = malloc(256);

			getcwd(buffer, 256); // 현재 디렉토리 경로 가져오기
			length = sprintf(output, "%s %s\n", argv[1], argv[2]);
			write(STDOUT_FILENO, output, length);
            
            bzero(output, sizeof(output));
			sprintf(output, "250 CWD command performed successfully.\n");
			strcpy(message_buff, output);
			free(buffer);

			return 1;
		}
		else{
            char* buffer;
			buffer = malloc(256);

			getcwd(buffer, 256); // 현재 디렉토리 경로 가져오기
            bzero(output, sizeof(output));
			sprintf(output, "%s: Can't find such file or directory.\n", buffer);
			strcpy(message_buff, output);
			return -1;
		}
		
	}
	else if(strcmp(argv[0], "CDUP")==0){ // "CDUP" 명령을 처리하는 조건문
		while ((c = getopt (argc, argv, "")) != -1){ // getopt를 이용하여 옵션을 파싱
			switch (c){
				case '?': // 옵션 문자가 잘못된 경우 에러 메시지 출력
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc > 2){ // "CDUP" 명령은 인수가 3개를 초과하면 안 되므로 에러 처리
			length = sprintf(output, "Error: only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		if(chdir("..") == 0){ // 상위 디렉토리로 이동 시도
			char* buffer;
			buffer = malloc(256);

			// 현재 디렉토리 경로 저장을 위한 버퍼 준비
			char paths[256];
			strcpy(paths, argv[1]); // argv 값을 미리 paths에 저장

			getcwd(buffer, 256); // 현재 디렉토리 경로를 얻어와서 buffer에 저장
			length = sprintf(output, "%s\n", argv[0]);
            bzero(output, sizeof(output));
			sprintf(output, "250 CWD command performed successfully.\n");
			strcpy(message_buff, output);
			free(buffer);

			return 1;
		}
		else{
            bzero(output, sizeof(output));
			sprintf(output, "550 ..: Can’t find such file or directory.\n");
			strcpy(message_buff, output);
			return -1;
		}
	}
	else if(strcmp(argv[0], "PWD")==0){ // "PWD" 명령어 처리 (Print Working Directory)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?': // 옵션 오류 처리
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc > 1){ // "PWD" 명령은 추가 인수를 필요로 하지 않음
			length = sprintf(output, "Error: argument is not required\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		char* buffer;
		buffer = malloc(200);

		getcwd(buffer, 200); // 현재 디렉토리 경로를 buffer에 저장
        bzero(output, sizeof(output));
		length = sprintf(output, "257 \"%s\" is current directory \n", buffer);
		strcpy(message_buff, output);
		free(buffer);

		return 1;
	}
	else if(strcmp(argv[0], "MKD")==0){ // "MKD" 명령어 처리 (Make Directory)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?': // 잘못된 옵션이 주어진 경우
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc < 2){ // 인수가 최소 하나 필요함 (생성할 디렉토리 이름)
			length = sprintf(output, "Error: argument is required\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		for(int i = 1; i < argc; i++){ // 모든 주어진 디렉토리 이름에 대해 반복
			if(mkdir(argv[i], 0604) == 0){ // 디렉토리 생성 시도, 성공 시 메시지 출력
				bzero(output, sizeof(output));
                sprintf(output, "250 MKD command performed successfully\n");
                strcpy(message_buff, output);
			}
			else{ // 실패 시 에러 메시지 출력
                bzero(output, sizeof(output));
				sprintf(output, "550 %s: cannot create directory\n", argv[i]);
				strcpy(message_buff, output);
			}
		}
		return 1;
	}
	else if(strcmp(argv[0], "DELE")==0){ // "DELE" 명령어 처리 (Delete File)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc < 2){ // 파일 삭제 명령은 하나 이상의 인수가 필요함
			length = sprintf(output, "Error: argument is required\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		for(int i = 1; i < argc; i++){ // 주어진 파일 이름에 대해 반복
			if(unlink(argv[i]) == 0){ // 파일 삭제 시도, 성공 시 메시지 출력
				bzero(output, sizeof(output));
		        sprintf(output, "250 DELE command performed successfully\n");
		        strcpy(message_buff, output);
			}
			else{ // 실패 시 에러 메시지 출력
				bzero(output, sizeof(output));
		        sprintf(output, "550 %s: Can't find such file or directory\n", argv[i]);
		        strcpy(message_buff, output);
			}
		}
		return 1;
	}
	else if(strcmp(argv[0], "RMD")==0){ // "RMD" 명령어 처리 (Remove Directory)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		if(argc < 2){ // 디렉토리 삭제 명령은 하나 이상의 인수가 필요함
			length = sprintf(output, "Error: argument is required\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		for(int i = 1; i < argc; i++){ // 주어진 디렉토리 이름에 대해 반복
			if(rmdir(argv[i]) == 0){ // 디렉토리 삭제 시도, 성공 시 메시지 출력
				bzero(output, sizeof(output));
		        sprintf(output, "250 RMD command performed successfully\n");
		        strcpy(message_buff, output);
			}
			else{ // 실패 시 에러 메시지 출력
				bzero(output, sizeof(output));
		        sprintf(output, "550 %s: can't remove directory\n", argv[i]);
		        strcpy(message_buff, output);
			}
		}
		return 1;
	}
    else if( (strcmp(argv[0], "TYPE")==0) && (strcmp(argv[1], "A")==0)){ // "RMD" 명령어 처리 (Remove Directory)
        if( argv[2][0] != '\0'){
            bzero(output, sizeof(output));
            sprintf(output, "Type doesn't set.\n");
            strcpy(message_buff, output);
            return 1;
        }
        bin_sign = 0;
        bzero(output, sizeof(output));
        sprintf(output, "201 Type set to A.\n");
        strcpy(message_buff, output);


		return 1;
	}
    else if( (strcmp(argv[0], "TYPE")==0) && (strcmp(argv[1], "I")==0) ){ // "RMD" 명령어 처리 (Remove Directory)
        if( argv[2][0] != '\0'){
            bzero(output, sizeof(output));
            sprintf(output, "Type doesn't set.\n");
            strcpy(message_buff, output);
            return 1;
        }

        bin_sign = 1;
        bzero(output, sizeof(output));
        sprintf(output, "201 Type set to I.\n");
        strcpy(message_buff, output);
		return 1;
	}
    else if( (strcmp(argv[0], "RETR")==0)){ // "get" 명령어 처리 (Remove Directory)
        strcpy(filename,argv[1]);
        int file_fd = open(filename, O_RDONLY);
        // 파일을 읽고 클라이언트에게 전송
		read(file_fd, result_buff, MAX_BUF);
        write(connfd, result_buff, MAX_BUF);
		
        return 1;
	}
    else if( (strcmp(argv[0], "STOR")==0)){ // "put" 명령어 처리 (Remove Directory)
		strcpy(filename,argv[1]);

		int file_fd = open(filename, O_WRONLY| O_CREAT | O_TRUNC);
		// 파일을 읽고 클라이언트에게 전송
		read(connfd, result_buff, MAX_BUF);
		write(file_fd, result_buff, MAX_BUF);
		return 1;
	}
	else if(strcmp(argv[0], "RNFR")==0){ // "RNFR" 명령어 처리 (Rename From)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		// printf("%d",argc);
		if(argc != 4){ // 이름 변경 명령은 정확히 두 개의 인수가 필요함 (원본 및 대상 이름)
			length = sprintf(output, "Error: two arguments are required\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}

		struct stat buf;
		int isFile = stat(argv[3], &buf)+1; // 대상 파일의 존재 여부 확인
		if(isFile){ // 대상 파일이 이미 존재하는 경우
            bzero(output, sizeof(output));
            sprintf(output, "550 %s:Can't find such file or directory\n", argv[1]);
            strcpy(message_buff, output);
			return -1;
		}
        else{
            bzero(output, sizeof(output));
            sprintf(output, "File exists, ready to rename\n");
            strcpy(message_buff, output);
        }
		if(rename(argv[1], argv[3]) == 0){ // 파일 이름 변경 시도, 성공 시 메시지 출력
            bzero(output, sizeof(output));
            sprintf(output, "250 RNTO command succeeds\n");
            strcpy(message_buff, output);
			// length = sprintf(output, "%s %s\n%s %s\n",argv[0], argv[1], argv[2], argv[3]);
			// strcpy(result_buff, output);
		}
		else{ // 실패 시 에러 메시지 출력
            bzero(output, sizeof(output));
            sprintf(output, "550 %s: can't be renamed\n", argv[1]);
            strcpy(message_buff, output);
			return -1;
		}
		//length = sprintf(output, "\n");
		// write(STDOUT_FILENO, output, length);
		return 1;
	}
	else if(strcmp(argv[0], "LIST")==0){ // "LIST" 명령어를 처리하는 부분
        while ((c = getopt (argc, argv, "")) != -1){
            switch (c){
				case '?':
                    length = sprintf(output, "Error: invalid option\n\n"); //다른 옵션이 나오면 출력된다.
					write(STDOUT_FILENO, output, length);
					return -1;
			}
		}
		int index = optind; //외부 전역함수를 함부로 바꾸면 안 되기 때문에 index라는 변수에 저장했다. 또한 이제부터는 옵션을 고려하지 않기 때문에 optind를 굳이 안 써도 된다.
		index++;

		argv[index+1][0] = '\0'; //혹시나 뒤에 쓰레기 값이 있을 수도 있어서
		struct stat buf;
			
			DIR *dp = NULL;
			struct dirent **dirp = 0;
			int n = 0;
			char path[1024]; 
			// printf("index = %d\n", index);
			char *directory = argv[index];
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index-1][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				directory = ".";
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf, result_buff, 100*i);
				}
				for (int i = 0; i < numStrings; i++) {
					free(argv[i]);  // 각 문자열에 대한 메모리 해제
				}
				free(argv);  // 문자열 포인터 배열에 대한 메모리 해제
                
				return 0;
			}

			dp = opendir(argv[index]); //디렉토리 주소를 가져온다. 실패하면 null을 반환
			int isFile = stat(argv[index], &buf)+1; //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.

			//디렉토리이지만 실행권한이 없으면 실행이 되지 않는다.
			if(S_ISDIR(buf.st_mode) && !(buf.st_mode & S_IXUSR)){
				length = sprintf(output, "Error: cannot access\n\n");
				write(STDOUT_FILENO, output, length);
				closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
			}

			//애초에 파일이 없으면 디렉토리가 아니다. 
			//파일이 있지만 디렉토리가 아니면 디렉토리가 아니다.
			else if(!isFile || (isFile && !S_ISDIR(buf.st_mode))) {
				length = sprintf(output, "Error: No such file or directory\n\n");
				write(STDOUT_FILENO, output, length);
			}
			//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
			else{
				length = sprintf(output, "%s\n", argv[index]);
				write(STDOUT_FILENO, output, length);
				n = scandir(argv[index], &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf,  result_buff, 100*i);
				}		
        		closedir(dp);
                
				return 0;
			}
            
			return -1;
	}
    else{
        return -1;
    }    
} 

// 파일 정보를 출력하는 함수
void print_file_information(struct dirent *dirp, struct stat *buf, char* result_buff, int result_idx) {
	struct passwd *pwd = getpwuid(buf->st_uid); // 파일 소유자의 사용자 이름을 가져옵니다.
    struct group *grp = getgrgid(buf->st_gid);  // 파일 소유자의 그룹 이름을 가져옵니다.

    char time_str[256];
    // 파일의 마지막 수정 시간을 문자열로 변환합니다.
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&buf->st_mtime));
	char temp[100] = {'\0', };
	int idx = 0;
	int lengths; // idx에 더해야하는 값         

	// 파일의 권한 및 속성을 문자열로 구성하여 출력합니다.
    lengths = sprintf(temp + idx, (S_ISDIR(buf->st_mode)) ? "d" : "-"); 
	idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IRUSR) ? "r" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IWUSR) ? "w" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IXUSR) ? "x" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IRGRP) ? "r" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IWGRP) ? "w" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IXGRP) ? "x" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IROTH) ? "r" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IWOTH) ? "w" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, (buf->st_mode & S_IXOTH) ? "x" : "-");
    idx += lengths;
	lengths = sprintf(temp + idx, " %2ld", buf->st_nlink); // 연결된 하드 링크의 수
    idx += lengths;
	lengths = sprintf(temp + idx, " %s", pwd ? pwd->pw_name : "unknown"); // 소유자 이름
    idx += lengths;
	lengths = sprintf(temp + idx, " %s", grp ? grp->gr_name : "unknown"); // 그룹 이름
    idx += lengths;
	lengths = sprintf(temp + idx, " %6ld", buf->st_size); // 파일 크기
    idx += lengths;
	lengths = sprintf(temp + idx, " %s", time_str); // 마지막 수정 시간
    idx += lengths;

    // 디렉터리일 경우 디렉터리 이름 뒤에 슬래시(/)를 붙여 출력
    if(S_ISDIR(buf->st_mode)){
        lengths = sprintf(temp + idx, " %s/\n", dirp->d_name);
        idx += lengths;
    }
    else{
        lengths = sprintf(temp + idx, " %s\n", dirp->d_name);
        idx += lengths;
    }
	strncpy(result_buff+result_idx, temp, sizeof(temp));
	// printf("%s",temp);
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


//Cntl+C를 누르면 시그널 발생
void sh_sigint(int signum){
    //client한테 QUIT를 보낸다.
    write(connfd, "QUIT", 4);
   	close(connfd);
	char time_str[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", t);
	bzero(buf, sizeof(buf));

	sprintf(buf, "%s Server is terminated\n", time_str);
	log_to_file(buf);
    exit(1);
}