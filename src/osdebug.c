#include "fio.h"
#include <stdarg.h>
#include "clib.h"
#include "host.h"

void osDbgPrintf(const char * format, ...) {

	int i;

	va_list v1;
	va_start(v1, format);

	int tmpint;
	char *tmpcharp;

	for(i=0; format[i]; ++i){
		if(format[i]=='%'){
			switch(format[i+1]){
				case '%':
					 break;
				case 'x':
				case 'X':
					tmpint = va_arg(v1, int);
					tmpcharp = utoa(format[i+1]=='x'?"0123456789abcdef":"0123456789ABCDEF",(unsigned)tmpint, 16);
					host_action (SYS_WRITE,1,tmpcharp,strlen(tmpcharp));
					break;
				case 'u':
					tmpint = va_arg(v1, int);
					tmpcharp = utoa("0123456789",(unsigned)tmpint, 10);
					host_action (SYS_WRITE,1,tmpcharp,strlen(tmpcharp));
					break;
				case 'd':
					tmpint = va_arg(v1, int);
					tmpcharp = itoa(format[i+1]=='x'?"0123456789abcdef":"0123456789ABCDEF", tmpint, format[i+1]=='d'?10: 16);
					host_action (SYS_WRITE,1,tmpcharp,strlen(tmpcharp));
					break;
				case 's':
					tmpcharp = va_arg(v1, char *);
					host_action (SYS_WRITE,1,tmpcharp,strlen(tmpcharp));
					break;
			}
			/* Skip the next character */
			++i;
		}else
			host_action (SYS_WRITE,1,format+i,1);
	}

	va_end(v1);
}
