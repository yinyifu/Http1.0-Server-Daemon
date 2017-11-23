#ifndef __DEBUG_H_YIFU__
#define __DEBUG_H_YIFU__
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <algorithm>

#define BOLD "\e[1m"
#define BOLDOFF "\e[22m"
#define ITALIC "\e[3m"
#define ITALICOFF "\e[23m"
#define UNDERLINE "[4m"
#define UNDERLINEOFF "[24m"
#define INVERSE "[7m"
#define INVERSEOFF "[27m"
#define STRIKE "[9m"
#define STRIKEOFF "[29m"

#define BLACKFORE "[30m"
#define REDFORE "[31m"
#define GREENFORE "[32m"
#define YELLOWFORE "[33m"
#define BLUEFORE "[34m"
#define MAGENTAFORE "[35m"
#define CYANFORE "[36m"
#define WHITEFORE "[37m"
#define DEFAULTFORE "[39m"

#define BLACKBACK "[40m"
#define REDBACK "[41m"
#define GREENBACK "[42m"
#define YELLOWBACK "[43m"
#define BLUEBACK "[44m"
#define MAGENTABACK "[45m"
#define CYANBACK "[46m"
#define WHITEBACK "[47m"
#define DEFAULTBACK "[49m"

#define RESET "[0m"

#define NAME "NAME"
#define SYNOPSIS "SYNOPSIS"
#define DESCRIPTION "DESCRIPTION"
#define OPTIONS "OPTIONS"
#define CHUNKWRAP 50


# define EMPTY(...)
# define DEFER(...) __VA_ARGS__ EMPTY()

#define ERRORMSG(TYPE)({printf("Error -------> %s failed, %s::%s at line %d\n",TYPE,__FILE__,__func__,__LINE__);})
#define ASSERT(COND)({if(!(COND)){ERRORMSG("Assertion");exit(-1);}})
#define ERRORIF(COND, MSG)({if(COND){perror(MSG);exit(-1);}})

#define pr_id() TOUPPERS
#define random() pr_id()
#define TOUPPER(C) ((char)(islower(C)?C-32:C))
#define TOUPPERS(STR) ([&](char * STR){int len = strlen(STR);char *upperstr=new char[len+1]; std::memset(upperstr, 0, len+1);std::transform(STR, STR+len, upperstr, ::toupper); return upperstr;})(STR)

#define STRCAT_3(STR_1, STR_2, STR_3) ([&](const char * STR1, const char * STR2, const char * STR3){int len1 = strlen(STR1);int len2 = strlen(STR2);int len3 = strlen(STR3);char *newstr=new char[len1+len2+len3+1]; std::memset(newstr, 0, len1+len2+len3+1);std::strcat(newstr, STR1);strcat(newstr, STR2);strcat(newstr, STR3); return newstr;})(STR_1, STR_2, STR_3)
#define TOBOLD(STR) STRCAT_3(BOLD, STR, BOLDOFF)
#define TOITALIC(STR) STRCAT_3(ITALIC, STR, ITALICOFF)

#define LENGTH(ARRAY) ARRAY==NULL?0:sizeof(ARRAY)/sizeof(ARRAY[0])

//#define max(a,b) ({__type_of__(a) _a = a; __type_of__(b) _b = b; _a>_b? _a: _b})

/* progname is the program name.
*. the array contains description at index 0
*. and other options at */
void
displayChunk(std::string chunk){
	int chunklen = chunk.length();
	int chunks = chunklen/CHUNKWRAP+1;
	std::string strs[chunks];
	int lastPoint = 0;
	int j;

	for(int i = 0; i < chunks; i++){
		j = (i+1)*CHUNKWRAP;
		if(j>=chunklen){
			j=chunklen;
		}else{
			for(;chunk.at(j)!=' '&&j<chunklen-1; ++j);
			j++;
		}
		std::cout << '\t' << chunk.substr(lastPoint, j-lastPoint) <<std::endl;
		lastPoint = j;
	}
	std::cout << std::endl ;
	
}

void
usage(char* progname, std::string usageinfo[], int arrlen)
{
	ERRORIF((arrlen < 3)|| (arrlen%2 ==0), "Usage function bad argument");
	// Header
	std::cout <<'\f'<<std::endl<< TOUPPERS(progname) << "(1)                Usage Guide                " << TOUPPERS(progname) << "(1)\n" << std::endl;
	
	// Name
	std::cout << TOBOLD(NAME) << std::endl ;
	displayChunk(usageinfo[0]);
	
	// SYNOPSIS
	std::cout << TOBOLD(SYNOPSIS) << std::endl;
	displayChunk(usageinfo[1]);

	//description
	std::cout << TOBOLD(DESCRIPTION) << std::endl;
	displayChunk(usageinfo[2]);

	//options
	std::cout << TOBOLD(OPTIONS) << std::endl;
	for(int i = 3; i < arrlen-2; i+=2){
		std::cout << TOBOLD(usageinfo[i].c_str());
		displayChunk(usageinfo[i+1]);
	}
	exit(1);
}


#endif
