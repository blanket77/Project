 ///////////////////////////////////////////////////////////////////////
 // File Name : kw2022202104_opt.c //
 // Date : 2024/04/05 //
 // OS : Ubuntu 20.04.6 LTS 64bits
 //
 // Author : Kim You Chan //
 // Student ID : 2022202104 //
 // -----------------------------------------------------------------//
 // Title: System Programming Assignment #1-2 ( ftp server )   //
 // Description : ls을 구현한다. //
 ///////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
int main(int argc, char *argv[]) {

	struct stat buf;

	DIR *dp = NULL;
	struct dirent *dirp = 0;
	char* aa = &argv[0][2]; // 실행파일을 ./을 빼고 읽기 위해서 이렇게 char* aa을 할당했다.

	//입력이 하나면 자기 자신의 폴더에 있는 파일을 출력한다.
	if(argc == 1){
		dp = opendir("./");
		while((dirp=readdir(dp)) != 0) //파일을 읽었으면 그 다음 파일을 가르키고 null을 가리킬 때까지 반복한다.
			printf("%s\n", dirp->d_name); //파일의 이름을 출력한다.
		closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
		return 0;
	}

	//폴더를 2개이상 읽어드리면 1개만 읽을 수 있다고 출력한다.
	if(argc >= 3){
		printf("only one directory path can be processed\n\n");
		return 0;
	}

	dp = opendir(argv[1]); //디렉토리 주소를 가져온다. 실패하면 null을 반환
	int isFile = stat(argv[1], &buf)+1; //파일을 정상적으로 불러오면 isFile에 1를 할당하고 아니면 0을 할당한다. 그러기 위해서 1을 더해야한다.

	//디렉토리이지만 실행권한이 없으면 실행이 되지 않는다.
	if(S_ISDIR(buf.st_mode) && !(buf.st_mode & S_IXUSR)){
		printf("%s: cannot access '%s' : Access denied\n\n", aa,argv[1]);
		closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
	}

	//애초에 파일이 없으면 디렉토리가 아니다. 
	//파일이 있지만 디렉토리가 아니면 디렉토리가 아니다.
	else if(!isFile || (isFile && !S_ISDIR(buf.st_mode))) 
		printf("%s: cannot access '%s' : No such directory\n\n", aa,argv[1]);
	
	//아무런 에러가 없기 때문에 폴더에 있는 파일을 다 출력한다..
	else{		
		while((dirp=readdir(dp)) != 0) //파일을 읽었으면 그 다음 파일을 가르키고 null을 가리킬 때까지 반복한다.
			printf("%s\n", dirp->d_name); //파일의 이름을 출력한다.
		closedir(dp); //디렉토리를 열었기 때문에 닫아야한다.
	}

	return 0;
}
