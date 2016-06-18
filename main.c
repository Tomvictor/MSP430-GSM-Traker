
//******************************************************************************
//  MSP430G2xx3 Demo - Basic Clock, LPM3 Using WDT ISR, VLO ACLK
//
//  Description: This program operates MSP430 normally in LPM3, pulsing P1.0
//  ~ 6 second intervals. WDT ISR used to wake-up system. All I/O configured
//  as low outputs to eliminate floating inputs. Current consumption does
//  increase when LED is powered on P1.0. Demo for measuring LPM3 current.
//  ACLK = VLO/2, MCLK = SMCLK = default DCO
//
//
//           MSP430G2xx3
//         ---------------
//     /|\|            XIN|-
//      | |               |
//      --|RST        XOUT|-
//        |               |
//        |           P1.0|-->LED
//
//******************************************************************************


#include <msp430.h>
unsigned int count=0;
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  BCSCTL1 |= DIVA_3;                        // ACLK/8
  BCSCTL3 |= LFXT1S_2;                      // ACLK = VLO
  //WDTCTL = WDT_ADLY_1000;                   // Interval timer
  //IE1 |= WDTIE;                             // Enable WDT interrupt
  P1DIR = 0xFF;                             // All P1.x outputs
  P1OUT = 0;                                // All P1.x reset
  P2DIR = 0xFF;                             // All P2.x outputs
  P2OUT = 0;                                // All P2.x reset
  // from timer
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = 12500;
  TACTL = TASSEL_1 + ID_3 + MC_1;                  // ACLK, contmode

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

  /*
   * Inbuild code for demo vlo
  while(1)
  {
    int i;
    P1OUT |= 0x01;                          // Set P1.0 LED on
    for (i = 10000; i>0; i--);              // Delay
    P1OUT &= ~0x01;                         // Reset P1.0 LED off
    __bis_SR_register(LPM3_bits + GIE);     // Enter LPM3
  }
  */
} //Main end
// Timer A0 interrupt service routine

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	//if (count == 6) {
		P1OUT ^= BIT0 ;
	//	count = 0 ;
	//}
	//count++ ;
	CCR0 = 12500;                            // Add Offset to CCR0
}


/*
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void)
#else
#error Compiler not supported!
#endif
{
  __bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)
}
*/
