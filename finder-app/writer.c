#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    openlog("writerlog", LOG_CONS | LOG_PID, LOG_USER);

    if(argc != 3)
    {
	syslog(LOG_ERR, "Didn't supply all arguments");
        return 1;
    }

    int fileHandle = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0664);

    if(fileHandle == -1)
    {
        syslog(LOG_ERR, "Couldn't open/create file");
	return 1;
    }

    int writecount = write(fileHandle,argv[2],strlen(argv[2]));

    if(writecount != strlen(argv[2]))
    {
        syslog(LOG_ERR, "Failed to write to file");
	return 1;
    }
    
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    
    closelog();

    return 0;
} 
