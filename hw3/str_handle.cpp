#include <stdlib.h>
#include <string.h>
char** strtok_command(const char *str,const char *d){ //use " " to  split command
	int num=0;
	const char *delim=d;
	char copy[256];
	strcpy(copy,str);
	char *p=strtok(copy,delim);
	while(p!=NULL){
		num++;
		p=strtok(NULL,delim);
	}
	char **argv=(char**) malloc((num+1)*sizeof(char*));
	int i;
	for(i=0;i<num;i++) argv[i]=(char*) malloc(256);
	num=0;
	strcpy(copy,str);
	p=strtok(copy,delim);
	while(p!=NULL){
		strcpy(argv[num],p);
		p=strtok(NULL,delim);
		num++;
	}
	argv[num]=(char*)0;
	return argv;
}