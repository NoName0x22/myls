#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


/* Struktura Przechowujaca Dane o Pojednycznym Pliku ! */
typedef struct{
	char nazwa[255];
	struct stat dokumentstat;
}plik;


/* Funkcja Porownujaca na Potrzeby Uzytego Qsorta */
int porownanie(const void *a, const void*b){
	const plik *pa = (const plik *)a;
	const plik *pb = (const plik *)b;
	
	return(strcmp(pa->nazwa,pb->nazwa));

}

/* Funkcja Konwertująca st_mode pliku na czytelną wersję  */

/*char *uprawnienia(unsigned int Prawa){
	char buffer[25];
	char typpliku;
	if(Prawa & S_IFREG){
		typpliku = '-';
	}
	if(Prawa & S_IFDIR){
		typpliku = 'd';
	}
	if(Prawa & S_IFLNK){
		typpliku = 'l';
	}
	if(Prawa & S_IFIFO){
		typpliku = 'p';
	}
	if(Prawa & S_IFSOCK){
		typpliku = 's';
	}
	if(Prawa & S_IFCHR){
		typpliku = 'c';
	}if(Prawa & S_IFBLK){
		typpliku = 'b';
	}
	
	sprintf(buffer,"%c %c %c %c %c %c %c %c %c %c",typpliku,
			(Prawa & S_IRUSR ? 'r' : '-'),
			(Prawa & S_IWUSR ? 'w' : '-'),
			(Prawa & S_IXUSR ? 'x' : '-'),
			(Prawa & S_IRGRP ? 'x' : '-'),
			(Prawa & S_IWGRP ? 'x' : '-'),
			(Prawa & S_IXGRP ? 'x' : '-'),
			(Prawa & S_IROTH ? 'x' : '-'),
			(Prawa & S_IWOTH ? 'x' : '-'),
			(Prawa & S_IXOTH ? 'x' : '-'));
	return buffer;
}*/


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
		tablicaPlikow[i].dokumentstat = filestat;
		sprintf(tablicaPlikow[i].nazwa, "%s", direntp->d_name);
		i++;
	}
	
	qsort(tablicaPlikow,iloscplikow,sizeof(plik),porownanie); /* QSort Uzyty do Posortowania Nazw Plikow W Porzadku Alfabetycznym !! */

	for(i = 0; i < iloscplikow; i++){
		printf("%u %s\n",tablicaPlikow[i].dokumentstat.st_mode,tablicaPlikow[i].nazwa);
	
	}
	

	free(tablicaPlikow);
	closedir(dirp);
	return 0;
}
