%{
#include <stdio.h>
#include "parser.hh"


extern void yyparse(void);
extern int yylex_destroy(void);


YY_BUFFER_STATE yybs = 0;

void 
parse_equation(char *buf,int l) {
    printf("\nparse\n");
    yybs = yy_scan_bytes(buf, l);
    yy_switch_to_buffer(yybs);
    yyparse();
    yy_delete_buffer(yybs);
    /* to avoid leakage */
    yylex_destroy();
}



%}
%option nounput
%%


"Acceleration:"		printf("ACC \n");return ACC;
"Gyro:"                 printf("GYRO \n");return GYRO;
"temp:"                 printf("TEMP \n");return TEMP;
"x:"                    printf("X \n");return X;
"y:"                    printf("Y \n");return Y;
"z:"                    printf("Z \n");return Z;
"rad/s"                 printf("RADSEC \n");return RADSEC;
"Left motor:"           printf("LMOTOR \n");return LMOTOR;
"Right motor:"          printf("RMOTOR \n");return RMOTOR;
"Battery:"              printf("BATTERY \n");return BATTERY;
"Capacitor:"            printf("CAPACITOR \n");return CAPACITOR;
"Charger:"              printf("CHARDER\n");return CHARGER;
"Ball:"                 printf("BALL \n");return BALL;
"Melexis R:"            printf("MELEXIS R \n");return MELEXISR;
"Melexis L:"            printf("MELEXIS L \n");return MELEXISL;
"Panda:"                printf("PANDA \n");return PANDA;
"Odometry:"             printf("ODOMETRY \n");return ODOMETRY;
"5V LDO:"               printf("5V LDO\n");return FIVEVLDO;
\x1b\[[0-9]+;[0-9]+H	printf("SETCURSOR \n");return SETCURSOR;
\x1b\[0K	        printf("CLEARSCREEN\n");return CLEARSCREEN;
\x1b\[[0-9]m	        printf("SETLINE\n");return SETLINE;
[-]*[[0-9]+\.[0-9]+	printf("FLOAT '%s'\n",yytext);yylval=atof(yytext);return FLOAT;
[0-9]+                  printf("NUMBER '%s'\n",yytext);yylval=atoi(yytext); return NUMBER;
on|off                  printf("ONOFF\n");yylval=!strcmp(yytext,"on"); return ONOFF;
\n                      printf("NEWLINE\n");/* ignore end of line */;
[ \t]+                  /*printf("WHITE\n");*//* ignore whitespace */;
[a-zA-Z]+	        printf("WTF '%s'\n",yytext);/* ignore the rest printable */;
.	                printf("WTFchar '%c'\n",yytext[0]);/* ignore the rest nonprintable */;
 
%%

