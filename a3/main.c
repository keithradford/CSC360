#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void diskinfo();
void disklist();
void diskget();
void diskput();

int main(int argc, char const *argv[]){
	if(strcmp(argv[0], "./diskinfo") == 0)
		diskinfo();
	else if(strcmp(argv[0], "./disklist") == 0)
		disklist();
	else if(strcmp(argv[0], "./diskget") == 0)
		diskget();
	else if(strcmp(argv[0],  "./diskput") == 0)
		diskput();
	else{
		printf("Error: Executable must be one of:\ndiskinfo\ndisklist\ndiskget\ndiskput\n");
		return 0;
	}

	return 1;
}

void diskinfo(){

}

void disklist(){
	
}

void diskget(){
	
}

void diskput(){
	
}