%{


	/*int yydebug = 1;
	void yyerror (const char *msg) {
		// Do not exit program on syntax errors
		int prev_exit = set_exit_on_error(0);
		error_print(yylineno, msg);
		set_exit_on_error(prev_exit);
	}*/
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

%%

START:				STATEMENT_LIST				{  }

STATEMENT_LIST:		STATEMENT_LIST STATEMENT	{  }
				|	/* epsilon */

STATEMENT:			stm_name val_string '\n'
				|	stm_tc VT_ARGS '\n'
				|	stm_normal VN_ARGS '\n'
				|	stm_vertex V_ARGS '\n'
				|	stm_face F_ARGS '\n'

VT_ARGS:			val_float val_float
VN_ARGS:			val_float val_float val_float
V_ARGS:				val_float val_float val_float
F_ARGS:				F_ARGS val_int
				|	/* epsilon */

%%

/*int main(int argc, char *argv[]) {
	//test...
	
	return yyparse();
}*/