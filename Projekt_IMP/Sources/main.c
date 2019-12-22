/**
* Author: Jozef Vanicky
* VUT Login: xvanic09
* Date: 2019-12-21
* Author's comment: N/A
**/
/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK 0x1Fu
#define GPIO_PIN(x) (((1)<<(x & GPIO_PIN_MASK)))

#define OVERFLOW 0x5DC

#define R1  0x400
#define R2  0x800
#define R3  0x1000000
#define R4  0x2000000
#define R5  0x4000000
#define R6  0x8000000
#define R7  0x10000000
#define R8  0x20000000

#define C1357Green  0x200
#define C1357Red    0x40

#define C2468Green  0x80
#define C2468Red    0x100

/* A delay function */
//DO NOT TOUCH
void delay(long long bound) {
  long long i;
  for(i=0;i<bound;i++);
}

/* Initialize the MCU - basic clock settings, turning the watchdog off */
//DO NOT TOUCH
void MCUInit(void)  {
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

void TurnClocksON(void)
{
    /* Turn on all port clocks */
    SIM->SCGC5 = SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;
}

void PortsInit_GPIO(void)
{
    /* Set corresponding pins (connected to LED's) for GPIO functionality */
    
    //Rows
    PORTA->PCR[10] = PORT_PCR_MUX(0x01);    //R1	//PIN24
    PORTA->PCR[11] = PORT_PCR_MUX(0x01);    //R2	//PIN26
    PORTA->PCR[24] = PORT_PCR_MUX(0x01);    //R3	//PIN39
    PORTA->PCR[25] = PORT_PCR_MUX(0x01);    //R4	//PIN40

    PORTA->PCR[26] = PORT_PCR_MUX(0x01);    //R5	//PIN37
    PORTA->PCR[27] = PORT_PCR_MUX(0x01);    //R6	//PIN35
    PORTA->PCR[28] = PORT_PCR_MUX(0x01);    //R7	//PIN38
    PORTA->PCR[29] = PORT_PCR_MUX(0x01);    //R8	//PIN36

    //Columns
    PORTA->PCR[9] = PORT_PCR_MUX(0x01);     //C1,3,5,7 - Green  //PIN28	//WireGreen
    PORTA->PCR[6] = PORT_PCR_MUX(0x01);     //C1,3,5,7 - Red	//PIN25	//WireRed

    PORTA->PCR[8] = PORT_PCR_MUX(0x01);     //C2,4,6,8 - Green  //PIN23	//WireBlue
    PORTA->PCR[7] = PORT_PCR_MUX(0x01);     //C2,4,6,8 - Red	//PIN27	//WireOrange


    /* Set GPIO on LED PINs as output because of Rows */
    PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);     // Have to set ROW PINs as output in GPIO here, not the colums which are here PWM
    PTA->PDOR = GPIO_PDOR_PDO(0x3F000C00);     // Disabled GND on Rx

}

void PortsInit_PWM(void) //Will be used later
{
    /* Set corresponding pins (connected to LED's) for GPIO functionality */

    //Rows
    PORTA->PCR[10] = PORT_PCR_MUX(0x01);    //R1	//PIN24
    PORTA->PCR[11] = PORT_PCR_MUX(0x01);    //R2	//PIN26
    PORTA->PCR[24] = PORT_PCR_MUX(0x01);    //R3	//PIN39
    PORTA->PCR[25] = PORT_PCR_MUX(0x01);    //R4	//PIN40

    PORTA->PCR[26] = PORT_PCR_MUX(0x01);    //R5	//PIN37
    PORTA->PCR[27] = PORT_PCR_MUX(0x01);    //R6	//PIN35
    PORTA->PCR[28] = PORT_PCR_MUX(0x01);    //R7	//PIN38
    PORTA->PCR[29] = PORT_PCR_MUX(0x01);    //R8	//PIN36

    //Columns
    PORTA->PCR[9] = PORT_PCR_MUX(0x03);     //C1,3,5,7 - Green  //PIN28	//WireGreen
    PORTA->PCR[6] = PORT_PCR_MUX(0x03);     //C1,3,5,7 - Red	//PIN25	//WireRed

    PORTA->PCR[8] = PORT_PCR_MUX(0x03);     //C2,4,6,8 - Green  //PIN23	//WireBlue
    PORTA->PCR[7] = PORT_PCR_MUX(0x03);     //C2,4,6,8 - Red	//PIN27	//WireOrange


    /* Set GPIO on LED PINs as output because of Rows */
    PTA->PDDR = GPIO_PDDR_PDD(0x3F000C00);     // Have to set ROW PINs as output in GPIO here, not the colums which are here PWM
    PTA->PDOR &= GPIO_PDOR_PDO(0x0);     // GND na katodu

}

void FTM0_Init(void) {
	SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;
	FTM0->CNT = 0x0;            // Vynulujeme registr citace (Counter) casovace
	FTM0->MOD = OVERFLOW;       // Nastavime hodnotu preteceni do Modulo registru

    //Nastavime rezim generovani PWM na zvolenem kanalu (n) casovace v ridicim
    //registru FTM_CnSC tohoto kanalu, konkretne:
    //Edge-aligned PWM: High-true pulses (clear Output on match, set Output on reload),
    //preruseni ani DMA requests nebudou vyuzivany.
	FTM0_C3SC = 0x28;
    FTM0_C4SC = 0x28;

	FTM0_C3V = OVERFLOW/2;
    FTM0_C4V = OVERFLOW/2;

    //Nastavime konfiguraci casovace v jeho stavovem a ridicim registru (SC):
    //(up counting mode pro Edge-aligned PWM, Clock Mode Selection (01),
    //Prescale Factor Selection (Divide by 8), bez vyuziti preruseni ci DMA.
    //Budeme-li masky v SC registru nastavovat postupne, je NUTNO
    //toto provadet pri Clock Mode Selection = 00 (tj. v rezimu TPM disabled).
	FTM0->SC = 0xB;

}

void FTM1_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;
 	FTM1->CNT = 0x0;            // Vynulujeme registr citace (Counter) casovace
	FTM1->MOD = OVERFLOW;       // Nastavime hodnotu preteceni do Modulo registru

    //Nastavime rezim generovani PWM na zvolenem kanalu (n) casovace v ridicim
    //registru FTM_CnSC tohoto kanalu, konkretne:
    //Edge-aligned PWM: High-true pulses (clear Output on match, set Output on reload),
    //preruseni ani DMA requests nebudou vyuzivany.
    FTM1_C0SC = 0x28;
    FTM1_C1SC = 0x28;

    FTM1_C0V = OVERFLOW/2;
    FTM1_C1V = OVERFLOW/2;

    //Nastavime konfiguraci casovace v jeho stavovem a ridicim registru (SC):
    //(up counting mode pro Edge-aligned PWM, Clock Mode Selection (01),
    //Prescale Factor Selection (Divide by 8), bez vyuziti preruseni ci DMA.
    //Budeme-li masky v SC registru nastavovat postupne, je NUTNO
    //toto provadet pri Clock Mode Selection = 00 (tj. v rezimu TPM disabled).
	FTM1->SC = 0xB;
}

void Effect_GPIO(){
    PortsInit_GPIO();
    
    for(int j = 0; j< 3; j++){
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                PTA->PDOR &= ~GPIO_PDOR_PDO(C1357Green|C2468Green);
                PTA->PDOR |= GPIO_PDOR_PDO(C2468Red|C1357Red);
            } else if (i == 1){
                PTA->PDOR &= ~GPIO_PDOR_PDO(C2468Red|C1357Red);
                PTA->PDOR |= GPIO_PDOR_PDO(C1357Green|C2468Green);
            }
            else{
                PTA->PDOR |= GPIO_PDOR_PDO(C1357Green|C2468Green|C2468Red|C1357Red);
            }
            PTA->PDOR |= GPIO_PDOR_PDO(R1|R2|R3|R4|R5|R6|R7|R8); // Off all Rx
            PTA->PDOR &= ~GPIO_PDOR_PDO(R1); // On R1
            delay(8000*20);
            PTA->PDOR |= GPIO_PDOR_PDO(R1); // Off R1
            PTA->PDOR &= ~GPIO_PDOR_PDO(R2); // On R2
            delay(8000*30);
            PTA->PDOR |= GPIO_PDOR_PDO(R2); // Off R2
            PTA->PDOR &= ~GPIO_PDOR_PDO(R3); // On R3
            delay(8000*40);
            PTA->PDOR |= GPIO_PDOR_PDO(R3); // Off R3
            PTA->PDOR &= ~GPIO_PDOR_PDO(R4); // On R4
            delay(8000*50);
            PTA->PDOR |= GPIO_PDOR_PDO(R4); // Off R4
            PTA->PDOR &= ~GPIO_PDOR_PDO(R5); // On R5
            delay(8000*60);
            PTA->PDOR |= GPIO_PDOR_PDO(R5); // Off R5
            PTA->PDOR &= ~GPIO_PDOR_PDO(R6); // On R6
            delay(8000*70);
            PTA->PDOR |= GPIO_PDOR_PDO(R5); // Off R5
            PTA->PDOR &= ~GPIO_PDOR_PDO(R6); // On R6
            delay(8000*80);
            PTA->PDOR |= GPIO_PDOR_PDO(R6); // Off R6
            PTA->PDOR &= ~GPIO_PDOR_PDO(R7); // On R7
            delay(8000*90);
            PTA->PDOR |= GPIO_PDOR_PDO(R7); // Off R7
            PTA->PDOR &= ~GPIO_PDOR_PDO(R8); // On R8
            delay(8000*100);
        }
    }
    
}


void Effect_PWM(){
    PortsInit_PWM();
    FTM0_Init();
    FTM1_Init();



}

int main(void)
{
    MCUInit();
    TurnClocksON();
    

    while (1) {
        Effect_GPIO();
        delay(8000*300);
        Effect_PWM();
    }


    return 0;
}
