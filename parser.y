%{
#include <stdio.h>
#include <string.h>
#define YYDEBUG 1
void 
yyerror(const char *str) {
    fprintf(stderr,"yyerror: '%s'\n",str);
}

int 
yywrap() {
    return 1;
}

extern int yylex();
extern int yyparse();
extern FILE *yyin;

%}

%token NUMBER HEAT ONOFF TARGET TEMPERATURE SETCURSOR CLEARSCREEN 
%token TEMP ACC GYRO LMOTOR RMOTOR BATTERY CAPACITOR BALL CHARGER
%token X Y Z RADSEC FLOAT MELEXISR MELEXISL PANDA ODOMETRY FIVEVLDO SETLINE

%%

commands: /* empty */
         | commands command|CLEARSCREEN;

command: charger_switch |
         target_set|
	 gyro|
	 odometry|
	 charger|
         panda|
	 rightdrive|
	 leftdrive|
         positionsensors|
         battery |
	 floatline|
         coilgun|
         menuline |
	 error { 
             printf("hmm, parse error ''\n");
yyerror;
	 }
;

menuline: SETCURSOR "Normal" SETLINE SETLINE '>' "Competition" '<' SETLINE "Debug" "acc" "sensor" "Debug" "pos" "sensors" "Debug" "gyro" {
         printf("\t!!!Menuline\n");
}

coilgun: SETCURSOR "Coilgun" ':' "0V" ',' "charging" ',' "Ball" '!' {
         printf("\t!!!Coilgun\n");
}

floatline: SETCURSOR FLOAT FLOAT FLOAT FLOAT FLOAT FLOAT {
         printf("\t!!!Floatline\n");
}

battery: SETCURSOR BATTERY "disconnected" FLOAT 'V' NUMBER '%' "Panda" ONOFF {
         printf("\t!!!Battery\n");
};

positionsensors: SETCURSOR "Position"  "sensors" "LDO" ONOFF {
         printf("\t!!!Position sensors\n");
};

rightdrive: SETCURSOR "Right" "drive" ':' "brake" FLOAT {
         printf("\t!!!right drive brake\n");
};

leftdrive: SETCURSOR "Left" "drive" ':' "brake" FLOAT {
         printf("\t!!!left drive brake\n");
};

charger_switch: CHARGER ONOFF {
         printf("\t!!!Charger on or off\n");
};

target_set: TARGET TEMPERATURE NUMBER {
         printf("\t!!!Temperature set\n");
};

gyro: SETCURSOR GYRO TEMP NUMBER X FLOAT RADSEC Y FLOAT RADSEC Z FLOAT RADSEC {
         printf("\t!!!Gyro %d %f\n",$4,(float)$6);
};

odometry: SETCURSOR ODOMETRY X FLOAT 'm' Y FLOAT 'm' "yaw" ':' FLOAT  "rad" {
         printf("\t!!!Odometry\n");
};

panda: SETCURSOR PANDA ONOFF {
         printf("\t!!!Panda\n");
};

charger: SETCURSOR CHARGER ONOFF {
         printf("\t!!!Charger\n");
};
%%
