 ///////////////////////////////////////////////////////////////////////
 // File Name : cli.c //
 // Date : 2024/04/14 //
 // OS : Ubuntu 20.04.6 LTS 64bits
 //
 // Author : Kim You Chan //
 // Student ID : 2022202104 //
 // -----------------------------------------------------------------//
 // Title: System Programming Assignment #1-3 ( ftp server )   //
 // Description : cli에서 리눅스 명령어를 FTP 명령어로 바꿔서 srv에 보낸다. //
 ///////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024  // 버퍼 크기를 정의하는 상수
#define SEND_SIZE 200     // 보내는 크기를 정의하는 상수

int main (int argc, char **argv) {

    char buffer[BUFFER_SIZE];  // 데이터 읽기용 버퍼
    ssize_t bytesRead;         // 읽은 바이트 수를 저장하는 변수
    char aa[SEND_SIZE];        // argc를 문자열로 변환하여 저장하기 위한 char 배열
    int idx = 0;               // 명령줄 인수 처리를 추적하는 인덱스

    // 'rename' 명령의 경우 argc를 인위적으로 증가
    if(strcmp(argv[1], "rename") == 0) 
        argc++;

    sprintf(aa, "%d", argc);   // argc(인수 개수)를 문자열로 변환하여 aa에 저장
    write(1, aa, SEND_SIZE);   // 표준 출력(stdout)에 인수 개수 출력

    write(1, argv[idx++], SEND_SIZE); // 첫 번째 인수(명령 자체) 출력
    printf("\n");

    char FTP_instruc[SEND_SIZE] = "";  // FTP 명령을 저장할 버퍼
    
    // 입력 인수에 따라 해당 FTP 명령을 결정
    if(strcmp(argv[idx], "ls") == 0) {
        strcpy(FTP_instruc, "NLST"); //FTP명령으로 변환
        idx++; // 그 다음 인자를 읽기 위해서 
    } else if(strcmp(argv[idx], "dir") == 0) {
        strcpy(FTP_instruc, "LIST");//FTP명령으로 변환
        idx++; // 그 다음 인자를 읽기 위해서
    } else if(strcmp(argv[idx], "pwd") == 0) {
        strcpy(FTP_instruc, "PWD");
        idx++; // 그 다음 인자를 읽기 위해서
    } else if(strcmp(argv[idx], "cd") == 0) {
        if(argc > 2 && strcmp(argv[idx+1], "..") == 0) {
            strcpy(FTP_instruc, "CDUP");//FTP명령으로 변환
            idx++; // 그 다음 인자를 읽기 위해서
        } else {
            strcpy(FTP_instruc, "CWD");
            idx++; // 그 다음 인자를 읽기 위해서
        }
    } else if(strcmp(argv[idx], "mkdir") == 0) {
        strcpy(FTP_instruc, "MKD");//FTP명령으로 변환
        idx++; // 그 다음 인자를 읽기 위해서
    } else if(strcmp(argv[idx], "delete") == 0) {
        strcpy(FTP_instruc, "DELE");//FTP명령으로 변환
        idx++; // 그 다음 인자를 읽기 위해서
    } else if(strcmp(argv[idx], "rmdir") == 0) {
        strcpy(FTP_instruc, "RMD");//FTP명령으로 변환
        idx++; // 그 다음 인자를 읽기 위해서
    } else if(strcmp(argv[idx], "rename") == 0) {
        strcpy(FTP_instruc, "RNFR");//FTP명령으로 변환
        write(1, FTP_instruc, SEND_SIZE);
        write(1, argv[2], SEND_SIZE);
        strcpy(FTP_instruc, "RNTO");//FTP명령으로 변환
        idx = 3;
    } else if(strcmp(argv[idx], "quit") == 0) {
        strcpy(FTP_instruc, "QUIT");//FTP명령으로 변환
        idx++; // 그 다음 인자를 읽기 위해서
    } else {
        printf("명령어가 잘못 되었습니다."); // 다른 명령인 경우 오류 출력
        exit(1); // 시스템 끝내기
    }
    
    write(1, FTP_instruc, SEND_SIZE); // FTP 명령 출력 및 system call로 전달

    // 남은 인수들 다 출력 및 system call로 전달
    for (int i = idx; i < argc; i++) {
        write(1, argv[i], SEND_SIZE);  
	}
    return 0;
}