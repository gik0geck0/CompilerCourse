#include <iostream>
#include <iomanip>

extern "C" {

extern "C" char* yytext;

extern "C" int yylex();

void token_handler(void)
{
	unsigned int t;
	while(( t = yylex()) ) {
#if defined(NDEBUG)
		std::cout << std::setw(3) << std::setfill('0') << t << ":/" << yytext << "/" << std::endl;
#endif
	} 
}

} // "C" extern

