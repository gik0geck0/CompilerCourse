/***************************************************** 
 * flex .l template
 *

<BOF>
{%
C/C++ includes, decls and defs
%}

flex declarations

%%

RE rules for tokenization 

%% 

C/C++ functions

<EOF>
 *
 ******************************************************/

%{
const int WORD = 1;
%}

%%

[a-zA-Z'_-]+           { return WORD; }

%%

void token_handler();

main()
{
	/* token handling code here */
	token_handler();
	return 0;
}

