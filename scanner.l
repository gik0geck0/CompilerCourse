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
[^a-zA-Z'_-]+          { /* Ignore any other characters... Don't care */  }

%%

void token_handler();

main()
{
	/* token handling code here */
	token_handler();
	return 0;
}

