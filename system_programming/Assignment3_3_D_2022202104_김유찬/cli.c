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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>


#define MAX_BUF 30000
#define RCV_BUFF 30000

char buf[MAX_BUF];
int sign = 0;
int bin_sign = 1;
char filename[100];

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
// log_in//
// ==================================================================
// Input: int sockfd -> socket descriptor
// Output: None
// Purpose: check user and password
///////////////////////////////////////////////////////////////////////
void log_in(int sockfd);

///////////////////////////////////////////////////////////////////////
// conv_cmd //
// ==================================================================
// Input: char* buf -> Buffer containing the original command from the client
//        char* cmd_buff -> Buffer where the converted command should be stored
// Output: int -> Returns 1 if the command conversion was successful, -1 if an error occurred
// Purpose: To convert client commands into a format that the server can process more effectively,
//          such as converting Linux system commands into FTP commands
///////////////////////////////////////////////////////////////////////
int conv_cmd(char* buf, char* cmd_buff);

///////////////////////////////////////////////////////////////////////
// process_result //
// ==================================================================
// Input: char* rcv_buff -> Buffer containing the server's response to the command
// Output: None
// Purpose: To process and potentially modify the server's response before it is sent back to the client,
//          such as formatting the output or filtering results
///////////////////////////////////////////////////////////////////////
void process_result(char* rcv_buff);



void disable_echo_mode() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void enable_echo_mode() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void read_password(char *buf, int max_buf) {
    int i = 0;
    char ch;

    while (i < max_buf - 1) {
        read(STDIN_FILENO, &ch, 1);
        if (ch == '\n') {
            buf[i] = '\0';
            break;
        } else if (ch == 127 || ch == 8) { // Handle backspace
            if (i > 0) {
                i--;
                write(STDOUT_FILENO, "\b \b", 3); // Erase the '*' on the screen
            }
        } else {
            buf[i++] = ch;
            write(STDOUT_FILENO, "*", 1);
        }
    }
    buf[i] = '\0';
}

int main(int argc, char *argv[]){
    int n, p_pid;
    struct sockaddr_in servaddr;
    int sockfd;
    char message_buff[MAX_BUF];
    char cmd_buff[MAX_BUF]= {'\0'}, rcv_buff[RCV_BUFF] = {'\0'}; //입력에 담은 buf, buf에 담긴거 변환해서 담는 cmd_buf, server에게 보낼 rcv_buff
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

    write(STDOUT_FILENO, "Connected to sswlab.kw.ac.kr.\n", sizeof("Connected to sswlab.kw.ac.kr.\n"));
    bzero(buf, MAX_BUF);
    // sprintf(buf, "before %d\n", sockfd);
    write(STDOUT_FILENO, buf, MAX_BUF);

    log_in(sockfd);
    
    bzero(buf, MAX_BUF);
    // sprintf(buf, "after %d\n", sockfd);
    write(STDOUT_FILENO, buf, MAX_BUF);
    // 클라이언트 엔드포인트(sockfd)에 서버를 연결한다.

    write(STDOUT_FILENO, "> ", 2);
    //소켓이 열려있고 server와 연결이 되어있으면 밑에 코드 작동
    for (;;){
        bzero(buf, MAX_BUF);
        //실행 파일을 제외한 입력을 띄어쓰기로 받는다.
        read(STDIN_FILENO, buf, MAX_BUF);
        for(int i = 0; i < MAX_BUF; i++){
			if(buf[i] == '\n') {
                buf[i] ='\0';
                break;
            }
		} //개행문자가 있으면 지운다.
           
        bzero(cmd_buff, MAX_BUF);
        if (conv_cmd(buf, cmd_buff) < 0) //리눅스 명령어(buf)를 FTP(cmd_buff)로 바꿔준다. 
        {
            write(STDERR_FILENO, "conv_cmd() error!!\n", sizeof("conv_cmd() error!!\n"));
            exit(1);  //프로그램 종료
        }
        for(int i = 0; i < MAX_BUF; i++){
			if(cmd_buff[i] == '\n') {
                cmd_buff[i] ='\0';
                break;
            }
		} //개행문자가 있으면 지운다.

        // //서버에게 변환된 FTP 명령어(cmd_buff)를 클라이언트한테 보낸다.
        // write(sockfd, cmd_buff, MAX_BUF);
        //서버에서 온 내용을 받는다.
        int listenfd, connfd;
        char *hostport;
        struct sockaddr_in temp;
        int count = 0;
            
        if (!strncmp(cmd_buff, "NLST", 4) || !strncmp(cmd_buff, "RETR", 4) || !strncmp(cmd_buff, "STOR", 4) || !strncmp(cmd_buff, "LIST", 4)){
            srand(time(NULL));
            
            /* make control connection and data connection */
            unsigned int data_port = 10001 + ( rand() % 50000 );

            // printf("%d\n",data_port);
            struct sockaddr_in serv_addr, cliaddr;

            listenfd = socket(AF_INET, SOCK_STREAM, 0); //sock을 만든다.
            
            bzero((char*)&serv_addr, sizeof(serv_addr)); // 서버 주소 구조체를 초기화한다.
            serv_addr.sin_family = AF_INET; //주소 체계를 인터넷 프로토콜로 설정
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP주소를 설정 
            serv_addr.sin_port = htons(data_port);// 포트 번호를 입력으로 받아온다.


            if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ //sock과 server 관련 정보를 합친다.
                printf("Server: Can't bind local address.\n");
                return 1;
            }

            int clilen = sizeof(cliaddr);
            listen(listenfd, 5);
            int client_len = sizeof(cliaddr);

            bzero((char*)&buf, MAX_BUF); // buf 0으로 초기화
            hostport = convert_addr_to_str(servaddr.sin_addr.s_addr, serv_addr.sin_port);
            printf("converting to %s\n", hostport);
            
            bzero((char*)&buf, MAX_BUF); // buf 0으로 초기화
            snprintf(buf, MAX_BUF, "PORT %s", hostport);
            write(sockfd, buf, MAX_BUF); // PORT 명령어 client한테 보냄
            
            bzero(message_buff, MAX_BUF);
            read(sockfd, message_buff, MAX_BUF); // message를 받아놓는다.
            write(STDOUT_FILENO, message_buff, MAX_BUF);

            // //여기서 server처럼 행동 
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &client_len);
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }
            
            
            //서버에게 변환된 FTP 명령어(cmd_buff)를 클라이언트한테 보낸다.
            write(sockfd, cmd_buff, MAX_BUF);   
            //200
            bzero(message_buff, MAX_BUF);
            read(sockfd, message_buff, MAX_BUF); // message를 받아놓는다.
            write(STDOUT_FILENO, message_buff, MAX_BUF);


            if(!strncmp(cmd_buff, "STOR", 4)){
                bzero(buf, MAX_BUF);
                read(sockfd, message_buff, MAX_BUF); // message를 받아놓는다.
                int file_fd = open(filename, O_RDONLY);
                // 파일을 읽고 클라이언트에게 전송
                count += read(file_fd, buf, MAX_BUF);
                write(sockfd, buf, MAX_BUF);
            }
            if (!strncmp(cmd_buff, "RETR", 4)){
                int file_fd = open(filename, O_WRONLY| O_CREAT | O_TRUNC);
                // 파일을 읽고 클라이언트에게 전송
                count += read(sockfd, buf, MAX_BUF);
                write(file_fd, buf, MAX_BUF);
            }

            else{}

            //150
            bzero(message_buff, MAX_BUF);
            read(sockfd, message_buff, MAX_BUF); // message를 받아놓는다.
            write(STDOUT_FILENO, message_buff, MAX_BUF);

            if (!strncmp(cmd_buff, "NLST", 4)|| !strncmp(cmd_buff, "LIST", 4) || !strncmp(cmd_buff, "RETR", 4) || !strncmp(cmd_buff, "STOR", 4)){
                bzero(buf, MAX_BUF);
                count += read(sockfd, buf, MAX_BUF); // message를 받아놓는다.
                write(STDOUT_FILENO, buf, MAX_BUF);
            }


            //226 
            bzero(message_buff, MAX_BUF);
            read(sockfd, message_buff, MAX_BUF); // message를 받아놓는다.
            write(STDOUT_FILENO, message_buff, MAX_BUF);

            sprintf(message_buff, "OK. %d bytes is received\n", count);
            write(STDOUT_FILENO, message_buff, MAX_BUF);
            
            count = 0;
            close(connfd);
            close(listenfd);
        }

        else{
            //서버에게 변환된 FTP 명령어(cmd_buff)를 클라이언트한테 보낸다.
            write(sockfd, cmd_buff, MAX_BUF);
            bzero(message_buff, MAX_BUF);
            read(sockfd, message_buff, MAX_BUF); // message를 받아놓는다.
            write(STDOUT_FILENO, message_buff, MAX_BUF);
            // if ((n = read(sockfd, rcv_buff, sizeof(rcv_buff))) < 0)
            // {
            //     write(STDERR_FILENO, "read() error\n", sizeof("read() error\n"));
            //     exit(1);  //프로그램 종료
            // }
            // rcv_buff[n] = '\0';
            if (!strcmp(cmd_buff, "QUIT"))
            {
                close(sockfd);
                // write(STDOUT_FILENO, "Program quit!!\n", sizeof("Program quit!!\n"));
                exit(1);  //프로그램 종료
            }
            else if (!strcmp(cmd_buff, "Q"))
            {
                close(sockfd);
                exit(1);  //프로그램 종료
            }
        }

        if(strcmp(message_buff, "201 Type set to A.") == 0){
            bin_sign = 0;
        }
        else if(strcmp(message_buff, "201 Type set to I.") == 0){
            bin_sign = 1;
        }
        else{  }
        
        write(STDOUT_FILENO, "> ", 2);
    }
    close(sockfd);
}

void log_in(int sockfd){
    int n;
    char user[MAX_BUF], *passwd;

    /* 코드 작성 (hint: Check if the ip is acceptable ) */
    n = read(sockfd, buf, MAX_BUF); //IP 신호가 잘 들어왔는지 확인
    buf[n] = '\0'; //마지막에 null문자 없을 수도 있기 때문에 쓴다.
    if(!strcmp(buf, "IP OK")){
        bzero(buf, MAX_BUF);
        read(sockfd, buf, MAX_BUF); 
        write(STDOUT_FILENO, buf, MAX_BUF);
    }
    else{
        bzero(buf, MAX_BUF);
        read(sockfd, buf, MAX_BUF); 
        write(STDOUT_FILENO, buf, MAX_BUF);
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
        char user[60];
        strncpy(user, buf,60);
        write(sockfd, buf, 60); //server에게 보낸다.

        bzero(buf, MAX_BUF);
        n = read(sockfd, buf, MAX_BUF);
        // write(STDOUT_FILENO, buf, MAX_BUF);

        if(!strcmp(buf, "ID OK")){ //한번 확인
            bzero(buf, MAX_BUF);
            read(sockfd, buf, MAX_BUF); 
            write(STDOUT_FILENO, buf, MAX_BUF);

            //buf를 초기화한다.
            bzero(buf, sizeof(buf));
            
            //패스워드를 적으라고 한다.
            write(STDOUT_FILENO, "Input Password : ", sizeof("Input Password : "));
            // 에코 모드를 비활성화한다.
            disable_echo_mode();
            read_password(buf, MAX_BUF);
            // 에코 모드를 활성화한다.
            enable_echo_mode();

            write(STDOUT_FILENO, "\n", 1); // 비밀번호 입력 후 줄 바꿈


            // 개행문자를 널 문자로 바꾼다.
            for(int i = 0; i < MAX_BUF; i++) {
                if(buf[i] == '\n') {
                    buf[i] = '\0';
                    break;
                }
            }
            write(sockfd, buf, 60); //server에게 보낸다.
            bzero(buf, MAX_BUF);
            n = read(sockfd, buf, MAX_BUF); //온 데이터 확인
            buf[n] = '\0'; //마지막에 null문자 없을 수도 있기 때문에 쓴다.
            if(!strcmp(buf, "OK")) { //아이디, 비번 모두 맞다.
                bzero(buf, MAX_BUF);
                read(sockfd, buf, MAX_BUF); 
                write(STDOUT_FILENO, buf, MAX_BUF);
                sign = 1;
                return;
            }
            else if(!strcmp(buf, "FAIL")){ //틀렸다.
                bzero(buf, MAX_BUF);
                read(sockfd, buf, MAX_BUF); 
                write(STDOUT_FILENO, buf, MAX_BUF);
                //틀렸다는 신호를 보내야한다.
            }
            else{ // 3번 모두 틀렸다.
                bzero(buf, MAX_BUF);
                read(sockfd, buf, MAX_BUF); 
                write(STDOUT_FILENO, buf, MAX_BUF);
                close(sockfd);
                exit(0);
            }
        }
        else if(!strcmp(buf, "ID NO")){ //한번 확인{
            bzero(buf, MAX_BUF);
            read(sockfd, buf, MAX_BUF); 
            write(STDOUT_FILENO, buf, MAX_BUF);
            //틀렸다는 신호를 보내야한다.
        }
        else{ // 3번 모두 틀렸다.
            bzero(buf, MAX_BUF);
            read(sockfd, buf, MAX_BUF); 
            write(STDOUT_FILENO, buf, MAX_BUF);
            close(sockfd);
            exit(0);
        }
    }
}

int conv_cmd(char* buf,char* cmd_buff){
    if(strncmp(buf, "cd ..", (int)strlen("cd ..") ) == 0) {
        strcpy(cmd_buff, "CDUP"); //맞으면 quit를 QUIT로 바꾼다.
        return 1;
    }

    char *ptr= strtok(buf, " "); //buff에 있는 리눅스 명령어를 FTP 명령어로 바꾼다.
    if(ptr == NULL)
        return -1;

    if( strncmp(ptr, "ls", (int)strlen("ls")) == 0){ //ls 명령어가 아니면 오류를 보내게 한다.
        strncpy(cmd_buff, "NLST",4); //맞으면 ls부분을 NLST로 바꾼다.
    }

    else if(strncmp(ptr, "dir", (int)strlen("dir") ) == 0) {
        strcpy(cmd_buff, "LIST"); //맞으면 dir를 LIST로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "pwd", (int)strlen("pwd") ) == 0) {
        strcpy(cmd_buff, "PWD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "cd", (int)strlen("cd") ) == 0) {
            strcpy(cmd_buff, "CWD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "mkdir", (int)strlen("mkdir") ) == 0) {
        strcpy(cmd_buff, "MKD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "delete", (int)strlen("delete") ) == 0) {
        strcpy(cmd_buff, "DELE"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "rmdir", (int)strlen("rmdir") ) == 0) {
        strcpy(cmd_buff, "RMD"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "bin", (int)strlen("bin") ) == 0) {
        strcpy(cmd_buff, "TYPE I"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "ascii", (int)strlen("ascii") ) == 0) {
        strcpy(cmd_buff, "TYPE A"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "get", (int)strlen("get") ) == 0) {
        strcpy(cmd_buff, "RETR"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
        ptr = strtok(NULL, " ");
        strcpy(filename ,ptr);
        strcat(cmd_buff," ");
        strcat(cmd_buff, ptr);
    }
    else if(strncmp(ptr, "put", (int)strlen("put") ) == 0) {
        strcpy(cmd_buff, "STOR"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
        ptr = strtok(NULL, " ");
        strcpy(filename ,ptr);
        strcat(cmd_buff," ");
        strcat(cmd_buff, ptr);   
    }
    else if(strncmp(ptr, "rename", (int)strlen("rename") ) == 0) {
        strcpy(cmd_buff, "RNFR "); //맞으면 quit를 QUIT로 바꾼다.
        ptr = strtok(NULL, " ");
        strcat(cmd_buff, ptr);
        strcat(cmd_buff, " ");
        strcat(cmd_buff, "RNTO ");
        ptr = strtok(NULL, " ");
        strcat(cmd_buff, ptr);
        //널문자이면 실행하지 않는다.
    }
    else if(strncmp(ptr, "quit", (int)strlen("quit") ) == 0) {
        strcpy(cmd_buff, "QUIT"); //맞으면 quit를 QUIT로 바꾼다.
            //널문자이면 실행하지 않는다.
    }
    else{
        return -1;
    }
    while(ptr != NULL){
        ptr = strtok(NULL, " ");
        if (ptr == NULL) //널문자를 만나면 빠져나온다.
            break;
        
        strcat(cmd_buff, " "); // 한칸 띄우고
        strcat(cmd_buff, ptr); // 그 다음 문자열을 넣는다.
    }
    return 1;
}

//서버에게 받은 rcv_buff 내용을 출력하는 함수
void process_result(char* rcv_buff){
    //100씩 끊어서 출력한다. 왜냐면 100씩 끊어서 서버에서 보냈기 때문이다.
    for(int i = 0 ; i < RCV_BUFF; i+=100){
        write(STDOUT_FILENO, rcv_buff+i , 100);
    }
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