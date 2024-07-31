///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/16 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-3 ( ftp server ) //
// Description : srv에서 명령어 처리하고 클라이언트한테 출력, 병렬처리 가능, 연결된 클라이언트 10초마다 출력
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
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#define LISTENQ 5 //클라이언트 신호를 받을 수 있는 갯수
#define BUF_SIZE 256 // 버퍼 크기를 256으로 받는다.
char output[1024];  // 출력을 위한 버퍼
int length = 0;         // sprintf로부터 반환된 문자열의 길이를 저장

#define MAX_BUFF 10000
#define SEND_BUFF 10000
#define SEND_SIZE 100 //받을 수 있는 문자열 갯수

///////////////////////////////////////////////////////////////////////
// client_info //
// ==================================================================
// Input: struct sockaddr_in* cliaddr -> Client's socket address information
// Output: int -> Returns 1 if successful, -1 if an error occurs
// Purpose: To retrieve and print the IP address and port number of the client
///////////////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in* cliaddr);

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

int cmd_process(char* buff, char* result_buff);
///////////////////////////////////////////////////////////////////////
// print_file_information //
// ==================================================================
// Input: struct dirent *dirp -> Directory entry to be printed
//           struct stat *buf -> File status information          
// Output: None 
// Purpose: To print detailed information about a directory or file
///////////////////////////////////////////////////////////////////////
void print_file_information(struct dirent *, struct stat *, char* , int );



//client 정보와 child_process의 담는 구조체이다. 
struct client_child_info {
    pid_t pid; //child process pid
    int port; //client port
    int connfd; //클라이언트와 연결된 소켓
    time_t starts_time; //시작시간을 담는 자료구조
    struct client_child_info* next; // 다음 주소를 가리키는 포인터
};

struct client_child_info* head = NULL; //client_child 정보를 담는 linked list 구조체의 head를 가리킴
int child_count = 0; //연결 중인 클라이언트 갯수
int listenfd = 0; 

///////////////////////////////////////////////////////////////////////
// client_child_info //
// ==================================================================
// Input: pid_t pid, int port, int connfd ,time_t starts_time -> Client's socket address information
// Output: None
// Purpose: store childprocess and client
///////////////////////////////////////////////////////////////////////
void add_child(pid_t pid, int port, int connfd ,time_t starts_time) {
    struct client_child_info* new_client = (struct client_child_info*)malloc(sizeof(struct client_child_info));
    new_client->pid = pid;
    new_client->port = port;
    new_client->connfd = connfd;
    new_client->starts_time = starts_time;
    new_client->next = NULL;

    //head가 없다면 
    if(head == NULL){
        head = new_client; //head가 new_cleint이다.
    }
    //그렇지 않다면
    else{
        struct client_child_info* point = head; //head를 가리킨다.
        while(point->next != NULL){ //point->next가 끝일 때까지
            point = point->next; // 앞으로 간다.
        }
        point->next = new_client; // point->next가 NULL이면 point->next는 추가된 클라이언트 정보를 가리킨다.
    }

    child_count++; //client가 추가되었기 때문에 연결된 갯수를 하나 늘린다.
}

///////////////////////////////////////////////////////////////////////
// remo_client //
// ==================================================================
// Input: pid_t pid -> Client's socket address information
// Output: None
// Purpose: remove childprocess and client
///////////////////////////////////////////////////////////////////////
void remo_client(pid_t pid) {
    struct client_child_info *cur = head; // 현재 노드를 가리킴
    struct client_child_info *prev = NULL; // 이전 노드를 가리킴, 처음에는 NULL로 초기화

    while (cur) { //curr이 NULL이 아닐 때까지
        if (cur->pid == pid) { //제거할 pid와 같으면
            if (prev) { //현재 노드가 첫번째 노드가 아니면
                prev->next = cur->next;
            } else { //현재 노드가 첫 번째 노드이면
                head = cur->next; // 리스트의 헤드를 현재 노드의 다음 노드로 설정
            }
            free(cur);//현재 노드를 메모리에서 해제
            child_count--; // 연결된 클라이언트 갯수 하나 줄임
            return; // 함수 종료
        }
        prev = cur; //이전 노드를 현재 노드로 설정
        cur = cur->next; //현재 노드를 다음 노드로 설정
    }
}
///////////////////////////////////////////////////////////////////////
// client_child_info //
// ==================================================================
// Input: struct sockaddr_in* cliaddr -> Client's socket address information
// Output: int -> Returns 1 if successful, -1 if an error occurs
// Purpose: To retrieve and print the IP address and port number of the client
///////////////////////////////////////////////////////////////////////
int client_child_info(struct sockaddr_in* cliaddr);

void sh_chld(int); // signal handler for SIGCHLD
void sh_alrm(int); // signal handler for SIGALRM
void sh_sigint(int); // signal handler for SIGALRM

int main(int argc, char *argv[]) {
    char buff[BUF_SIZE]; // 데이터 송수신을 위한 버퍼
    int n; //읽거나 쓴 데이터의 바이트 수
    struct sockaddr_in server_addr, client_addr; // 서버와 클라이언트 주소 정보 구조체
    int listenfd, connfd; // 서버 및 클라이언트 소켓 파일 디스크립터
    int len; // 주소 구조체의 크기
    int port; // 서버 포트 번호
    
    int clilen, len_out; // 클라이언트 주소의 길이 및 데이터 전송 길이
    char result_buff[SEND_BUFF]; // 데이터 수신 및 결과 전송을 위한 버퍼
    char temp[MAX_BUFF]; // 임시 데이터 저장을 위한 버퍼

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

    if (signal(SIGINT, sh_sigint) == SIG_ERR) {
        perror("Can't set signal handler");
        return 1;
    }


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

    listen(listenfd, LISTENQ); // 동시에 5개의 클라이언트까지 연결을 기다린다.
    for(;;) {
        pid_t pid;
        len = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr*)&client_addr, &len); //클라이언트의 연결 요청을 수락한다.

        if(connfd < 0){
            printf("Server: accept failed.\n");
            return 0;
        }
        /* 프로세스 소스 생성 (fork() 이용) client 연결이 들어올 때마다 연결*/ 
        //자식 프로세스
        if( (pid = fork()) == 0){
            close(listenfd);

            while(1){
                memset(buff, '\0', sizeof(buff));
                memset(result_buff, '\0', sizeof(result_buff));
                memset(temp, '\0', sizeof(temp));
            
                n = read(connfd, buff, MAX_BUFF); //connfd에 있는 내용 buff에 적는다.
                buff[n] = '\0'; //끝에는 null문자를 넣는다.

                for(int i = 0; i < MAX_BUFF; i++){
                    if(buff[i] == '\n') 
                        buff[i] ='\0';
                }
                
                strcpy(temp , buff); // buff의 데이터는 strtok땜에 바뀌기 때문에 temp에 저장한다.

                // printf("%s\n", buff);

                if (cmd_process(buff, result_buff) < 0) //client에서 온 FTP명령어를 실행한다.
                {
                    write(STDERR_FILENO, "cmd_process() err!!\n", sizeof("cmd_process() err!!\n"));
                    write(connfd, "Q", sizeof(result_buff)); // client한테 큐를 보낸다.
                    // write(connfd, "QUIT", sizeof("QUIT"));
                    break;
                }
                
                write(connfd, result_buff, sizeof(result_buff)); //result_buff에 있는 내용을 client에게 보낸다.
                
                if (!strcmp(result_buff, "QUIT"))
                {
                    printf("Client(%5d)'s Release\n\n", getpid());
                    close(connfd); //보낼 파일을 닫는다.
                    exit(0);
                    break;
                }
                printf("%s\t\t[%d]\n", temp, getpid());
            }
            
            close(connfd);
            exit(0);
        }
    
        else if(pid < 0){// fork 실패 시 클라이언트 소켓을 닫는다.
            perror("fork 실패!!");
            close(connfd); // 클라이언트 소켓 닫음
        }
        //부모 프로세스에서 하는일
        else if(pid != 0){
            //부모가 자식프로세스 pid를 갖고 있기 때문에 여기서 새로운 만들어진 Process PID를 출력한다.
			if (client_child_info(&client_addr) < 0) //client의 ip정보와 port번호를 출력한다.
				write(STDERR_FILENO, "client_child_info() err!!\n", sizeof("client_child_info() err!!\n"));
			    
			printf("Child Process ID : %d\n", pid);

            // struct client_child_info* curr = head;
            time_t now = time(NULL);
            add_child(pid, ntohs(client_addr.sin_port), connfd, now);
            alarm(1);
       
			while (waitpid(-1, NULL, WNOHANG) > 0);  // 종료된 자식 프로세스 정리
        }
        else{ }


        close(connfd); //클라이언트 소켓을 닫는다.
    }

    return 0;
}

void sh_chld(int signum) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        remo_client(pid);
    }
}

void sh_alrm(int signum) {
    struct client_child_info* curr = head;
    time_t now = time(NULL);
    printf("Current Number of Client :  %d\n", child_count);
    printf("  PID\t PORT\tTIME\n");
    while (curr) {
        printf("%d\t%d\t%4ld\n", curr->pid, curr->port, now - curr->starts_time);
        curr = curr->next;
    }
    alarm(10);
}

//Cntl+C를 누르면 시그널 발생
void sh_sigint(int signum){
    
    struct client_child_info* curr = head; //child process 정보를 담은 구조체 헤드를 가리킴
    while (curr) { //curr이 NULL이 아닐 때 까지
        close(curr->connfd); //모든 client의 연결 종료
        kill(curr->pid, SIGKILL); //모든 child process 종료
        curr = curr->next; //그 다음 chlid process 정보를 가리킨다.
    }
    exit(1);
}

// 클라이언트 IP, PORT 정보를 출력하는 함수
int client_child_info(struct sockaddr_in* cliaddr){
    if(cliaddr->sin_family != AF_INET)
        return -1;
    
	printf("==========Client info==========\n");
	printf("client IP: %s\n", inet_ntoa(cliaddr->sin_addr));
	printf("client port: %d\n", cliaddr->sin_port);
	printf("===============================\n");
	return 1;
}

// FTP 명령어를 실행한다.
int cmd_process(char* buff, char* result_buff){
	optind = 0;//외부전역함수를 초기화한다.
    int numStrings = SEND_SIZE;
    char **argv = malloc(numStrings * sizeof(char*)); // 문자열 포인터 배열을 동적 할당
    for (int i = 0; i < numStrings; i++) {
        argv[i] = malloc(256 * sizeof(char)); // 각 문자열에 256 바이트 할당
    }	
    
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
    if(strcmp(argv[0], "QUIT")==0){ // 첫 번째 인수가 "QUIT"인 경우
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
        strcpy(result_buff, "QUIT");
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
				// length = sprintf(output, "%s\n", argv[index]);		
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
			length = sprintf(output, "\"%s\" is current directory\n\n", buffer);
			strcpy(result_buff, output);
			free(buffer);

			return 1;
		}
		else{
			length = sprintf(output, "Error: directory not found\n\n");
			write(STDOUT_FILENO, output, length);
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
		if(chdir(argv[1]) == 0){ // 상위 디렉토리로 이동 시도
			char* buffer;
			buffer = malloc(256);

			// 현재 디렉토리 경로 저장을 위한 버퍼 준비
			char paths[256];
			strcpy(paths, argv[1]); // argv 값을 미리 paths에 저장

			getcwd(buffer, 256); // 현재 디렉토리 경로를 얻어와서 buffer에 저장
			length = sprintf(output, "%s\n", argv[0]);
			strcpy(result_buff, output);
			length = sprintf(output, "\"%s\" is current directory\n\n", buffer);
			strcat(result_buff, output);
			free(buffer);

			return 1;
		}
		else{
			length = sprintf(output, "Error: directory not found\n\n");
			write(STDOUT_FILENO, output, length);
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
		buffer = malloc(256);

		getcwd(buffer, 256); // 현재 디렉토리 경로를 buffer에 저장
		length = sprintf(output, "\"%s\" is current directory\n\n", buffer);
		strcpy(result_buff, output);
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
				length = sprintf(output, "%s %s\n", argv[0], argv[i]);
				strcpy(result_buff, output);
			}
			else{ // 실패 시 에러 메시지 출력
				length = sprintf(output, "Error: cannot create directory \'%s\': File exists\n", argv[i]);
				write(STDOUT_FILENO, output, length);
			}
		}
		//length = sprintf(output, "\n");
		// write(STDOUT_FILENO, output, length);
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
				length = sprintf(output, "%s %s\n", argv[0], argv[i]);
				strcpy(result_buff, output);
			}
			else{ // 실패 시 에러 메시지 출력
				length = sprintf(output, "Error: failed to delete \'%s\'\n", argv[i]);
				write(STDOUT_FILENO, output, length);
			}
		}
		//length = sprintf(output, "\n");
		// write(STDOUT_FILENO, output, length);
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
				length = sprintf(output, "%s %s\n", argv[0], argv[i]);
				strcat(result_buff, output);
			}
			else{ // 실패 시 에러 메시지 출력
				length = sprintf(output, "Error: failed to remove \'%s\'\n", argv[i]);
				write(STDOUT_FILENO, output, length);
			}
		}
		//length = sprintf(output, "\n");
		// write(STDOUT_FILENO, output, length);
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
			length = sprintf(output, "Error: name to change already exists\n\n");
			write(STDOUT_FILENO, output, length);
			return -1;
		}
		if(rename(argv[1], argv[3]) == 0){ // 파일 이름 변경 시도, 성공 시 메시지 출력
			length = sprintf(output, "%s %s\n%s %s\n",argv[0], argv[1], argv[2], argv[3]);
			strcpy(result_buff, output);
		}
		else{ // 실패 시 에러 메시지 출력
			length = sprintf(output, "Error: failed to rename");
			write(STDOUT_FILENO, output, length);
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