%{
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "globals.h"
#define YYDEBUG 1
void 
yyerror(const char *str) {
//    printf("yyerror: '%s'\n",str);
}

int 
yywrap() {
    return 1;
}

extern int yylex();
extern int yyparse();
extern FILE *yyin;

%}


// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":
%union {
	int ival;
	float fval;
	char *sval;
}

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:
%token <ival> NUMBER
%token <fval> FLOAT 
%token <sval> SVAL

%token X Y Z HEAT ONOFF TARGET TEMPERATURE SETCURSOR CLEARSCREEN 
%token TEMP ACC GYRO LMOTOR RMOTOR BATTERYNORM BATTERYDISC CAPACITOR CHARGER
%token RADSEC MELEXISR MELEXISL PANDA ODOMETRY FIVEVLDO SETLINE
%token COILGUN CHARGING WAITING BALL YAW RAD
%token NORMAL COMPETITION DEBUG ACCSENSOR POSSENSORS


%%

commands: /* empty */
         | commands SETCURSOR command|CLEARSCREEN;

command: /* empty line */ |
         charger|
         target_set|
	 gyro|
	 odometry|
         panda|
	 rightdrive|
	 leftdrive|
         positionsensors|
         battery |
	 floatline|
         coilgun|
         menuline/* |
	 error { 
             printf("hmm, parse error ''\n");
yyerror;
	 }*/
;


opt_selector: /* empty */ 
	| '<' 
	| '>';

menuline: opt_selector NORMAL opt_selector COMPETITION opt_selector DEBUG ACCSENSOR opt_selector DEBUG POSSENSORS opt_selector DEBUG GYRO opt_selector {
         printf("\t!!!Menuline\n");
         pthread_mutex_lock( &count_mutex2 );
         pthread_mutex_unlock( &count_mutex2 );

}
//parser:.[5;1HCoilgun: 399V, waiting , Ball! ----

coilgun: COILGUN NUMBER 'V' ',' coilstatus ',' BALL '!' {
         printf("\t!!!Coilgun\n");
}

coilstatus: CHARGING | WAITING;

floatline: FLOAT FLOAT FLOAT FLOAT FLOAT FLOAT {
         printf("\t!!!Floatline\n");
}

battery: batterystatus FLOAT 'V' NUMBER '%' PANDA ONOFF {
         printf("\t!!!Battery\n");
};
batterystatus: BATTERYDISC  | BATTERYNORM 
;

positionsensors: "Position"  "sensors" "LDO" ONOFF {
         printf("\t!!!Position sensors\n");
};

rightdrive: "Right" "drive" ':' "brake" FLOAT {
         printf("\t!!!right drive brake\n");
};

leftdrive: "Left" "drive" ':' "brake" FLOAT {
         printf("\t!!!left drive brake\n");
};

target_set: TARGET TEMPERATURE NUMBER {
         printf("\t!!!Temperature set\n");
};

gyro: GYRO TEMP NUMBER X FLOAT RADSEC Y FLOAT RADSEC Z FLOAT RADSEC {
         printf("\t!!!Gyro %d %f\n",$3,$5);
};

odometry: ODOMETRY X FLOAT 'm' Y FLOAT 'm' YAW FLOAT  RAD {
         printf("\t!!!Odometry %f %f %f \n",$3,$6,$9);
};

panda: PANDA ONOFF {
         printf("\t!!!Panda\n");
};

charger: CHARGER ONOFF {
         printf("\t!!!Charger\n");
};
%%
