#include <pic18f4520.h>
#include "config.h"
#include "lcd.h"
#include "keypad.h"
#include "ssd.h"
#include "timer.h"
#include "atraso.h"

#define L_ON  0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1  0x80
#define L_L2  0xC0
#define LED (*(volatile __near unsigned char*) 0xF83)

static char senha[4];
int quant = 0;
char confirma = 0;
int valor = 0;

void cartao() {
    char i = 0;
    char apertado = 1;
    unsigned int key = 0;
    
    

    lcdCommand(L_CLR);
    lcdString("Valor:");
    atraso_ms(20);
    lcdNumber(quant * 5);
    lcdCommand(L_L2);
    atraso_ms(20);
    lcdString("Senha:");
    while (i < 4) { //aguarda os 4 digitos
        kpDebounce();
        if (apertado == 0) { 
            if ((kpReadKey() != 0) && (kpRead() != key)) {
                key = kpRead();
                senha[i] = kpReadKey(); 
                lcdChar('*');
                i++;
                atraso_ms(15);
                apertado = 1;
                ssdDigit(16,0); // tentativa de corrigir bug no pino de dados
            }//end if
        }//end if
        else {
                apertado = 0;
            }
    }//end while
    atraso_ms(1000);
    lcdCommand(L_CLR);
    lcdString("Confirmado.");
    atraso_ms(4000);
    lcdCommand(L_CLR);
}

int destino() {
    unsigned int tecla;
    lcdCommand(L_L1);
    char apertado = 0;
    lcdString("Destino: ");
    kpDebounce();
    if (apertado == 0) {
        if ((kpRead != tecla)&&(kpReadKey != 0)) {
            tecla = kpRead();
            switch (kpReadKey()) {
                case 'U': //destino 1
                    lcdCommand(L_L2);
                    lcdString("Zona Norte ");
                    LED = 0x01;
                    break;
                case 'L': // destino 2
                    lcdCommand(L_L2);
                    lcdString("Zona Oeste");
                    LED = 0x02;
                    break;
                case 'D': //destino 3
                    lcdCommand(L_L2);
                    lcdString("Zona Sul       ");
                    LED = 0x04;
                    break;
                case 'R': //destino 4
                    lcdCommand(L_L2);
                    lcdString("Zona Leste");
                    LED = 0x08;
                    break;
                case 'X': //aumenta numero de tickets
                    quant++;
                    apertado = 1;
                    break;
                case 'B': //diminui numero de tickets
                    quant--;
                    apertado = 1;
                    break;
                case 's': //confirma operação
                    LED = 0xFF;
                    confirma = 1;
                    break;
                default:
                    break;
            }//end switch
        }
    }else{apertado = 0;}
    
}// end tickets

void main(void) {
    char slot;
    lcdInit();
    kpInit();
    ssdInit();
    timerInit();
    (*(volatile __near unsigned char*) 0xF95) = 0x00; //inicia LED como saida
    lcdCommand(L_L1);
    lcdString("Tickets Metro");
    atraso_ms(3000);
    lcdCommand(L_CLR);
    for (;;) {
        timerReset(25000);
        ssdUpdate();
        switch (slot) {
            case 0:
                if (confirma == 0){
                    destino(); 
                }
                slot = 1;
                break;
            case 1:
                kpDebounce();
                slot = 2;
                break;
            case 2:
                ssdDigit(((quant / 1000) % 10), 0);
                ssdDigit(((quant / 100) % 10), 1);
                ssdDigit(((quant / 10) % 10), 2);
                ssdDigit(((quant / 1) % 10), 3);
                slot = 3;
                break;
            case 3:
                if(confirma == 1){
                    
                    cartao(); //inicia pagamento
                    confirma = 0;
                }
                slot = 0;
                break;
            default:
                slot = 0;
                break;
        }//end switch
        timerWait();
    }//end for
}//end main
