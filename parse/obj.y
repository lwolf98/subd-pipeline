%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <math.h>
	#include "stack.h"
	#include "strop.h"

	cfg_t *cfg;
	face_t face;
	stack_t *v_buffer;

	int yylex(void);
	extern int yylineno;
	extern FILE *yyin;

	int yydebug = 1;
	void yyerror (const char *msg) {
		// Do not exit program on syntax errors
		fprintf(stderr, "ERR: Line: %d:\n\t%s\n\n", yylineno, msg);
	}

	void add_v_cfg(int v, int vt, int vn);
%}

%define parse.error verbose

%union {
	char *str;
	int dec;
	float flt;
}

%token stm_name stm_vertex stm_normal stm_tc stm_face stm_mtllib stm_usemtl stm_crease
<str> val_string
<dec> val_int
<flt> val_float

%type	<flt>	NUMBER

%%

START:				STATEMENT_LIST

STATEMENT_LIST:		STATEMENT_LIST STATEMENT
				|	/* epsilon */

STATEMENT:			stm_name val_string		{
												string buf;
												init_string(&buf);
												buf.append(&buf, $2);
												cfg->name = buf.get(&buf);
											}
				|	stm_mtllib val_string	{
												string buf;
												init_string(&buf);
												buf.append(&buf, $2);
												cfg->mtllib_path = buf.get(&buf);
											}
				|	stm_usemtl val_string	{
												string buf;
												init_string(&buf);
												buf.append(&buf, $2);
												cfg->material = buf.get(&buf);
											}
				|	stm_tc VT_ARGS
				|	stm_normal VN_ARGS
				|	stm_vertex V_ARGS
				|	stm_face F_ARGS			{
												// push vertex list to face config
												cfg->f->push(cfg->f, v_buffer);

												// reset vertex list
												v_buffer = new_stack(sizeof(int *));
											}
				|	stm_crease CREASE_ARGS

VT_ARGS:			NUMBER NUMBER				{
														vec2_t tc;
														tc.x[0] = $1;
														tc.x[1] = $2;
														cfg->vt->push(cfg->vt, &tc);
													}
VN_ARGS:			NUMBER NUMBER NUMBER	{
														vec3_t normal;
														normal.x[0] = $1;
														normal.x[1] = $2;
														normal.x[2] = $3;
														cfg->vn->push(cfg->vn, &normal);
													}
V_ARGS:				NUMBER NUMBER NUMBER	{
														vec3_t vert;
														vert.x[0] = $1;
														vert.x[1] = $2;
														vert.x[2] = $3;
														cfg->v->push(cfg->v, &vert);
													}
NUMBER:				val_float
				|	val_int							{ $$ = val_int; }
F_ARGS:				F_ARGS_1
				|	F_ARGS_2
				|	F_ARGS_3
				|	F_ARGS_4
F_ARGS_1:			F_ARGS_1 val_int							{ add_v_cfg($2, -1, -1); }
				|	val_int										{ add_v_cfg($1, -1, -1); }
F_ARGS_2:			F_ARGS_2 val_int '/' val_int				{ add_v_cfg($2, $4, -1); }
				|	val_int '/' val_int							{ add_v_cfg($1, $3, -1); }
F_ARGS_3:			F_ARGS_3 val_int '/' '/' val_int			{ add_v_cfg($2, -1, $5); }
				|	val_int '/' '/' val_int						{ add_v_cfg($1, -1, $4); }
F_ARGS_4:			F_ARGS_4 val_int '/' val_int '/' val_int	{ add_v_cfg($2, $4, $6); }
				|	val_int '/' val_int '/' val_int				{ add_v_cfg($1, $3, $5); }
CREASE_ARGS:	val_int val_int val_float		{
													float sharpness = $3;
													float sharp_floor = floor(sharpness);
													if (sharpness > 0.f) {
														if (sharpness != sharp_floor)
															sharpness = sharpness - sharp_floor;
														else
															sharpness = 1.f;
													}
													crease_t c = {$1, $2, sharpness};
													cfg->creases->push(cfg->creases, &c);
												}

%%

cfg_t *parse_obj(char *file_in) {
	v_buffer = new_stack(sizeof(int *));
	string source;
	init_string(&source);
	source.append(&source, file_in);
	cfg = new_cfg(source.get(&source));

	yyin = fopen(file_in, "r");
	if (yyin == NULL) {
		printf("Could not open file: %s\n", file_in);
		return NULL;
	}
	yyparse();

	return cfg;
}

void add_v_cfg(int v, int vt, int vn) {
	int *vert = (int *)malloc(3*sizeof(int));
	vert[0] = v, vert[1] = vt, vert[2] = vn;
	v_buffer->push(v_buffer, &vert);
}
