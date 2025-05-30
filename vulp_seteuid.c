/* vulp_seteuid.c*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define DELAY 10000

int main()
{
	char * fn = "/tmp/XYZ";
	char buffer[60];
	FILE *fp;
	long int i;

	/* get user input */
	scanf("%50s", buffer );

	// Baja el nivel de privilegios del proceso
	uid_t real_user_id = getuid();
	uid_t effective_user_id = geteuid();
    seteuid(real_user_id);
	if (!access(fn, W_OK)) {

		/* simulating delay */
		for (i = 0; i < DELAY; i++) {
			int a = i^2;
		}

		fp = fopen(fn, "a+");
		fwrite("\n", sizeof(char), 1, fp);
		fwrite(buffer, sizeof(char), strlen(buffer), fp);
		fclose(fp);
	}
	else printf("No permission \n");

	// Restaura el nivel de privilegios del proceso
	seteuid(effective_user_id);
}
