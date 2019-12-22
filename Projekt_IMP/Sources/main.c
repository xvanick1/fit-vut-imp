//       An example for demonstrating basic principles of FITkit3 usage.
//
// It includes GPIO - inputs from button press/release, outputs for LED control,
// timer in output compare mode for generating periodic events (via interrupt
// service routine) and speaker handling (via alternating log. 0/1 through
// GPIO output on a reasonable frequency). Using this as a basis for IMP projects
// as well as for testing basic FITkit3 operation is strongly recommended.
//
//            (c) 2019 Michal Bidlo, BUT FIT, bidlom@fit.vutbr.cz
////////////////////////////////////////////////////////////////////////////
/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK 0x1Fu
#define GPIO_PIN(x) (((1)<<(x & GPIO_PIN_MASK)))

#define OVERFLOW 0x5DC

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

void PortsInit_Classic(void)
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
    PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);     // Have to set ROW PINs as output in GPIO here, not the colums which are here PWM
    PTA->PDOR &= GPIO_PDOR_PDO(0xC0FFF03F);     // GND na katodu

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
    PTA->PDOR &= GPIO_PDOR_PDO(0xC0FFF3FF);     // GND na katodu

}

void FTM0_Init(void) {
	SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;
	FTM0->CNT = 0x0;
	FTM0->MOD = OVERFLOW;

	FTM0_C3SC = 0x28;
    FTM0_C4SC = 0x28;

	FTM0_C3V = OVERFLOW/2;
    FTM0_C4V = OVERFLOW/2;

	FTM0->SC = 0xB;

}

void FTM1_Init(void) {
    SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;
    FTM1->CNT = 0x0;
    FTM1->MOD = OVERFLOW;

    FTM1_C0SC = 0x28;
    FTM1_C1SC = 0x28;

    FTM1_C0V = OVERFLOW/2;
    FTM1_C1V = OVERFLOW/2;

    FTM1->SC = 0xB;
}

int main(void)
{
    MCUInit();
    TurnClocksON();
    PortsInit_PWM();

    FTM0_Init();
    FTM1_Init();

    while(1){

    }


    return 0;
}