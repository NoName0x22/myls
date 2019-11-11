#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <time.h>
#include <linux/limits.h>
#include <stdio.h>
#include <fcntl.h>



/*
 * ZNANE BUGI 
 * 1. Błędne Wyswietlania Sciezki na Plik wskazywany przez Link dla ./myls plik w przypadku gdy odczytywany link znajduje się w innym katalogu;
 * 2. Wyświetlanie Początkowej Zawartości 80 znaków nie zawsze działa poprawnie.
 * */



const char * miesiace[12] = {"sty", "lut", "mar", "kwi", "maj", "cze", "lip", "sie", "wrz", "paz", "lis", "gru"};

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

/*Funkcja Umożliwiająca Konwersję z Liczby Sekund z st_ctime na Czytelną Wersję */
char* dataOdCzasu(char* str, time_t val)
{		time_t aktualny_czas;
		time(&aktualny_czas);
		if(aktualny_czas - val < 15778463){
			if(localtime(&val)->tm_min < 10){ /* Trywialny Sposób Dodawania Zera przed Minutami mniejszymi od 10*/
				sprintf(str, "%2d %3.3s %d:%02d", localtime(&val)->tm_mday, miesiace[localtime(&val)->tm_mon], localtime(&val)->tm_hour, localtime(&val)->tm_min); 
			}else{
				sprintf(str, "%2d %3.3s %d:%d", localtime(&val)->tm_mday, miesiace[localtime(&val)->tm_mon], localtime(&val)->tm_hour, localtime(&val)->tm_min); 
			}
		}else{
		sprintf(str, "%2d %3.3s %5d", localtime(&val)->tm_mday, miesiace[localtime(&val)->tm_mon], localtime(&val)->tm_year+1900); 
		}
        return str;
}

/*Funkcja Zwracająca typ pliku */
static int znakTypuPliku(unsigned int mode)
{
    char c;

    if (S_ISREG(mode))
        c = '-';
    else if (S_ISDIR(mode))
        c = 'd';
    else if (S_ISBLK(mode))
        c = 'b';
    else if (S_ISCHR(mode))
        c = 'c';
    else if (S_ISFIFO(mode))
        c = 'p';
    else if (S_ISLNK(mode))
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
			printf("%s",buffer);
}

/* Funkcja Pobierająca Maksymalną szerokość Wyświetlanych Pól aby móc poźniej ustalić ich szerokość przy wyświetlaniu */
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
	char data[36];
	
	
	
	if(args == 1){
		
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
			if((lstat(direntp->d_name,&filestat)) == -1){
				perror("Lstat");
				return 1;
			}
			tablicaPlikow[i].dokumentstat = filestat;
			sprintf(tablicaPlikow[i].nazwa, "%s", direntp->d_name);
			i++;
		}
		
		printf("\n");
		
		qsort(tablicaPlikow,iloscplikow,sizeof(plik),porownanie); /* QSort Uzyty do Posortowania Nazw Plikow W Porzadku Alfabetycznym !! */
		pobierzMaksSzerokoscKolumn(tablicaPlikow,iloscplikow,szerokoscKolumn); 
		
		for(i = 0; i < iloscplikow; i++){
			if(!S_ISLNK(tablicaPlikow[i].dokumentstat.st_mode)){ /* Warunek Sprawdzający Czy Plik Jest Linkem  */
				
				uprawnienia(tablicaPlikow[i].dokumentstat.st_mode),
				printf(" %*ld %s  %s %*ld %s %s\n",szerokoscKolumn[0],
				(unsigned long)tablicaPlikow[i].dokumentstat.st_nlink,
				nazwaUzytkownika(tablicaPlikow[i].dokumentstat.st_uid),
				nazwaGrupy(tablicaPlikow[i].dokumentstat.st_gid),
				szerokoscKolumn[1],
				tablicaPlikow[i].dokumentstat.st_size,
				dataOdCzasu(data,tablicaPlikow[i].dokumentstat.st_ctime),
				tablicaPlikow[i].nazwa);
				
			}else{ /* Blok Pobierania Danych O Linku w Przypadku Gdy Plik Jest linkiem */
				
				char *buf;
				ssize_t nbytes, bufsiz;
				char buffor[255];
				bufsiz = tablicaPlikow[i].dokumentstat.st_size + 1;
				if (tablicaPlikow[i].dokumentstat.st_size == 0){
					bufsiz = PATH_MAX;
				}
				
				buf = malloc(bufsiz);
				if (buf == NULL) {
					perror("malloc");
					exit(EXIT_FAILURE);
				}
				
				nbytes = readlink(tablicaPlikow[i].nazwa, buf, bufsiz);
				if (nbytes == -1) {
					perror("readlink");
					exit(EXIT_FAILURE);
				}
				
				snprintf(buffor,sizeof(buffor),"%s -> %.*s\n", tablicaPlikow[i].nazwa, (int) nbytes, buf);
				
				uprawnienia(tablicaPlikow[i].dokumentstat.st_mode),
				printf(" %*ld %s  %s %*ld %s %s\n",szerokoscKolumn[0],
				(unsigned long)tablicaPlikow[i].dokumentstat.st_nlink,
				nazwaUzytkownika(tablicaPlikow[i].dokumentstat.st_uid),
				nazwaGrupy(tablicaPlikow[i].dokumentstat.st_gid),
				szerokoscKolumn[1],
				tablicaPlikow[i].dokumentstat.st_size,
				dataOdCzasu(data,tablicaPlikow[i].dokumentstat.st_ctime),
				buffor);
				free(buf);
				
			}
		
		}
			free(tablicaPlikow);
			closedir(dirp);
	}else if(args == 2){
			
		struct stat fstat;
		if((lstat(argv[1], &fstat)) == -1){
			perror("lstat");
			return 1; 
		}
		if(S_ISDIR(fstat.st_mode)){
			char sciezka[255];
			realpath(argv[1],sciezka);
			printf("Informacje o\t%s\n", argv[1]);
			printf("Typ Pliku:\tKatalog\n");
			printf("Sciezka :\t%s\n",sciezka);
			printf("Rozmiar:\t%lu\n", fstat.st_size);
			printf("Uprawnienia:\t"); 
			uprawnienia(fstat.st_mode);
			printf("\n");
			printf("Ostatnio Uzywany:\t%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_atime)->tm_year + 1900,
			localtime(&fstat.st_atime)->tm_hour,
			localtime(&fstat.st_atime)->tm_min,
			localtime(&fstat.st_atime)->tm_sec);
			
			printf("Ostatnio Modyfikowany:\t%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_mtime)->tm_year + 1900,
			localtime(&fstat.st_mtime)->tm_hour,
			localtime(&fstat.st_mtime)->tm_min,
			localtime(&fstat.st_mtime)->tm_sec);
			
			printf("Ostatnio Zmieniany Stan:%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_ctime)->tm_year + 1900,
			localtime(&fstat.st_ctime)->tm_hour,
			localtime(&fstat.st_ctime)->tm_min,
			localtime(&fstat.st_ctime)->tm_sec);
			
			
			
			
		}else if(S_ISREG(fstat.st_mode)){
			char sciezka[255];
			int cat;
			char zawartosc[80];
			size_t nbity = sizeof(zawartosc);
			ssize_t odczytanebity;
			realpath(argv[1],sciezka);
			printf("Informacje o\t%s\n", argv[1]);
			printf("Typ Pliku:\tZwykly Plik\n");
			printf("Sciezka :\t%s\n",sciezka);
			printf("Rozmiar:\t%lu\n", fstat.st_size);
			printf("Uprawnienia:\t"); 
			uprawnienia(fstat.st_mode);
			printf("\n");
			printf("Ostatnio Uzywany:\t%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_atime)->tm_year + 1900,
			localtime(&fstat.st_atime)->tm_hour,
			localtime(&fstat.st_atime)->tm_min,
			localtime(&fstat.st_atime)->tm_sec);
			
			printf("Ostatnio Modyfikowany:\t%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_mtime)->tm_year + 1900,
			localtime(&fstat.st_mtime)->tm_hour,
			localtime(&fstat.st_mtime)->tm_min,
			localtime(&fstat.st_mtime)->tm_sec);
			
			printf("Ostatnio Zmieniany Stan:%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_ctime)->tm_year + 1900,
			localtime(&fstat.st_ctime)->tm_hour,
			localtime(&fstat.st_ctime)->tm_min,
			localtime(&fstat.st_ctime)->tm_sec);
			
			if(!(fstat.st_mode & S_IXOTH) && !(fstat.st_mode & S_IXGRP) && !(fstat.st_mode & S_IXUSR)){
				printf("Poczatkowa Zawartosc:\n");
				cat = open(argv[1], O_RDONLY);
				odczytanebity = read(cat, zawartosc, nbity);
				if(odczytanebity != 0){
					printf("%s\n", zawartosc);
				}
				close(cat);
			}
		}else if(S_ISLNK(fstat.st_mode)){
			
			char sciezka[255];
			char *buf;
			ssize_t nbytes, bufsiz;
			char buffor[255];
			char buffor2[255];
			char cwd[255];
			bufsiz = fstat.st_size + 1;
			if((getcwd(cwd,sizeof(cwd))) != NULL){
				snprintf(buffor2,sizeof(buffor2),"\t%s/%s\n",cwd,argv[1]);
			}
			
			realpath(argv[1],sciezka);
			
			if (fstat.st_size == 0){
					bufsiz = PATH_MAX;
				}
				
				buf = malloc(bufsiz);
			
			if (buf == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			
			nbytes = readlink(argv[1], buf, bufsiz);
			if (nbytes == -1) {
				perror("readlink");
				exit(EXIT_FAILURE);
			}
				
			snprintf(buffor,sizeof(buffor),"%s %s",sciezka, buf);
			
			printf("Informacje o %s\n", argv[1]);
			printf("Typ Pliku: \tLink Symboliczny\n");
			printf("Sciezka:%s",buffor2);
			printf("Wskazuje Na: \t%s\n",sciezka);
			printf("Rozmiar: \t%lu\n", fstat.st_size);
			printf("Uprawnienia:\t"); 
			uprawnienia(fstat.st_mode);
			printf("\n");
			printf("Ostatnio Uzywany:\t%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_atime)->tm_year + 1900,
			localtime(&fstat.st_atime)->tm_hour,
			localtime(&fstat.st_atime)->tm_min,
			localtime(&fstat.st_atime)->tm_sec);
			
			printf("Ostatnio Modyfikowany:\t%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_mtime)->tm_year + 1900,
			localtime(&fstat.st_mtime)->tm_hour,
			localtime(&fstat.st_mtime)->tm_min,
			localtime(&fstat.st_mtime)->tm_sec);
			
			printf("Ostatnio Zmieniany Stan:%d %s %d %d:%d:%d\n", 
			localtime(&fstat.st_atime)->tm_mday, 
			miesiace[localtime(&fstat.st_atime)->tm_mon], 
			localtime(&fstat.st_ctime)->tm_year + 1900,
			localtime(&fstat.st_ctime)->tm_hour,
			localtime(&fstat.st_ctime)->tm_min,
			localtime(&fstat.st_ctime)->tm_sec);
			
			free(buf);
		
		}
	
	}else{
		printf("Przyklad Uzycia: ./myls [nazwa_pliku]\n");
		return 1;
	}
	
	

	return 0;
}
