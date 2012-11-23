%{
#include <stdio.h>
#include "parser.hh"


extern void yyparse(void);
extern int yylex_destroy(void);


YY_BUFFER_STATE yybs = 0;

void 
parse_equation(char *buf,int l) {
 //   printf("\nparse\n");
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


"Acceleration:"		/*printf("ACC \n");*/return ACC;
"Gyro:"                 /*printf("GYRO \n");*/return GYRO;
"temp:"                 /*printf("TEMP \n");*/return TEMP;
"x:"                    /*printf("X \n");*/return X;
"y:"                    /*printf("Y \n");*/return Y;
"z:"                    /*printf("Z \n");*/return Z;
"rad/s"                 /*printf("RADSEC \n");*/return RADSEC;
"rad"                   /*printf("RAD\n");*/return RAD;
"yaw:"                  /*printf("YAW\n");*/return YAW;
"Left motor:"           /*printf("LMOTOR \n");*/return LMOTOR;
"Right motor:"          /*printf("RMOTOR \n");*/return RMOTOR;
"Battery: normal"       /*printf("BATTERYNORM \n");*/return BATTERYNORM;
"Battery: disconnected" /*printf("BATTERYDISC \n");*/return BATTERYDISC;
"Capacitor:"            /*printf("CAPACITOR \n");*/return CAPACITOR;
"Coilgun:"              /*printf("COILGUN\n");*/return COILGUN;
"Charger:"              /*printf("CHARDER\n");*/return CHARGER;
"Melexis R:"            /*printf("MELEXIS R \n");*/return MELEXISR;
"Melexis L:"            /*printf("MELEXIS L \n");*/return MELEXISL;
"Panda"                 /*printf("PANDA \n");*/return PANDA;
"Odometry:"             /*printf("ODOMETRY \n");*/return ODOMETRY;
"5V LDO:"               /*printf("5V LDO\n");*/return FIVEVLDO;
"charging"              /*printf("CHARGING\n");*/return CHARGING;
"waiting"               /*printf("WAITING\n");*/return WAITING;
"Ball"                  /*printf("BALL\n");*/return BALL;
"Normal"                /*printf("NORMAL\n");*/return NORMAL;
"Competition"           /*printf("Competition\n");*/return COMPETITION;
"Debug"                 /*printf("Debug\n");*/return DEBUG;
"acc sensor"            /*printf("ACCSENSOR\n");*/return ACCSENSOR;
"pos sensors"           /*printf("POSSENSORS\n");*/return POSSENSORS;
\x1b\[[0-9]+;[0-9]+H	/*printf("SETCURSOR \n");*/return SETCURSOR;
\x1b\[0K	        /*printf("CLEARSCREEN\n");*/return CLEARSCREEN;
\x1b\[[0-9]m	        /*printf("SETLINE\n");*/return SETLINE;
[-]*[[0-9]+\.[0-9]+	/*printf("FLOAT '%s'\n",yytext);*/yylval.fval=atof(yytext);return FLOAT;
[0-9]+                  /*printf("NUMBER '%s'\n",yytext);*/yylval.ival=atoi(yytext); return NUMBER;
on|off                  /*printf("ONOFF\n");*/yylval.ival=!strcmp(yytext,"on"); return ONOFF;
\n                      /*printf("NEWLINE\n");*//* ignore end of line */;
[ \t]+                  /*printf("WHITE\n");*//* ignore whitespace */;
[a-zA-Z]	        /*printf("WTF '%c'\n",yytext[0]);*/return yytext[0];
.	                /*printf("WTFcharnonprint '%c'\n",yytext[0]);*/return yytext[0];
 
%%

