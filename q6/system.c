#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef WIN32
#define popen _popen
#define pclose _plose
#endif

int main(int argc, char* argv[]) {
	char Com[100];
	size_t rSize = 0;
	FILE *fp = NULL;
	char Buff[1024];
	
	sprintf(Com, "%s %s", argv[1], argv[2]);
	fp = popen(Com, "r");

	if (!fp) {
		printf("error[%d: %s]\n", errno, strerror(errno));
		return -1;
	}

	rSize = fread((void*)Buff,sizeof(char),1024-1, fp);

	if (rSize == 0) {
		pclose(fp);
		printf("error[%d: %s]\n", errno, strerror(errno));
		return -1;
	}
	
	pclose(fp);
	Buff[rSize] = 0;

	printf("%s\n", Buff);
	return 0;
}
