#include <unistd.h>
extern int capflags;
void parse_args(int argc,char **argv)
{
	int opt;
	while((opt = getopt(argc,argv,"cCpP")) != -1){
			switch(opt){
					case 'c':
					case 'C':
					capflags = 1;
					break;
					case 'p':
					case 'P':
					capflags = 0;
					break;
			}
	}
}
