%{
	#include "strop.h"
	#include "stack.h"

	//stack_t *obj;
	cfg_t *cfg;
	string buf;

	int cnt = 0;
	int row = 0;
	int col = 0;

	vec3_t vert;
	face_t face;
%}

%option noyywrap nounput noinput yylineno
%x VERT FACE

FLT -?[0-9]*[.][0-9]*
INT -?[0-9]+
%%

<INITIAL>v[ ]		{ BEGIN(VERT); row = 0; }
<VERT>{FLT}				{
							//if (col == 0) cnt++;
							//printf("ln: %d, [%d|%d]: '%s'\n", yylineno, cnt, col, yytext);
							vert.x[col] = strtof(yytext, NULL);
							col++;	
							if (col >= 3) {
								col = 0;
								cfg->v->push(cfg->v, &vert);
								BEGIN(INITIAL);
							}
						}
<VERT>\n				{
							//col = 0; BEGIN(INITIAL);
						}
<INITIAL>f[ ]		{ BEGIN(FACE); col = 0; row = 0; }
<FACE>{INT}			{
						cnt++;
						//printf("ln: %d, cnt: %d, [%d|%d]: '%s'\n", yylineno, cnt, row, col, yytext);
						face.m[row][col] = atoi(yytext);						
						row++;
						if (row >= 3) {
							row = 0;
							col++;
							if (col >= 4) {
								col = 0;
								//printf("pushed face\n");
								cfg->f->push(cfg->f, &face);
								BEGIN(INITIAL);
							}
						}
					}

<*>.					//ECHO;
<*>(\ |\n|\t)			{ }

%%

cfg_t *parse_obj(char *file_in) {
	cfg = new_cfg();
	init_string(&buf);

	yyin = fopen(file_in, "r");
	if (yyin == NULL) {
		printf("Could not open file: %s\n", file_in);
		return NULL;
	}
	yylex();

	return cfg;
}
