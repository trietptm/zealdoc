%{
        #define YYSTYPE char *          /* define semantic values */
        #include <math.h>
        #include <stdio.h>
        #include <ctype.h>
	#include <exp.h>		/* export exp_main() */
	#include <number.h>
	#include <shell.h>
	#include <string.h>

	#define EXP_MAX_LENGTH	1024

        int yylex (void);
        void yyerror (char const *);

	char g_exp[EXP_MAX_LENGTH];
	char *g_pexp = g_exp;
%}

%token NUM
%left '-' '+'   /* left-associative */

%left '*' '/'
%left NEG       /* negative */
%right '^'      /* right-associative */

%%
input:          /* empty line */
        | input line
;

line:     '\n'  /* ignore '\n' line */
        | exp '\n'  { printf ("\t%s\n", $1); } /* valid exp, print it */
;

exp:      NUM                /* { $$ = $1;         } */
        | exp '+' exp	{ $$ = shell_add($1, $3); }
        | exp '-' exp	{ $$ = shell_sub($1, $3); }
	/*
        | exp '*' exp        { $$ = $1 * $3;    }
        | exp '/' exp        { $$ = $1 / $3;    }
        | '-' exp  %prec NEG { $$ = -$2;        }
        | exp '^' exp        { $$ = pow ($1, $3); }
        | '(' exp ')'        { $$ = $2;         }
	*/
;
%%

int exp_main (void)
{
	yyparse ();
	return 0;
}

int yylex (void)
{
	yylval = (YYSTYPE)sh_get_exp();

	if (yylval) {
		if (yylval[0] == '.' || isdigit(yylval[0])) {
			printf("exp=%s\n", yylval);
                	return NUM;
		}
		/* sign */
		return yylval[0];
	}

        return 0;
}

void yyerror (char const *s)
{
        fprintf (stderr, "%s\n", s);
}

