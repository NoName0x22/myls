#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


typedef struct{
	struct dirent* dokument;
	struct stat dokumentstat;
}plik;

int porownanie(const void *a, const void*b){
	const plik *pa = (const plik *)a;
	const plik *pb = (const plik *)b;
	
	return(strcmp(pa->dokument->d_name,pb->dokument->d_name));

}

int main(){
	DIR* dirp;
	struct dirent* direntp;
	struct stat filestat;
	plik* tablicaPlikow;
	int iloscplikow = 0;
	int i = 0; 
	if((dirp = opendir(".")) == NULL){
		perror("Nastapil Blad !!\n");
		return 1;
	}
	
	for(;;){
		direntp = readdir(dirp);
		if(direntp == NULL) break;
		iloscplikow ++;	
	}
	
	rewinddir(dirp); 
        tablicaPlikow = malloc(sizeof(plik) * iloscplikow);
	
	for(;;){
		direntp = readdir(dirp);
		if(direntp == NULL) break;
		stat(direntp->d_name,&filestat);
		tablicaPlikow[i].dokument = direntp;
		tablicaPlikow[i].dokumentstat = filestat;
		i++;
	}
	
	qsort(tablicaPlikow,iloscplikow,sizeof(plik),porownanie);

	for(i = 0; i < iloscplikow; i++){
		printf("%u %s\n",tablicaPlikow[i].dokumentstat.st_mode,tablicaPlikow[i].dokument->d_name);
	
	}
	

	free(tablicaPlikow);
	closedir(dirp);
	return 0;
}
