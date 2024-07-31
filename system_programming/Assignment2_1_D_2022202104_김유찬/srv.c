///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/02 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Kim You Chan //
// Student ID : 2022202104 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 ( ftp server ) //
// Description : client에서 온 명령어를 실행하고 출력결과를 server에 보낸다.//
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

char output[1024];  //출력 버퍼
int length =0;         //sfprint에서 나오는 문자열의 길이

int main(int argc, char **argv)
{
    struct sockaddr_in server_addr, cliaddr; // 서버 및 클라이언트 주소 정보를 저장할 구조체
    int listenfd; // 서버의 소켓 파일 디스크립터 (서버 측 엔드포인트)
    int connfd; // 클라이언트와 통신을 위한 소켓 파일 디스크립터
    int clilen, len_out; // 클라이언트 주소의 길이 및 데이터 전송 길이
    int portno = atoi(argv[1]); // 명령행 인수에서 포트 번호를 가져와 정수로 변환
    char buff[MAX_BUFF], result_buff[SEND_BUFF]; // 데이터 수신 및 결과 전송을 위한 버퍼
    int n; // 읽기/쓰기 데이터 길이 또는 결과를 저장하는 변수
    char temp[MAX_BUFF]; // 임시 데이터 저장을 위한 버퍼

    //socket TCP로 만들어서 listenfd(서버 쪽 네트워크 엔드포인트)로 만든다..
    if((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("Server: Can't open stream socket.");
        return 0;
    }
    
    //초기화 시킴
    bzero((char *)&server_addr, sizeof(server_addr));
    //server 정보 입력
    server_addr.sin_family = AF_INET;
    //INADDR_ANY는 네트워크가 여러개가 있어서도 어느 한쪽의 IP주소에 국한되지 않고 모두 다 포용한다.
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(portno);
	//서버 주소를 listenfd에 bind한다. 이제 listenfd는 네트워크 통신을 위한 통신 엔드포인트 역할을 수행한다.
    if(bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("Server: Can't bind local address.\n");
        return 0;
    }

    //queue에 들어갈 수 있는 수 5개
    listen(listenfd, 5);
    /* open socket and listen */
    for (;;)
    {
        clilen = sizeof(cliaddr);
        //listenfd, 즉 엔드포인트에서 받은 cliaddr와 연결시키고 그 파일 디스크립터 번호를 connfd에 할당한다. 이 connfd는 sockfd와 같은 파일이다.
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
        if (client_info(&cliaddr) < 0) //client의 ip정보와 port번호를 출력한다.
            write(STDERR_FILENO, "client_info() err!!\n", sizeof("client_info() err!!\n"));
        while (1)
        {
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
                write(STDOUT_FILENO, "QUIT\n", sizeof("QUIT\n"));
                close(connfd); //보낼 파일을 닫는다.
                exit(0);
                break;
            }
			write(STDOUT_FILENO, temp, sizeof(temp));
			write(STDOUT_FILENO,"\n", 1);
        }
    }
    close(listenfd); //서버쪽 네트워크 엔드포인트를 닫고 프로그램을 끝낸다.
    return 0;
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
					return -1;
			}
		}
		if(argc > 1){ // 인수가 너무 많은 경우
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