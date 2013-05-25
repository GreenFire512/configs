#define _BSD_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

char *tzparis = "Europe/Moscow";

static Display *dpy;
/*
char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *buf = NULL;

	va_start(fmtargs, fmt);
	if (vasprintf(&buf, fmt, fmtargs) == -1){
		fprintf(stderr, "malloc vasprintf\n");
		exit(1);
    }
	va_end(fmtargs);

	return buf;
}
*/
char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf(buf);
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}


char *
up() {
    struct sysinfo info;
    int h,m = 0;
    sysinfo(&info);
    h = info.uptime/3600;
    m = (info.uptime - h*3600 )/60;
    return smprintf("%dh %dm",h,m);
}

/*
char*
runcmd(char* cmd) {
	FILE* fp = popen(cmd, "r");
	if (fp == NULL) return NULL;
	char ln[30];
	fgets(ln, sizeof(ln)-1, fp);
	pclose(fp);
	ln[strlen(ln)-1]='\0';
	return smprintf("%s", ln);
}

int
getvolume() {
	int volume;
        sscanf(runcmd("amixer | grep -A 6 Headphone \
			| grep -o '[0-9%]*%'"), "%i%%", &volume);
	return volume;
}

void
setvolume(int percent) {
	char volcmd[32];
	sprintf(volcmd, "amixer set Headphone %i%%", percent);
	system(volcmd);
}
*/



int
main(int argc, char *argv[])
{
	char *status = NULL;
	char *tmprs = NULL;
	char *avgs = NULL;
	char *uptm = NULL;
	//int volume;
    
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}
	
	/* checks every second */
	for (;;sleep(1)) { 
            tmprs = mktimes("%H:%M:%S %d %h %Y", tzparis);
            uptm = up();
	    avgs = loadavg();
	    //volume = getvolume();
	    status = smprintf("Load: %s | Up: %s | Date: %s", avgs, uptm, tmprs);
	    setstatus(status);
	    free(avgs);
	    free(status);
	    free(tmprs);
            free(uptm);
	}

	XCloseDisplay(dpy);

	return 0;
}

