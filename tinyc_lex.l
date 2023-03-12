
D			[0-9]
L			[a-zA-Z_]

%{
#define YYSTYPE char*
#include "y.tab.h"

void comment();

void yyerror();
%}


%%
"#include"    { return INCLUDE; }
#[^\n]*       { }
"//"[^\n]*    { }
"/*"          { comment(); }
[ \t\n]+      { }

"{"           { return OPEN_BRACE; }
"}"           { return CLOSE_BRACE; }
"("           { return '('; }
")"           { return ')'; }
"~"           { return '~'; }
"!"           { return '!'; }
"+"           { return '+'; }
"-"           { return '-'; }
"*"           { return '*'; }
"<"           { return '<'; }
"<="          { return LESS_OR_EQUAL; }
"="           { return '='; }
";"           { return ';'; }
","           { return ','; }
[0-9]+        {
                //检查数字在合法范围内，不能以0开头
                yylval = strdup(yytext); return NUMBER;
              }
"if"          { return IF; }
"while"       { return WHILE; }
"return"      { return RETURN; }

"int"         { return TYPE; }
{L}({L}|{D})* { yylval = strdup(yytext); return IDENTIFIER; }

"<"[a-z.]+">" { return HEADER_NAME; }
%%

#define INPUT_EOF 0

//多行注释处理
void comment(void) {

    char c, prev = 0;
    //跳过，直到*/出现
    while ((c = input()) != INPUT_EOF) {
        if (c == '/' && prev == '*')
            return;
        prev = c;
    }
    yyerror("unterminated comment");
}
