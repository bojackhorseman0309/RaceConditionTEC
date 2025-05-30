/* vulp.c*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#define DELAY 10000

int are_the_same_file(int fd1, int fd2);

int main()
{
	char * fn = "/tmp/XYZ";
	char buffer[60];
	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	long int i, j, f, k;

	scanf("%50s", buffer );

    // Verifica y obtiene el archivo en distintas variables de tipo FILE
	if (!access(fn, W_OK)) {
		/* simulating delay */
		for (i = 0; i < DELAY; i++) {
			int a = i^2;
		}

		fp = fopen(fn, "a+");
		if (fp == NULL) {
			perror("Error opening file fp");
			return 1;
		}
	}
	else printf("No permission fp \n");

	if (!access(fn, W_OK)) {
		/* simulating delay */
		for (j = 0; j < DELAY; j++) {
			int a = j^2;
		}

		fp2 = fopen(fn, "a+");
		if (fp2 == NULL) {
			perror("Error opening file fp2");
			return 1;
		}
	}
	else printf("No permission fp2 \n");

	if (!access(fn, W_OK)) {
		/* simulating delay */
		for (f = 0; f < DELAY; f++) {
			int a = f^2;
		}

		fp3 = fopen(fn, "a+");
		if (fp3 == NULL) {
			perror("Error opening file fp3");
			return 1;
		}
	}
	else printf("No permission fp3 \n");

	if (!access(fn, W_OK)) {
		/* simulating delay */
		for (k = 0; k < DELAY; k++) {
			int a = k^2;
		}

		fp4 = fopen(fn, "a+");
		if (fp4 == NULL) {
			perror("Error opening file fp4");
			return 1;
		}
	}
	else printf("No permission fp4 \n");

	// Convierte el FILE a su numero de file descriptor
	int fd1 = fileno(fp);
	int fd2 = fileno(fp2);
	int fd3 = fileno(fp3);
	int fd4 = fileno(fp4);

	// Valida si alguno de los descriptores indica que no es valido
	if (fd1 < 0 || fd2 < 0 || fd3 < 0 || fd4 < 0) {
		printf("Existe algun file descriptor invalido \n");
		return 1;
	}

	// Compara los file descriptors para ver si son el mismo archivo
	int is_file1_2 = are_the_same_file(fd1, fd2);
	int is_file3_4 = are_the_same_file(fd3, fd4);
	int is_file1_3 = are_the_same_file(fd1, fd3);

	if (is_file1_2 && is_file3_4 && is_file1_3) {
		fwrite("\n", sizeof(char), 1, fp);
		fwrite(buffer, sizeof(char), strlen(buffer), fp);
		fclose(fp);
		fclose(fp2);
		fclose(fp3);
		fclose(fp4);
	} else {
		printf("Los file descriptors no apuntan al mismo archivo \n");
	}
}

int are_the_same_file(int fd1, int fd2) {
	// Se usa fstat para obtener los atributos del archivo
	struct stat stat1, stat2;
	if (fstat(fd1, &stat1) < 0 || fstat(fd2, &stat2) < 0) {
		printf("No se pudo obtener un file stat correctamente \n");
		return 0;
	}
	// Compara los inodes y id de dispositivos
	return (stat1.st_ino == stat2.st_ino && stat1.st_dev == stat2.st_dev);
}


