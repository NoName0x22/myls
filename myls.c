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

/* Funkcja Zwracajaca Nazwę Właściciela jako Parametr Przyjmuje Jego ID */
char* nazwaUzytkownika(unsigned int uid)
{
    struct passwd *pws;
    pws = getpwuid(uid);
        return pws->pw_name;
}

/* Funkcja Zwracajaca Nazwę Grupy jako Parametr Przyjmuje Jej ID */
char* nazwaGrupy(unsigned int uid)
{
    struct group *grupa;
    grupa = getgrgid(uid);
        return grupa->gr_name;
}

/* Funkcja Porownujaca na Potrzeby Uzytego Qsorta sortującego w porządku alfabetycznym*/
int porownanie(const void *a, const void*b){
	const plik *pa = (const plik *)a;
	const plik *pb = (const plik *)b;
	
	return(strcmp(pa->nazwa,pb->nazwa));

}

/*Funkcja Zwracająca typ pliku */
static int znakTypuPliku(unsigned int mode)
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
	char typpliku = znakTypuPliku(Prawa);
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


void pobierzMaksSzerokoscKolumn(plik tablica[], int rozmiar, int szerokosc[]){
	int tempszerokosc[2] = {0,0}; 
	int i = 0;
	int makspierwsza = (int)tablica[0].dokumentstat.st_nlink;
	int maksdruga = (int)tablica[0].dokumentstat.st_size;
	

	for(i = 1; i < rozmiar; i++){
		if(makspierwsza < (int)tablica[i].dokumentstat.st_nlink){
			makspierwsza = (int)tablica[i].dokumentstat.st_nlink;
		
		}
	}
	
	for(i = 1; i < rozmiar; i++){
		if(maksdruga < (int)tablica[i].dokumentstat.st_size){
			maksdruga = (int)tablica[i].dokumentstat.st_size;
		
		}
	}
	
	while(makspierwsza != 0){
		tempszerokosc[0] ++;
		makspierwsza /= 10;
	}
	
	while(maksdruga != 0){
		tempszerokosc[1] ++;
		maksdruga /= 10;
	}
	szerokosc[0] = tempszerokosc[0];
	szerokosc[1] = tempszerokosc[1];
}


int main(int args, char* argv[]){
	DIR* dirp;
	struct dirent* direntp;
	struct stat filestat;
	plik* tablicaPlikow;
	int iloscplikow = 0;
	int i = 0;
	int szerokoscKolumn[2];
	
	
	
	if(args == 1){
		
		if((dirp = opendir(".")) == NULL){
			perror("Nastapil Blad !!\n");
			return 1;
		}
		
		/* Pętla Zliczająca Ilość Plików W Katalogu */
		for(;;){
			direntp = readdir(dirp);
			if(direntp == NULL) break;
			if(strcmp(direntp->d_name,".") == 0) continue;
			if(strcmp(direntp->d_name,"..") == 0) continue;
			if(direntp->d_name[0] == '.') continue;
			iloscplikow ++;	
		}
		
		rewinddir(dirp); 
		/* Dynamiczna Alokcja Pamięci dla Tablocy Zawierającej Informacje o Plikach */
		tablicaPlikow = malloc(sizeof(plik) * iloscplikow);
		
		
		/* Pętla Zapisująca Informacje o Plikach do Struktury */
		for(;;){
			direntp = readdir(dirp);
			if(direntp == NULL) break;
			if(strcmp(direntp->d_name,".") == 0) continue;
			if(strcmp(direntp->d_name,"..") == 0) continue;
			if(direntp->d_name[0] == '.') continue;
			stat(direntp->d_name,&filestat);
			tablicaPlikow[i].dokumentstat = filestat;
			sprintf(tablicaPlikow[i].nazwa, "%s", direntp->d_name);
			i++;
		}
		
		qsort(tablicaPlikow,iloscplikow,sizeof(plik),porownanie); /* QSort Uzyty do Posortowania Nazw Plikow W Porzadku Alfabetycznym !! */
		pobierzMaksSzerokoscKolumn(tablicaPlikow,iloscplikow,szerokoscKolumn);
		
		for(i = 0; i < iloscplikow; i++){
			uprawnienia(tablicaPlikow[i].dokumentstat.st_mode),
			printf(" %*ld %s  %s %*ld %s\n",szerokoscKolumn[0],
			(unsigned long)tablicaPlikow[i].dokumentstat.st_nlink,
			nazwaUzytkownika(tablicaPlikow[i].dokumentstat.st_uid),
			nazwaGrupy(tablicaPlikow[i].dokumentstat.st_gid),
			szerokoscKolumn[1],
			tablicaPlikow[i].dokumentstat.st_size,
			tablicaPlikow[i].nazwa);
		
		}
		
			free(tablicaPlikow);
			closedir(dirp);
	}
	

	return 0;
}
