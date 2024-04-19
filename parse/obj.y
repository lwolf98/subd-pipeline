%{
	#include <stdio.h>
	#include <stdlib.h>
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
		//int prev_exit = set_exit_on_error(0);
		//error_print(yylineno, msg);
		//set_exit_on_error(prev_exit);

		fprintf(stderr, "ERR: Line: %d:\n\t%s\n\n", yylineno, msg);
	}

	int show_dbgy = 1;
	void dbgy(char *msg);
	void add_v_cfg(int v, int vt, int vn);
%}

%define parse.error verbose

%union {
	char *str;
	int dec;
	float flt;
}

%token stm_name stm_vertex stm_normal stm_tc stm_face
<str> val_string
<dec> val_int
<flt> val_float

%type	<flt>	NUMBER

%%

START:				STATEMENT_LIST				{ /*dbgy("start detected\n");*/ }

STATEMENT_LIST:		STATEMENT_LIST STATEMENT	{ /*dbgy("statement list detected\n");*/ }
				|	/* epsilon */

STATEMENT:			stm_name val_string		{
												string buf;
												init_string(&buf);
												buf.append(&buf, $2);
												cfg->name = buf.get(&buf);
												/*printf("name detected: "); printf("%s\n", $2);*/
											}
				|	stm_tc VT_ARGS			{ printf(" <- tc detected\n"); }
				|	stm_normal VN_ARGS		{ printf(" <- normal detected\n"); }
				|	stm_vertex V_ARGS		{ printf(" <- vertex detected\n"); }
				|	stm_face F_ARGS			{
												/*printf("<- face detected\n");*/
												// push vertex list to face config
												cfg->f->push(cfg->f, v_buffer);
												printf("face info: i:%d\n", cfg->f->index);
												stack_t *v_list = ((stack_t *)cfg->f->storage) + cfg->f->index; //&(((stack_t *)cfg->f->storage)[cfg->f->index]);
												int *out = ((int **)v_list->storage)[v_list->index];
												//int *out = ((int **)v_buffer->storage)[v_buffer->index];
												printf("(%d_%d_%d)\n", out[0], out[1], out[2]);

												//printf("face access: %s\n", v_cfg_to_str(out));
												//printf("face access: %s\n", v_cfg_to_str(((int **)cfg->f->storage)[0]));
												// reset vertex list
												v_buffer = new_stack(sizeof(int *));
											}

VT_ARGS:			NUMBER NUMBER				{
														printf("%f %f", $1, $2);
														vec2_t tc;
														tc.x[0] = $1;
														tc.x[1] = $2;
														cfg->vt->push(cfg->vt, &tc);
													}
VN_ARGS:			NUMBER NUMBER NUMBER	{
														printf("%f %f %f", $1, $2, $3);
														vec3_t normal;
														normal.x[0] = $1;
														normal.x[1] = $2;
														normal.x[2] = $3;
														cfg->vn->push(cfg->vn, &normal);
													}
V_ARGS:				NUMBER NUMBER NUMBER	{
														printf("%f %f %f", $1, $2, $3);
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
F_ARGS_1:			F_ARGS_1 val_int							{ add_v_cfg($2, -1, -1); printf("%d ", $2); }
				|	val_int										{ add_v_cfg($1, -1, -1); printf("%d ", $1); }
F_ARGS_2:			F_ARGS_2 val_int '/' val_int				{ add_v_cfg($2, $4, -1); printf("%d_%d ", $2, $4); }
				|	val_int '/' val_int							{ add_v_cfg($1, $3, -1); printf("%d_%d ", $1, $3); }
F_ARGS_3:			F_ARGS_3 val_int '/' '/' val_int			{ add_v_cfg($2, -1, $5); printf("%d__%d ", $2, $5); }
				|	val_int '/' '/' val_int						{ add_v_cfg($1, -1, $4); printf("%d__%d ", $1, $4); }
F_ARGS_4:			F_ARGS_4 val_int '/' val_int '/' val_int	{
																	printf("%d_%d_%d ", $2, $4, $6);
																	add_v_cfg($2, $4, $6);
																}
				|	val_int '/' val_int '/' val_int				{
																	printf("%d_%d_%d ", $1, $3, $5);
																	add_v_cfg($1, $3, $5);
																}

%%

cfg_t *parse_obj(char *file_in) {
	v_buffer = new_stack(sizeof(int *));
	cfg = new_cfg();
	//init_string(&buf);

	yyin = fopen(file_in, "r");
	if (yyin == NULL) {
		printf("Could not open file: %s\n", file_in);
		return NULL;
	}
	yyparse();

	return cfg;
}

void dbgy(char *msg) {
	if (show_dbgy)
		printf("[%d Y] %s", yylineno, msg);
}

void add_v_cfg(int v, int vt, int vn) {
	int *vert = (int *)malloc(3*sizeof(int));
	vert[0] = v, vert[1] = vt, vert[2] = vn;
	v_buffer->push(v_buffer, &vert);
	printf("(v_buffer: %d)", v_buffer->index);
	int *out = ((int **)v_buffer->storage)[v_buffer->index];
	printf("(%d_%d_%d) ", out[0], out[1], out[2]);
}
