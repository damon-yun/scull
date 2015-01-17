#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
	FILE *fp;
	char buf[128] = "123";

	fp = fopen("/dev/myprintf","w+");
	if (!fp) {
		printf("/dev/myprintf open error\n");
		exit(1);
	}
	printf("/dev/myprintf open success\n");
	fputs("Hello,world\n",fp);
	fseek(fp,0,SEEK_SET);
	fgets(buf,128,fp);
	puts(buf);
	
	memset(buf,0,128);
	fseek(fp,0,SEEK_SET);
	fputs("Hi,every one\n",fp);
	fseek(fp,0,SEEK_SET);
	fgets(buf,128,fp);
	puts(buf);

	memset(buf,0,128);
	fgets(buf,128,fp);
	puts(buf);

	fclose(fp);
	return 0;
}
