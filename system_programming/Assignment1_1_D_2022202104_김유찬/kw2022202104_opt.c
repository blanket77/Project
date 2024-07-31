 ///////////////////////////////////////////////////////////////////////
 // File Name : kw2022202104_opt.c //
 // Date : 2024/03/29 //
 // OS : Ubuntu 20.04.6 LTS 64bits
 //
 // Author : Kim You Chan //
 // Student ID : 2022202104 //
 // -----------------------------------------------------------------//
 // Title: System Programming Assignment #1-1 ( ftp server )   //
 // Description : option도 함께 입력 받아 작성할 수 있게 한다. //
 ///////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <stdio.h>
int main (int argc, char **argv)
{
	int aflag = 0, bflag = 0;
	char *cvalue = NULL; //c옵션 문자열을 입력받기 위해 출력
	int c=0;
	opterr = 0; //이 값이 0이 아니면  에러시 발생 메세지가 출력된다. 0이면 에러시 메세지가 출력되지 않는다.  defalut =0  
	//옵션과 문자열을 끝까지 확인한다.
	while ((c = getopt (argc, argv, "abc:")) != -1) //c:이기 때문에 c 뒤 문자열을 optarg로 받을 수 있다. getopt 함수는 전달된 인수를 옵션에 따라 구별할 수 있도록 한 함수이다.
	{
		switch (c)
		{
			case 'a': //옵션이 a일 때 
				aflag++; //aflage 값이 1추가된다. 
				break;
			case 'b'://옵션이 b일 때
				bflag++; //bflage 값이 1추가된다.
				break;
			case 'c'://옵션이 c일 때
				cvalue = optarg; //-c옵션 뒤에 있는 문자열을 optarg이 담겨있다.
				break;
			case '?':
                printf("Unknow option character\n"); //다른 옵션이 나오면 출력된다.
                break;
                }
    }
		
	printf("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue); // 잘 작동하는지 출력해본다.

	//optind는 옵션에 붙여있는 것까지 고려하여 입력 배열의 index를 알려주기 때문에 optind를 써야한다.  
	int index = optind; //외부 전역함수를 함부로 바꾸면 안 되기 때문에 index라는 변수에 저장했다. 또한 이제부터는 옵션을 고려하지 않기 때문에 optind를 굳이 안 써도 된다.
	while(argv[index] != NULL) //더이상 입력값이 없으면 NULL이기 때문에 NULL까지 반복한다.
		printf("Non-option argument %s \n", argv[index++] ); // 옵션을 제외한 나머지 입력값을 차례로 출력한다.
	return 0;
}
