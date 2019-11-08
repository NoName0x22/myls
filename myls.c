#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <grp.h>


#define S_IFREG  0100000  /* regular */
#define S_IFDIR  0040000  /* directory */
#define S_IFLNK  0120000  /* symbolic link */
#define S_IFBLK  0060000  /* block special */
#define S_IFCHR  0020000  /* character special */
#define S_IFIFO  0010000  /* named pipe (fifo) */

/* Struktura Przechowujaca Dane o Pojednycznym Pliku ! */
typedef struct{
	char nazwa[255];
	struct stat dokumentstat;
}plik;

/* */
char* nazwaUzytkownika(unsigned int uid)
{
    struct passwd *pws;
    pws = getpwuid(uid);
        return pws->pw_name;
}

char* nazwaGrupy(unsigned int uid)
{
    struct group *grupa;
    grupa = getgrgid(uid);
        return grupa->gr_name;
}

/* Funkcja Porownujaca na Potrzeby Uzytego Qsorta */
int porownanie(const void *a, const void*b){
	const plik *pa = (const plik *)a;
	const plik *pb = (const plik *)b;
	
	return(strcmp(pa->nazwa,pb->nazwa));

}

/*Funkcja Zwracająca typ pliku */
static int filetypeletter(unsigned int mode)
{
    char c;

    if (mode & S_IFREG)
        c = '-';
    else if (mode & S_IFDIR)
        c = 'd';
    else if (mode & S_IFBLK)
        c = 'b';
    else if (mode & S_IFCHR)
        c = 'c';
    else if (mode & S_IFIFO)
        c = 'p';
    else if (mode & S_IFLNK)
        c = 'l';
    else
    {
       
        c = '?';
    }
    return(c);
}


/* Funkcja Konwertująca st_mode pliku na czytelną wersję  */
void uprawnienia(unsigned int Prawa){
	char buffer[25];
	char typpliku = filetypeletter(Prawa);
	sprintf(buffer,"%c%c%c%c%c%c%c%c%c%c",typpliku,
			(Prawa & S_IRUSR ? 'r' : '-'),
			(Prawa & S_IWUSR ? 'w' : '-'),
			(Prawa & S_IXUSR ? 'x' : '-'),
			(Prawa & S_IRGRP ? 'r' : '-'),
			(Prawa & S_IWGRP ? 'w' : '-'),
			(Prawa & S_IXGRP ? 'x' : '-'),
			(Prawa & S_IROTH ? 'r' : '-'),
			(Prawa & S_IWOTH ? 'w' : '-'),
			(Prawa & S_IXOTH ? 'x' : '-'));
	printf("%s", buffer);
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
	
	/* Pętla Zliczająca Ilość Plików W Katalogu */
	for(;;){
		direntp = readdir(dirp);
		if(direntp == NULL) break;
		iloscplikow ++;	
	}
	
	rewinddir(dirp); 
	/* Dynamiczna Alokcja Pamięci dla Tablocy Zawierającej Informacje o Plikach */
    tablicaPlikow = malloc(sizeof(plik) * iloscplikow);
	
	
	/* Pętla Zapisująca Informacje o Plikach do Struktury */
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
		uprawnienia(tablicaPlikow[i].dokumentstat.st_mode),
		printf(" %ld %s %s %s\n",
		tablicaPlikow[i].dokumentstat.st_nlink,
		nazwaUzytkownika(tablicaPlikow[i].dokumentstat.st_uid),
		nazwaGrupy(tablicaPlikow[i].dokumentstat.st_gid),
		tablicaPlikow[i].nazwa);
	
	}
	

	free(tablicaPlikow);
	closedir(dirp);
	return 0;
}
