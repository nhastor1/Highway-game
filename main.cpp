#include "mbed.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "string.h"

const int SIVA_ASFALT = 50744;
const int SIVA_SCORE = 63390;
const int CRNA_ISPIS = 25388;
const int poravnjajScore = 170;
const double PI = atan(1)*4;
const int SURVIVAL = 1;
const int DESTINATION = 2;
const int EASY = 1;
const int MEDIUM = 2;
const int HARD = 3;
const double KOEF_BRZINA = 100./41;

AnalogIn kretanje(p15);
double kretanjeStaro;
int pozicija = 63;
int pozcijaDesnoMax = 63 + 5*10;
int pozcijaLijevoMax = 63 - 5*10;
int brzina = 10;
int maksBrojAuta = 3;
int mod;
int tezina;
int score;
int zivoti = 3;
double progres;
int udarenoAuto;

int rekord = 0;
int destinationLvl = 1;

int pocetniY = -46;
int krajnjiY = 251;

int autaX[3] = {20};
int autaY[3] = {pocetniY};
int autaBoje[3] = {LCD_COLOR_BLUE};
int brojAuta = 1;

int izborX[3] = {20, 63, 106};
int izborBoje[3] = {LCD_COLOR_RED, LCD_COLOR_BLUE, LCD_COLOR_YELLOW};

// Koordinate novcic
int izborXNovcicSrca[3] = {33, 78, 123};

int novcicX[3] = {};
int novcicY[3] = {};
int brojNovcica = 0;

int srceX[3] = {};
int srceY[3] = {};
int brojSrca = 0;

int ukupnoStvari = brojAuta + brojNovcica + brojSrca;

int timer = 0;
int timerZaBrzinu = 0;
int sudar = 0;
int koefPomjeranjaLinija = 0;

int timerGranicaGenerisanja = 7;

TS_StateTypeDef TS_State = { 0 };

char* to_string(int d){
	char str[6];
	snprintf(str, 6, "%d", d);
	return str;
}

void iscrtajIsprekidaneLinije(){
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    
    koefPomjeranjaLinija = (koefPomjeranjaLinija + 3) % 10;
    if(koefPomjeranjaLinija > 4){
        BSP_LCD_DrawVLine(55, 0, koefPomjeranjaLinija - 4);
        BSP_LCD_DrawVLine(100, 0, koefPomjeranjaLinija - 4);
    }
    
    for(int i=0; i<25; i++){
        BSP_LCD_DrawVLine(55, i*10 + koefPomjeranjaLinija, 6);
        BSP_LCD_DrawVLine(100, i*10 + koefPomjeranjaLinija, 6);
    }
}

void iscrtajNovcic(int x, int y){
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillCircle(x, y, 8);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_YELLOW);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(x-3, y-4, (uint8_t *)"G", LEFT_MODE);
}

void iscrtajSrce(int x, int y){
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillCircle(x, y, 3);
    BSP_LCD_FillCircle(x+5, y, 3);
    BSP_LCD_DrawPixel(x+2, y+6, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+2, y+5, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+1, y+4, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+2, y+4, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+3, y+4, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+4, y+4, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x-1, y+3, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x, y+3, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+1, y+3, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+2, y+3, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+3, y+3, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+4, y+3, LCD_COLOR_RED);
    BSP_LCD_DrawPixel(x+5, y+3, LCD_COLOR_RED);
}

void iscrtajProgres(double progress){
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(poravnjajScore, 120, 60, 12);
    
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    for(int i=1; i<59 && progress>((double)(i*10))/6; i++){
        BSP_LCD_DrawVLine(poravnjajScore+i, 122, 8);
    }
}

void iscrtajSat(){
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(poravnjajScore, 160, 65, 60);
    
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font8);
    
    
    BSP_LCD_DisplayStringAt(poravnjajScore+10, 210, (uint8_t *)"0", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore, 198, (uint8_t *)"20", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+2, 186, (uint8_t *)"40", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+10, 174, (uint8_t *)"60", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+25, 164, (uint8_t *)"80", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+42, 174, (uint8_t *)"100", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+48, 186, (uint8_t *)"120", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+50, 198, (uint8_t *)"140", LEFT_MODE);
    BSP_LCD_DisplayStringAt(poravnjajScore+40, 210, (uint8_t *)"160", LEFT_MODE);
    
}

int kazaljka_x[] = {};
int kazaljka_y[] = {};

int dajX(double speed){
    return (int) ((poravnjajScore+32) - (cos(1.25*PI - 1.5*PI*(100-speed)/100))*32);
}

int dajY(double speed){
    return (int) (190 - (sin(1.25*PI - 1.5*PI*(100-speed)/100))*30);
}

void iscrtajKazaljku(double speed){
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    int x = dajX(speed);
    int y = dajY(speed);
    
    BSP_LCD_DrawLine(poravnjajScore+32, 190, x, y);
    BSP_LCD_DrawLine(poravnjajScore+33, 190, x, y);
    BSP_LCD_DrawLine(poravnjajScore+32, 189, x, y);
    BSP_LCD_DrawLine(poravnjajScore+32, 191, x, y);
    BSP_LCD_DrawLine(poravnjajScore+33, 189, x, y);
    BSP_LCD_DrawLine(poravnjajScore+33, 191, x, y);
}

void iscrtajSpeedometer(double speed){
    iscrtajSat();
    iscrtajKazaljku(speed);
}

const int BOJA_CYAN_AUTO = 3708;

void isrctajAutic(int x,int y, int boja){
    BSP_LCD_SetTextColor(boja);
    
    BSP_LCD_FillRect(x, y, 30, 37);
    
    BSP_LCD_FillCircle(x+11,y,11);
    BSP_LCD_FillCircle(x+18,y,11);
    BSP_LCD_FillRect(x+11,y-11,10,11);
    
    BSP_LCD_FillCircle(x+8,y+37,8);
    BSP_LCD_FillCircle(x+21,y+37,8);
    BSP_LCD_FillRect(x+8,y+37,17,8);
    
    //Prozori
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    
    BSP_LCD_FillRect(x+2, y+12, 26, 20);
    BSP_LCD_FillCircle(x+8, y+12, 6);
    BSP_LCD_FillCircle(x+21, y+12, 6);
    BSP_LCD_FillRect(x+8, y+6, 12, 6);
    
    BSP_LCD_FillCircle(x+6, y+32, 4);
    BSP_LCD_FillCircle(x+23,y+32, 4);
    BSP_LCD_FillRect(x+6, y+32, 16, 4);
    
    BSP_LCD_SetTextColor(boja);
    
    //KROV
    BSP_LCD_FillRect(x+4, y+18, 22, 10);
    
    for(int i=0; i<3; i++){
        BSP_LCD_DrawLine(x+2, y+12+i, x+4, y+18+i);
        BSP_LCD_DrawLine(x+27, y+12+i, x+25, y+18+i);
        
        BSP_LCD_DrawLine(x+25, y+28+i, x+27, y+32+i);
        BSP_LCD_DrawLine(x+4, y+28+i, x+2, y+32+i);
    }
    
    //Stop svjetla
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(x+6, y+44, 4, 1);
    BSP_LCD_FillRect(x+20, y+44, 4, 1);
}

void refreshSrce(){
    int obrisiSrce = poravnjajScore+35;
    if(zivoti>0){
        iscrtajSrce(poravnjajScore+5, 80);
        
        if(zivoti>1){
           iscrtajSrce(poravnjajScore+25, 80);
           
           if(zivoti>2){
               iscrtajSrce(poravnjajScore+45, 80);
               return;
           }
        }
        else
            obrisiSrce -=20;
    }
    
    BSP_LCD_SetTextColor(SIVA_SCORE);
    BSP_LCD_FillRect(obrisiSrce, 75, 30, 15);
}

void refreshSpeed(){
    double speed = (brzina - 5) * KOEF_BRZINA;
    if(speed > 100)
        speed = 100;
    iscrtajSpeedometer(speed);
}

void refreshUI(){
    BSP_LCD_SetTextColor(SIVA_ASFALT);
    BSP_LCD_FillRect(10, 0, 135, 250);
    iscrtajIsprekidaneLinije();
    
    // Ispis score
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(poravnjajScore, 30, (uint8_t *)to_string(score), LEFT_MODE);
    
    if(mod==DESTINATION){
        iscrtajProgres(progres);
    }
}

void iscrtajUI(){
    /* Clear the LCD */
    BSP_LCD_Clear(SIVA_ASFALT);
    
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(0, 0, 10, 250);
    BSP_LCD_FillRect(145, 0, 10, 250);
    iscrtajIsprekidaneLinije();
    
    BSP_LCD_SetTextColor(SIVA_SCORE);
    BSP_LCD_FillRect(155, 0, 95, 250);

    // ISPIS SCORE
    BSP_LCD_SetBackColor(SIVA_SCORE);
    BSP_LCD_SetTextColor(CRNA_ISPIS);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(poravnjajScore, 15, (uint8_t *)"Score", LEFT_MODE);
    
    // ISPIS VRIJEDNOST SCORE
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(poravnjajScore, 30, (uint8_t *)to_string(score), LEFT_MODE);
    
    
    // ISPIS ZIVOTI
    BSP_LCD_SetBackColor(SIVA_SCORE);
    BSP_LCD_SetTextColor(CRNA_ISPIS);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(poravnjajScore, 60, (uint8_t *)"Life", LEFT_MODE);
    
    // ISCRTAJ SRCA
    if(zivoti>0){
        iscrtajSrce(poravnjajScore+5, 80);
        
        if(zivoti>1){
           iscrtajSrce(poravnjajScore+25, 80);
           
           if(zivoti>2){
               iscrtajSrce(poravnjajScore+45, 80);
           }
        }
    }

    
    // ISPIS PROGRESS
    BSP_LCD_SetBackColor(SIVA_SCORE);
    BSP_LCD_SetTextColor(CRNA_ISPIS);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(poravnjajScore, 100, (uint8_t *)"Progress", LEFT_MODE);
    
    // ISCRTAJ PROGRESS
    iscrtajProgres(progres);
    
    // ISPISI SPEEDOMETER
    BSP_LCD_SetBackColor(SIVA_SCORE);
    BSP_LCD_SetTextColor(CRNA_ISPIS);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(poravnjajScore, 140, (uint8_t *)"Speed", LEFT_MODE);
    
    // ISCRTAJ SPEEDOMETER
    refreshSpeed();
}

int dajRandom(){
    int broj = rand()%12;
    if(broj < 4){
        return 0;
    }else if(broj>8){
        return 2;
    }
    return 1;
}

int dajRandomAutoNovcicIliSrce(){
    int broj = rand()%20;
    if(broj == 19){
        return 3; // SRCE
    }else if(broj>13){
        return 2; // Novcic
    }
    return 1; // AUTO
}

void dodajAuto(){
    autaX[brojAuta] = izborX[dajRandom()];
    autaY[brojAuta] = pocetniY;
    autaBoje[brojAuta] = izborBoje[dajRandom()];
        
    brojAuta++;
    ukupnoStvari++;
}

void dodajNovcic(){
    novcicX[brojNovcica] = izborXNovcicSrca[dajRandom()];
    novcicY[brojNovcica] = pocetniY;
        
    brojNovcica++;
    ukupnoStvari++;
}

void dodajSrce(){
    srceX[brojSrca] = izborXNovcicSrca[dajRandom()];
    srceY[brojSrca] = pocetniY;
        
    brojSrca++;
    ukupnoStvari++;
}

void pomjeriAuta(){
    // Pomjeri auta
    for(int i = 0; i< brojAuta; i++){
        isrctajAutic(autaX[i], autaY[i], autaBoje[i]);
        autaY[i] = autaY[i] + brzina;
    }
    
    // Pomjeri novcice
    for(int i = 0; i< brojNovcica; i++){
        iscrtajNovcic(novcicX[i], novcicY[i]);
        novcicY[i] = novcicY[i] + brzina;
    }
    
    // Pomjeri srca
    for(int i = 0; i< brojSrca; i++){
        iscrtajSrce(srceX[i], srceY[i]);
        srceY[i] = srceY[i] + brzina;
    }
    
    // Ispalo auto
    for(int i = 0; i<brojAuta; i++){
        
        if(autaY[i] >= krajnjiY){
            for(int j=i ; j<brojAuta-1; j++){
                autaX[j] = autaX[j+1];
                autaY[j] = autaY[j+1];
                autaBoje[j] = autaBoje[j+1];
            }
            
            brojAuta--;
            ukupnoStvari--;
        }
    }
    
    // Ispao novcic
    for(int i = 0; i<brojNovcica; i++){
        
        if(novcicY[i] >= krajnjiY){
            for(int j=i ; j<brojNovcica-1; j++){
                novcicX[j] = novcicX[j+1];
                novcicY[j] = novcicY[j+1];
            }
            
            brojNovcica--;
            ukupnoStvari--;
        }
    }
    
    // Ispalo srce
    for(int i = 0; i<brojSrca; i++){
        
        if(srceY[i] >= krajnjiY){
            for(int j=i ; j<brojSrca-1; j++){
                srceX[j] = srceX[j+1];
                srceY[j] = srceY[j+1];
            }
            
            brojSrca--;
            ukupnoStvari--;
        }
    }
    
    timer++;
    
    // ostavio sam maksBrojAuta ali se ponasa kao maks broj stvari a ne auta
    if(ukupnoStvari<maksBrojAuta && timer> timerGranicaGenerisanja){ // bilo timer>7
        timer = 0;

        int auticSrceNovcic = dajRandomAutoNovcicIliSrce();
        if(auticSrceNovcic == 1)
            dodajAuto();
        else if(auticSrceNovcic == 2)
            dodajNovcic();
        else
            dodajSrce();
    }
}

void iscrtajPocetniMeni(){
    BSP_LCD_Clear(SIVA_ASFALT);
    BSP_LCD_SetBackColor(SIVA_ASFALT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(70, 100, (uint8_t *)"START GAME", LEFT_MODE);
    BSP_LCD_DisplayStringAt(70, 130, (uint8_t *)"EXIT", LEFT_MODE);
}

int izborPocetni(){
    while(1){
        BSP_TS_GetState(&TS_State);
    	if(TS_State.touchDetected){
    	    uint16_t x1 = TS_State.touchX[0];
    	    uint16_t y1 = TS_State.touchY[0];
    
    	    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    	    
    	    // START
    	    // 70, 97 | 182, 97 | 70, 117 | 182, 117
    	    
    	    if(x1 >= 70 && x1 <= 182 && y1 >= 97 && y1 <= 117)
    	        return 1;
    	    
    	    // EXIT
    	    // 70, 128 | 117, 128 | 70, 144 | 117, 144
    	    if(x1 >= 70 && x1 <= 117 && y1 >= 128 && y1 <= 144)
    	        return 0;
    
    	}
    	wait_ms(10);
    }
}

void iscrtajMeniModIgre(){
    BSP_LCD_Clear(SIVA_ASFALT);
    BSP_LCD_SetBackColor(SIVA_ASFALT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(70, 100, (uint8_t *)"SURVIVAL", LEFT_MODE);
    BSP_LCD_DisplayStringAt(70, 130, (uint8_t *)"DESTINATION", LEFT_MODE);
}

int izborMod(){
    wait(0.5);
    while(1){
        BSP_TS_GetState(&TS_State);
    	if(TS_State.touchDetected){
    	    uint16_t x1 = TS_State.touchX[0];
    	    uint16_t y1 = TS_State.touchY[0];
    
    	    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    	    
    	    // SURVIVAL
    	    // 70, 97 | 160, 97 | 70, 117 | 160, 117
    	    if(x1 >= 70 && x1 <= 182 && y1 >= 97 && y1 <= 117)
    	        return SURVIVAL;
    	    
    	    // DESTINATION
    	    // 70, 128 | 192, 128 | 70, 144 | 192, 144
    	    if(x1 >= 70 && x1 <= 192 && y1 >= 128 && y1 <= 144){
    	        destinationLvl = 1;
    	        return DESTINATION;
    	    }
    
    	}
    	wait_ms(10);
    }
}

void iscrtajMeniLevelIgre(){
    BSP_LCD_Clear(SIVA_ASFALT);
    BSP_LCD_SetBackColor(SIVA_ASFALT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(70, 90, (uint8_t *)"EASY", LEFT_MODE);
    BSP_LCD_DisplayStringAt(70, 120, (uint8_t *)"MEDIUM", LEFT_MODE);
    BSP_LCD_DisplayStringAt(70, 150, (uint8_t *)"HARD", LEFT_MODE);
}

int izborLevel(){
    wait(0.5);
    while(1){
        BSP_TS_GetState(&TS_State);
    	if(TS_State.touchDetected){
    	    uint16_t x1 = TS_State.touchX[0];
    	    uint16_t y1 = TS_State.touchY[0];
    
    	    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    	    
    	    // EASY
    	    // 70, 90 | 116, 90 | 70, 103 | 116, 103
    	    if(x1 >= 70 && x1 <= 116 && y1 >= 90 && y1 <= 103){
    	        brzina = 10;
    	        return EASY;
    	    }
    	        
    	    
    	    // MEDIUM
    	    // 70, 119 | 138, 119 | 70, 132 | 138, 132
    	    if(x1 >= 70 && x1 <= 138 && y1 >= 119 && y1 <= 132){
    	        brzina = 20;
    	        return MEDIUM;   
    	    }
    	        
    	        
	        // HARD
    	    // 70, 151 | 115, 151 | 70, 161 | 115, 161
    	    if(x1 >= 70 && x1 <= 115 && y1 >= 151 && y1 <= 161){
    	        brzina = 30;
    	        return HARD;
    	    }
    	        
    
    	}
    	wait_ms(10);
    }
}

void iscrtajZavrsniMeni(){
    BSP_LCD_Clear(SIVA_ASFALT);
    BSP_LCD_SetBackColor(SIVA_ASFALT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    
    if(score > rekord){
        rekord = score;
        BSP_LCD_DisplayStringAt(70, 60, (uint8_t *)"NEW RECORD!", LEFT_MODE);
    }
    BSP_LCD_DisplayStringAt(70, 90, (uint8_t *)"Score", LEFT_MODE);
    BSP_LCD_DisplayStringAt(70, 120, (uint8_t *) to_string(score), LEFT_MODE);
    BSP_LCD_DisplayStringAt(70, 150, (uint8_t *)"MAIN MENU", LEFT_MODE);
}

void izborZavrsni(){
    while(1){
        BSP_TS_GetState(&TS_State);
    	if(TS_State.touchDetected){
    	    uint16_t x1 = TS_State.touchX[0];
    	    uint16_t y1 = TS_State.touchY[0];
    
    	    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    	    
    	    // 70, 151 | 171, 151 | 70, 161 | 171, 161
    	    if(x1 >= 70 && x1 <= 171 && y1 >= 151 && y1 <= 161)
    	        return ;
    
    	}
    	wait_ms(10);
    }
}

void iscrtajLvlDestination(){
    BSP_LCD_Clear(SIVA_ASFALT);
    BSP_LCD_SetBackColor(SIVA_ASFALT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);

    char str[9] = "Level ";

    BSP_LCD_DisplayStringAt(70, 120, (uint8_t *) strcat(str, to_string(destinationLvl++)), LEFT_MODE);
    wait(2);
}

// Ostavio sam isto da se zove udarenoAuto al u zavisnosti od sudara pozivaju se druge funkcije
// za srce, novcic ili auto, auto - 1   |  novcic - 2   |   srce - 3
void provjeriSudarZaDatoAuto(int x, int y, int i){
    if(pozicija < x && pozicija + 30 > x || pozicija > x && pozicija < x + 30 || 
        pozicija == x){
        if(180-11 < y-11 && 180-11+56>y-11 || 180-11 > y-11 && 180-11 < y-11 + 56){
            sudar = 1;
            udarenoAuto = i;
        }
    }
}

void provjeriSudarZaDatiNovcic(int x, int y, int i){
    if(pozicija < x - 8 && pozicija + 30 > x - 8 || pozicija > x - 8 && pozicija < x - 8 + 16 || 
        pozicija == x - 8){
        if(180-11 < y-8 && 180-11+56>y-8 || 180-11 > y-11 && 180-11 < y-8 + 16){
            sudar = 2;
            udarenoAuto = i;
        }
    }
}

void provjeriSudarZaDatoSrce(int x, int y, int i){
    if(pozicija < x-3 && pozicija + 30 > x-3 || pozicija > x-3 && pozicija < x -3 + 11 || 
        pozicija == x-3){
        if(180-11 < y-3 && 180-11+56>y-3 || 180-11 > y-3 && 180-11 < y-3 + 9){
            sudar = 3;
            udarenoAuto = i;
        }
    }
}

void provjeriSudar(){
    // Sudar auto
    for(int i=0; i<brojAuta; i++){
        provjeriSudarZaDatoAuto(autaX[i], autaY[i], i);
    }
    
    // Sudar novcic
    for(int i=0; i<brojNovcica; i++){
        provjeriSudarZaDatiNovcic(novcicX[i], novcicY[i], i);
    }
    
    // Sudar srce
    for(int i=0; i<brojSrca; i++){
        provjeriSudarZaDatoSrce(srceX[i], srceY[i], i);
    }
    
    if(sudar){
        
        if(sudar==1){
            //IZBACIVANJE UDARENOG AUTA
            
            for(int j=udarenoAuto ; j<brojAuta-1; j++){
                autaX[j] = autaX[j+1];
                autaY[j] = autaY[j+1];
                autaBoje[j] = autaBoje[j+1];
            }
            brojAuta--;
            zivoti--;
            refreshSrce();
        }
        else if(sudar==2){
            for(int j=udarenoAuto ; j<brojNovcica-1; j++){
                novcicX[j] = novcicX[j+1];
                novcicY[j] = novcicY[j+1];
            }
            
            brojNovcica--;
            if(tezina == EASY)
                score += 50;
            else if(tezina == MEDIUM)
                score += 100;
            else
                score += 150;
        }
        else{
            for(int j=udarenoAuto ; j<brojSrca-1; j++){
                srceX[j] = srceX[j+1];
                srceY[j] = srceY[j+1];
            }
            
            brojSrca--;
            if(zivoti<3){
                zivoti++;
                refreshSrce();
            }
                
        }
        ukupnoStvari--;
        sudar = 0;
    }
}

int main() {
    printf("HIGHWAY GAME!\nAll rights reserved \n\tEmir Mujanovic and \n\t\tNedim Hastor\n");
    
    srand(time(NULL));
    BSP_LCD_Init();

    /* Touchscreen initialization */
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }
    
    while(1){


        // RESTART
        zivoti = 3;
        pozicija = 63;
        score = 0;
        progres = 0;
        
        iscrtajPocetniMeni();
    
        int izbor = izborPocetni();
        if(izbor == 0){
            BSP_LCD_Clear(LCD_COLOR_BLACK);
            return 0;
        }
        
        iscrtajMeniModIgre();
        
        mod = izborMod();
    
        iscrtajMeniLevelIgre();
        
        tezina = izborLevel();
    
        if(mod == DESTINATION)
            iscrtajLvlDestination();
        iscrtajUI();
        
        kretanjeStaro = kretanje;
        
        wait(0.5);
    
        while (1) {
            BSP_TS_GetState(&TS_State);
            if(TS_State.touchDetected) {
                uint16_t x1 = TS_State.touchX[0];
                uint16_t y1 = TS_State.touchY[0];
                
                if(x1> poravnjajScore -10)
                    break;
                    
                if(x1 < pozicija + 15 && pozicija != pozcijaLijevoMax){
                    if(pozicija<=63)
                        pozicija = pozcijaLijevoMax;
                    else
                        pozicija = 63;
                }else if(x1 > pozicija + 15 && pozicija != pozcijaDesnoMax){
                    if(pozicija>=63)
                        pozicija = pozcijaDesnoMax;
                    else
                        pozicija = 63;
                }
                
                wait_ms(10);
            }
            
            refreshUI();
            pomjeriAuta();
            
            if(kretanje!=kretanjeStaro){
                
                if(kretanje<kretanjeStaro && pozicija!=pozcijaLijevoMax){
                    pozicija -= 10;
                }else if(kretanje>kretanjeStaro && pozicija!=pozcijaDesnoMax){
                    pozicija += 10;
                }
                
                kretanjeStaro = kretanje;
                wait_ms(10);
                
            }
            
            provjeriSudar();
            
            if(zivoti==0){
                break;
            }
            
            if(tezina == EASY){
                score = score + 1;
            }else if(tezina == MEDIUM){
                score = score + 2;
            }else if(tezina == HARD){
                score = score + 3;
            }
            
            if(mod == DESTINATION && progres < 100){
                progres = progres + 0.5;
            }else if(mod == DESTINATION && !(progres<100)){
                iscrtajLvlDestination();
                pozicija = 63;
                brojAuta = 0;
                brojNovcica = 0;
                brojSrca = 0;
                ukupnoStvari = 0;
                progres = 0;
                brzina = brzina + 5;
                iscrtajUI();
            }
            
            if(mod == SURVIVAL && timerZaBrzinu % 100 == 0){
                brzina = brzina + 5;
                refreshSpeed();
            }
            
            if(timerZaBrzinu % 200 == 0 && timerGranicaGenerisanja > 2){
                timerGranicaGenerisanja--;
            }
            
            isrctajAutic(pozicija,180, LCD_COLOR_YELLOW);
            timerZaBrzinu++;
        }
        
        iscrtajZavrsniMeni();
        izborZavrsni();
    
    }
    
}
