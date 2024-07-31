 ///////////////////////////////////////////////////////////////////////
 // File Name : srv.c //
 // Date : 2024/04/014 //
 // OS : Ubuntu 20.04.6 LTS 64bits
 //
 // Author : Kim You Chan //
 // Student ID : 2022202104 //
 // -----------------------------------------------------------------//
 // Title: System Programming Assignment #1-3 ( ftp server )   //
 // Description : client에서 받은 명령어로 sever에서 실행한다. //
 ///////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUFFER_SIZE 1024 // 버퍼 크기를 정의하는 상수
#define SEND_SIZE 200 //system call로 사용하는 수신 크기

char output[1024];  //출력 버퍼
int length;         //sfprint에서 나오는 문자열의 길이

///////////////////////////////////////////////////////////////////////
// print_file_information //
// ==================================================================
// Input: struct dirent *dirp -> Directory entry to be printed
//           struct stat *buf -> File status information          
// Output: None 
// Purpose: To print detailed information about a directory or file
///////////////////////////////////////////////////////////////////////

// 파일 정보를 출력하는 함수
void print_file_information(struct dirent *dirp, struct stat *buf) {
    struct passwd *pwd = getpwuid(buf->st_uid); // 파일 소유자의 사용자 이름을 가져옵니다.
    struct group *grp = getgrgid(buf->st_gid);  // 파일 소유자의 그룹 이름을 가져옵니다.

    char time_str[256];
    // 파일의 마지막 수정 시간을 문자열로 변환합니다.
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&buf->st_mtime));

    // 파일의 권한 및 속성을 문자열로 구성하여 출력합니다.
    length = sprintf(output, (S_ISDIR(buf->st_mode)) ? "d" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IRUSR) ? "r" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IWUSR) ? "w" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IXUSR) ? "x" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IRGRP) ? "r" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IWGRP) ? "w" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IXGRP) ? "x" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IROTH) ? "r" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IWOTH) ? "w" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, (buf->st_mode & S_IXOTH) ? "x" : "-");
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, " %2ld", buf->st_nlink); // 연결된 하드 링크의 수
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, " %s", pwd ? pwd->pw_name : "unknown"); // 소유자 이름
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, " %s", grp ? grp->gr_name : "unknown"); // 그룹 이름
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, " %6ld", buf->st_size); // 파일 크기
    write(STDOUT_FILENO, output, length);
    length = sprintf(output, " %s", time_str); // 마지막 수정 시간
    write(STDOUT_FILENO, output, length);
    
    // 디렉터리일 경우 디렉터리 이름 뒤에 슬래시(/)를 붙여 출력
    if(S_ISDIR(buf->st_mode)){
        length = sprintf(output, " %s/\n", dirp->d_name);
        write(STDOUT_FILENO, output, length);  
    }
    else{
        length = sprintf(output, " %s\n", dirp->d_name);
        write(STDOUT_FILENO, output, length);
    }
}

int main (){
	
    char buffer[BUFFER_SIZE]; // 버퍼 크기를 지정하는 상수
    ssize_t bytesRead;
	char *cvalue = NULL; // 'c' 옵션의 문자열을 저장하기 위한 포인터

	read(0, buffer, SEND_SIZE); // 표준 입력으로부터 SEND_SIZE만큼 데이터를 읽어 buffer에 저장
	int argc = atoi(buffer); // 읽어온 데이터를 정수로 변환하여 argc에 저장

	int numStrings = SEND_SIZE;
    char **argv = malloc(numStrings * sizeof(char*)); // 문자열 포인터 배열을 동적 할당
    for (int i = 0; i < numStrings; i++) {
        argv[i] = malloc(256 * sizeof(char)); // 각 문자열에 256 바이트 할당
    }

	// 입력된 모든 명령어를 argv 배열에 저장
	int i = 0;
	
	while(i < argc){
		bytesRead = read(0, buffer, SEND_SIZE); // 표준 입력으로부터 데이터 읽기
		if(bytesRead == 0) break; // 더 이상 읽을 데이터가 없으면 반복 중단
		strcpy(argv[i], buffer); // 읽은 데이터를 argv 배열의 해당 인덱스에 복사
		i++;
	}
	
	int c=0;
	char instructions[20] = ""; // 수행할 명령어를 저장할 배열
	opterr = 0; // getopt 함수의 에러 메시지 출력 설정, 0이면 출력 안 함
	if(strcmp(argv[1], "QUIT")==0){ // 첫 번째 인수가 "QUIT"인 경우
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?': // 옵션 문자가 잘못된 경우
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc > 2){ // 인수가 너무 많은 경우
			length = sprintf(output, "Error: argument is not required\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		length = sprintf(output, "QUIT success\n"); // QUIT 명령 성공 메시지 출력
		write(STDOUT_FILENO, output, length);
		return 0;
	}
	else if(strcmp(argv[1], "CWD")==0){ // "CWD" 명령 처리
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc > 3){
			length = sprintf(output, "Error: only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		if(chdir(argv[2]) == 0){ // 디렉토리 변경 시도
			char* buffer;
			buffer = malloc(256);

			getcwd(buffer, 256); // 현재 디렉토리 경로 가져오기
			length = sprintf(output, "%s %s\n", argv[1], argv[2]);
			write(STDOUT_FILENO, output, length);
			length = sprintf(output, "\"%s\" is current directory\n\n", buffer);
			write(STDOUT_FILENO, output, length);
			free(buffer);

			return 0;
		}
		else{
			length = sprintf(output, "Error: directory not found\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		
	}
	else if(strcmp(argv[1], "CDUP")==0){ // "CDUP" 명령을 처리하는 조건문
		while ((c = getopt (argc, argv, "")) != -1){ // getopt를 이용하여 옵션을 파싱
			switch (c){
				case '?': // 옵션 문자가 잘못된 경우 에러 메시지 출력
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc > 3){ // "CDUP" 명령은 인수가 3개를 초과하면 안 되므로 에러 처리
			length = sprintf(output, "Error: only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		if(chdir(argv[2]) == 0){ // 상위 디렉토리로 이동 시도
			char* buffer;
			buffer = malloc(256);

			// 현재 디렉토리 경로 저장을 위한 버퍼 준비
			char paths[256];
			strcpy(paths, argv[2]); // argv 값을 미리 paths에 저장

			getcwd(buffer, 256); // 현재 디렉토리 경로를 얻어와서 buffer에 저장
			length = sprintf(output, "%s\n", argv[1]);
			write(STDOUT_FILENO, output, length);
			length = sprintf(output, "\"%s\" is current directory\n\n", buffer);
			write(STDOUT_FILENO, output, length);
			free(buffer);

			return 0;
		}
		else{
			length = sprintf(output, "Error: directory not found\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
	}
	else if(strcmp(argv[1], "PWD")==0){ // "PWD" 명령어 처리 (Print Working Directory)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?': // 옵션 오류 처리
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc > 2){ // "PWD" 명령은 추가 인수를 필요로 하지 않음
			length = sprintf(output, "Error: argument is not required\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		char* buffer;
		buffer = malloc(256);

		getcwd(buffer, 256); // 현재 디렉토리 경로를 buffer에 저장
		length = sprintf(output, "\"%s\" is current directory\n\n", buffer);
		write(STDOUT_FILENO, output, length);
		free(buffer);

		return 0;
	}
	else if(strcmp(argv[1], "MKD")==0){ // "MKD" 명령어 처리 (Make Directory)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?': // 잘못된 옵션이 주어진 경우
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc < 3){ // 인수가 최소 하나 필요함 (생성할 디렉토리 이름)
			length = sprintf(output, "Error: argument is required\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		for(int i = 2; i < argc; i++){ // 모든 주어진 디렉토리 이름에 대해 반복
			if(mkdir(argv[i], 0604) == 0){ // 디렉토리 생성 시도, 성공 시 메시지 출력
				length = sprintf(output, "%s %s\n", argv[1], argv[i]);
				write(STDOUT_FILENO, output, length);
			}
			else{ // 실패 시 에러 메시지 출력
				length = sprintf(output, "Error: cannot create directory \'%s\': File exists\n", argv[i]);
				write(STDOUT_FILENO, output, length);
			}
		}
		length = sprintf(output, "\n");
		write(STDOUT_FILENO, output, length);
		return 0;
	}
	else if(strcmp(argv[1], "DELE")==0){ // "DELE" 명령어 처리 (Delete File)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc < 3){ // 파일 삭제 명령은 하나 이상의 인수가 필요함
			length = sprintf(output, "Error: argument is required\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		for(int i = 2; i < argc; i++){ // 주어진 파일 이름에 대해 반복
			if(unlink(argv[i]) == 0){ // 파일 삭제 시도, 성공 시 메시지 출력
				length = sprintf(output, "%s %s\n", argv[1], argv[i]);
				write(STDOUT_FILENO, output, length);
			}
			else{ // 실패 시 에러 메시지 출력
				length = sprintf(output, "Error: failed to delete \'%s\'\n", argv[i]);
				write(STDOUT_FILENO, output, length);
			}
		}
		length = sprintf(output, "\n");
		write(STDOUT_FILENO, output, length);
		return 0;
	}
	else if(strcmp(argv[1], "RMD")==0){ // "RMD" 명령어 처리 (Remove Directory)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc < 3){ // 디렉토리 삭제 명령은 하나 이상의 인수가 필요함
			length = sprintf(output, "Error: argument is required\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		for(int i = 2; i < argc; i++){ // 주어진 디렉토리 이름에 대해 반복
			if(rmdir(argv[i]) == 0){ // 디렉토리 삭제 시도, 성공 시 메시지 출력
				length = sprintf(output, "%s %s\n", argv[1], argv[i]);
				write(STDOUT_FILENO, output, length);
			}
			else{ // 실패 시 에러 메시지 출력
				length = sprintf(output, "Error: failed to remove \'%s\'\n", argv[i]);
				write(STDOUT_FILENO, output, length);
			}
		}
		length = sprintf(output, "\n");
		write(STDOUT_FILENO, output, length);
		return 0;
	}
	else if(strcmp(argv[1], "RNFR")==0){ // "RNFR" 명령어 처리 (Rename From)
		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n");
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		if(argc != 5){ // 이름 변경 명령은 정확히 두 개의 인수가 필요함 (원본 및 대상 이름)
			length = sprintf(output, "Error: two arguments are required\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}

		struct stat buf;
		int isFile = stat(argv[4], &buf)+1; // 대상 파일의 존재 여부 확인
		if(isFile){ // 대상 파일이 이미 존재하는 경우
			length = sprintf(output, "Error: name to change already exists\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		if(rename(argv[2], argv[4]) == 0){ // 파일 이름 변경 시도, 성공 시 메시지 출력
			length = sprintf(output, "%s %s\n", argv[2], argv[4]);
			write(STDOUT_FILENO, output, length);
		}
		else{ // 실패 시 에러 메시지 출력
			length = sprintf(output, "Error: failed to rename");
			write(STDOUT_FILENO, output, length);
		}
		length = sprintf(output, "\n");
		write(STDOUT_FILENO, output, length);
		return 0;
	}
	else if(strcmp(argv[1], "NLST")==0){
		int aflag = 0, lflag = 0;
		strcat(instructions, argv[1]);
		int a = 2;
		while(argv[a][0] == '-'){
			strcat(instructions, " ");
			strcat(instructions, argv[a]);
			a++;
		}

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
					return 1;
			}
		}
		int index = optind; //외부 전역함수를 함부로 바꾸면 안 되기 때문에 index라는 변수에 저장했다. 또한 이제부터는 옵션을 고려하지 않기 때문에 optind를 굳이 안 써도 된다.
		index++;

		if(argv[index+1][0] != '\0'){
			length = sprintf(output, "only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		length = sprintf(output, "%s\n", instructions);
		write(STDOUT_FILENO, output, length);
		if(aflag == 0 && lflag ==0){ // 아무런 옵션이 없는 겨우
			struct stat buf; 

			DIR *dp = NULL;
			struct dirent **dirp = 0;
			int n = 0;
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				stat(argv[index], &buf); //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.
				int count = 0;
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					if(count == 4){
						length = sprintf(output, "\n");
						write(STDOUT_FILENO, output, length);
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(output, "%s/     ", dirp[i]->d_name);	
						write(STDOUT_FILENO, output, length);
					}
					else{
						length = sprintf(output, "%s     ", dirp[i]->d_name);
						write(STDOUT_FILENO, output, length);
					}
				}
				length = sprintf(output, "\n");			
				write(STDOUT_FILENO, output, length);
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
				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					if(count == 4){
						length = sprintf(output, "\n");
						write(STDOUT_FILENO, output, length);
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(output, "%s/     ", dirp[i]->d_name);	
						write(STDOUT_FILENO, output, length);
					}
					else{
						length = sprintf(output, "%s     ", dirp[i]->d_name);
						write(STDOUT_FILENO, output, length);
					}
				}
				length = sprintf(output, "\n");			
				write(STDOUT_FILENO, output, length);
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
			if(argv[index][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				stat(argv[index], &buf); //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.
				int count = 0;
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					if(count == 4){
						length = sprintf(output, "\n");
						write(STDOUT_FILENO, output, length);
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(output, "%s/     ", dirp[i]->d_name);	
						write(STDOUT_FILENO, output, length);
					}
					else{
						length = sprintf(output, "%s     ", dirp[i]->d_name);
						write(STDOUT_FILENO, output, length);
					}
				}
				length = sprintf(output, "\n");			
				write(STDOUT_FILENO, output, length);
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
				for (int i = 0; i < n; i++) {
					if(count == 4){
						length = sprintf(output, "\n");
						write(STDOUT_FILENO, output, length);
						count = 0;
					}
					count++;
					stat(dirp[i]->d_name, &buf);
					if(S_ISDIR(buf.st_mode)){
						length = sprintf(output, "%s/     ", dirp[i]->d_name);	
						write(STDOUT_FILENO, output, length);
					}
					else{
						length = sprintf(output, "%s     ", dirp[i]->d_name);
						write(STDOUT_FILENO, output, length);
					}
				}
				length = sprintf(output, "\n");			
				write(STDOUT_FILENO, output, length);
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
			if(argv[index][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				directory = ".";
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					if(dirp[i]->d_name[0] == '.' ) continue;
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf);
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
					print_file_information(dirp[i], &buf);
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

			char *directory = argv[index];
			//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
			if(argv[index][0] == '\0'){
				dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
				directory = ".";
				n = scandir(".", &dirp, NULL, alphasort);
				// 정렬된 파일 이름을 출력한다.
				for (int i = 0; i < n; i++) {
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					stat(path, &buf);
					print_file_information(dirp[i], &buf);
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
					snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
					// length = sprintf(output, "%s\n", path);
					stat(path, &buf);
					print_file_information(dirp[i], &buf);
				}		
        		closedir(dp);
				return 0;
			}
			return 0;
		}
	}
	else if(strcmp(argv[1], "LIST")==0){ // "LIST" 명령어를 처리하는 부분

		strcat(instructions, argv[1]);
		int a = 2;
		while(argv[a][0] == '-'){
			strcat(instructions, " ");
			strcat(instructions, argv[a]);
			a++;
		}

		while ((c = getopt (argc, argv, "")) != -1){
			switch (c){
				case '?':
					length = sprintf(output, "Error: invalid option\n\n"); //다른 옵션이 나오면 출력된다.
					write(STDOUT_FILENO, output, length);
					return 1;
			}
		}
		int index = optind; //외부 전역함수를 함부로 바꾸면 안 되기 때문에 index라는 변수에 저장했다. 또한 이제부터는 옵션을 고려하지 않기 때문에 optind를 굳이 안 써도 된다.
		index++;

		if(argv[index+1][0] != '\0'){
			length = sprintf(output, "only one directory path can be processed\n\n");
			write(STDOUT_FILENO, output, length);
			return 1;
		}
		length = sprintf(output, "%s\n", instructions);
		write(STDOUT_FILENO, output, length);

		struct stat buf;

		DIR *dp = NULL;
		struct dirent **dirp = 0;
		int n = 0;
		char path[1024]; 

		char *directory = argv[index];
		//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
		if(argv[index][0] == '\0'){
			dp = opendir("."); //디렉토리 주소를 가져온다. 실패하면 null을 반환
			directory = ".";
			n = scandir(".", &dirp, NULL, alphasort);
			// 정렬된 파일 이름을 출력한다.
			for (int i = 0; i < n; i++) {
				snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
				stat(path, &buf);
				print_file_information(dirp[i], &buf);
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
			closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
		}
		//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
		else{
			length = sprintf(output, "%s\n", argv[index]);
			write(STDOUT_FILENO, output, length);
			n = scandir(argv[index], &dirp, NULL, alphasort);
			// 정렬된 파일 이름을 출력한다.
			for (int i = 0; i < n; i++) {
				snprintf(path, sizeof(path), "%s/%s", directory, dirp[i]->d_name);
				// length = sprintf(output, "%s\n", path);
				stat(path, &buf);
				print_file_information(dirp[i], &buf);
				
			}		
			closedir(dp);
			return 0;
		}
		return 0;
	}
	else{//없는 명령어 에러출력을 한다.
		length = sprintf(output, "This command does not exist.\n");
		write(STDOUT_FILENO, output, length);
	}

    return 0;
}