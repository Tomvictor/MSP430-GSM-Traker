//*****************************************************************************
//  MSP430G2xx3 Demo - Basic Clock, LPM3 Using WDT ISR, VLO ACLK
//
//  Description: This program operates MSP430 normally in LPM3, pulsing P1.0
//  ~ 6 second intervals. WDT ISR used to wake-up system. All I/O configured
//  as low outputs to eliminate floating inputs. Current consumption does
//  increase when LED is powered on P1.0. Demo for measuring LPM3 current.
//  ACLK = VLO/2, MCLK = SMCLK = default DCO
//
/*	Start
 * Disable watch Dog Timer
 * Initialize Clock
 * Initialize USCI Module
 * Initialize GPIO
 * Initiallize Timer
 * Initial AT commands are send to M66 Via UART
 * Wait for SIM Ready for Airtel, Then give sleep at command, Make sure M66 is running vial VDD_EXT S/L
 * Pull DTR pin to High
 * Now module will be in sleep
 * Set CCR0 to 1Hr Delay
 *
 * In ISR of CCR0 (1Hr Delay)
 * 	pull low the DTR pin (Wake from sleep mode),
 *  give soft delay 5 Secs then Turn off M66 by
 *	PWR_Key S/L (Now M66 is Turned off),
 *  Set CCR1 for 2Hr Delay, Exit ISR
 *
 * In ISR of CCR1 (2Hr Delay)
 * 	Turn On M66 by giving PWR KY S/L
 * 	M66 Initialize fn call
 * 	sleep Command
 * 	pull DTR High (Now in Sleep Mod)
 * 	Set CCR0 for 1Hr Wakeup delay
 *
 */

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
  CCTL0 = OUTMOD_4 + CCIE;                  // CCR0 toggle, interrupt enabled
  CCTL1 = OUTMOD_4 + CCIE;                  // CCR1 toggle, interrupt enabled
  CCTL2 = OUTMOD_4 + CCIE;                  // CCR1 toggle, interrupt enabled
  CCR0 = 12500;
  TACTL = TASSEL_1 + ID_3 + MC_2;                  // ACLK, contmode

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
} //Main end

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)

{
  CCR0 += 200;                              // Add Offset to CCR0
}

// Timer_A2 Interrupt Vector (TA0IV) handler
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)

{
  switch( TA0IV )
  {
  case  2: CCR1 += 1000;                    // Add Offset to CCR1
           break;
  case  4: CCR2 += 1000;                    // Add Offset to CCR1
             break;
  case 10: P1OUT ^= 0x01;                   // Timer_A3 overflow
           break;
 }
}
