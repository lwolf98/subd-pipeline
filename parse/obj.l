%{
	#include "obj.tab.h"
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
%x CMT NAME

OP_O "o "
OP_F "f "
OP_VT "vt "
OP_VN "vn "
OP_V "v "
FLT -?[0-9]*[.][0-9]*
INT -?[0-9]+
STRING [^\"<]*
%%

<INITIAL>"#"		{ BEGIN(CMT); }
<CMT>\n				{ BEGIN(INITIAL); }
<INITIAL>{OP_O}		{ BEGIN(NAME); return stm_name; }
<INITIAL>{OP_VT}	{ return stm_tc; }
<INITIAL>{OP_VN}	{ return stm_normal; }
<INITIAL>{OP_V}		{ return stm_vertex; }
<INITIAL>{OP_F}		{ return stm_face; }

<INITIAL>{INT}		{ yylval.dec = atoi(yytext); return val_int; }
<INITIAL>{FLT}		{ yylval.flt = strtof(yytext, NULL); return val_float; }
<NAME>{STRING}		{ BEGIN(INITIAL); yylval.str = yytext; return val_string; }

<*>(\ |\n|\t)			{ }
<INITIAL>.		{ return *yytext; }

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
