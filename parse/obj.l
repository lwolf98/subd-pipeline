%{
	#include "obj.tab.h"
	#include "stack.h"
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

<INITIAL>#.*\n		{ /* do nothing */ }
<INITIAL>{OP_O}		{ BEGIN(NAME); return stm_name; }
<INITIAL>{OP_VT}	{ return stm_tc; }
<INITIAL>{OP_VN}	{ return stm_normal; }
<INITIAL>{OP_V}		{ return stm_vertex; }
<INITIAL>{OP_F}		{ return stm_face; }

<INITIAL>"mtllib ".*\n	{ /* ignore for now */ }
<INITIAL>"usemtl ".*\n	{ /* ignore for now */ }
<INITIAL>"s ".*\n	{ /* ignore for now */ }

<INITIAL>{INT}		{ yylval.dec = atoi(yytext); return val_int; }
<INITIAL>{FLT}		{ yylval.flt = strtof(yytext, NULL); return val_float; }
<NAME>{STRING}		{
						BEGIN(INITIAL);
						yylval.str = yytext;
						return val_string;
					}

<INITIAL>(\ |\n|\t)		{ /* ignore blanks */ }
<INITIAL>.				{
							/* pass any other character */
							return *yytext;
						}

%%

// C code...
