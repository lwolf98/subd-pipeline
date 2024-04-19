%{
	#include "obj.tab.h"
	#include "stack.h"

	int show_dbgl = 0;
	void dbgl(char *msg);
%}

%option noyywrap nounput noinput yylineno
%x NAME

OP_O "o "
OP_F "f "
OP_VT "vt "
OP_VN "vn "
OP_V "v "
FLT -?[0-9]*[.][0-9]*([e]-?[0-9]+)?
INT -?[0-9]+
STRING [^\"<\n]*
%%

<INITIAL>#.*\n		{ printf("comment detected\n"); }
<INITIAL>{OP_O}		{ BEGIN(NAME); printf("stm_name\n"); return stm_name; }
<INITIAL>{OP_VT}	{ printf("stm_tc\n"); return stm_tc; }
<INITIAL>{OP_VN}	{ printf("stm_normal\n"); return stm_normal; }
<INITIAL>{OP_V}		{ printf("stm_vertex\n"); return stm_vertex; }
<INITIAL>{OP_F}		{ printf("stm_face\n"); return stm_face; }

<INITIAL>"mtllib ".*\n	{ /* ignore for now */ printf("ignore mtllib\n"); }
<INITIAL>"usemtl ".*\n	{ /* ignore for now */ printf("ignore usemtl\n"); }
<INITIAL>"s ".*\n	{ /* ignore for now */ printf("ignore smoothing\n"); }

<INITIAL>{INT}		{ yylval.dec = atoi(yytext); printf("val_int\n"); return val_int; }
<INITIAL>{FLT}		{ yylval.flt = strtof(yytext, NULL); printf("val_float\n"); return val_float; }
<NAME>{STRING}		{
						BEGIN(INITIAL);
						yylval.str = yytext;
						printf("val_string "); printf("%s", yytext); printf("\n");
						return val_string;
					}

<INITIAL>(\ |\n|\t)		{ printf("blank\n"); }
<INITIAL>.				{ printf("ANY\n"); return *yytext; }

%%

void dbgl(char *msg) {
	if (show_dbgl)
		printf("[%d L] %s", yylineno, msg);
}
