//*****************************************************************************
//  MSP430G2553 Code for GSM Tracking Device
//  Description: M66 - GSM/GPRS Module by Quectel
//  > Turn the device in to sleep mode for 30 mints (Timer A)
//  > Turn the device to power off for 2 Hr (Timer B)
//
//  This program operates MSP430 normally in LPM3, pulsing P1.0
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
//  By
//	  Tomvictor
//    Technorip, Cochin
//	  +91 999 50 600 98
//    June 2016
//    Built with CCS Version 6
//******************************************************************************


#include <msp430g2553.h>

#define VDD_EXT_PORT       P2OUT
#define PWR_KEY_PORT       P2OUT
#define DTR_PORT           P2OUT

#define VDD_EXT_PIN       BIT0
#define PWR_KEY_PIN       BIT1
#define DTR_PIN           BIT2

//void UART_Tx(void);
void initialise(void);
void configure_GPIO(void);
void configure_CLK(void);
void PWR_KEY_SW(void);

char string1[90], TxByte, RxByte;
const char sleep[] = { "at+qsclk=1\n" },
		sleep_Check[]= { "at+qsclk?\n" },
		at[] = { "at\n"},ATV1[] = { "ATV1\n"},
		ATE1[] = { "ATE1\n"},
		cmee[] = { "AT+CMEE=2\n"},
		ipr[] = { "AT+IPR?\n"},
		ipr1[] = { "AT+IPR=115200\n"},
		w[] = { "AT&W\n"}    ;
unsigned int i, s=0, sl_flag = 0, x = 0, j = 0 ,count1 = 0, count2 = 0, TX_Flag=0;


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  // Configure_CLKs
  configure_CLK();
  // ConfigureGPIOs
  configure_GPIO();
  // UART Codes
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 8;                              // 1MHz 115200
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  // Ok program starts here

  // Timer Configuration
  CCTL0 =  CCIE;                    			 // CCR0 toggle, interrupt enabled
  CCTL1 =  + CCIE;                 			 // CCR1 toggle, interrupt enabled
  //CCTL2 = OUTMOD_4 + CCIE;                  // CCR1 toggle, interrupt enabled
  TACTL = TASSEL_1 + ID_3 + MC_2;             // ACLK, Timer A input divider: 3 - /8 ,Continous up

  // 2.5 Sec delay to PWR Key Pin to turn on the module
   PWR_KEY_SW();

   __bis_SR_register(GIE);       //  interrupts enabled
   __delay_cycles(5000000); // Wait until all the serial outputs were printed
  initialise();
  __delay_cycles(3000000); // Wait until all the serial outputs were printed

  //Sleep code
  for (x=0; x < sizeof sleep; x++){
 	  UCA0TXBUF = sleep[x] ;
 	  TX_Flag = 0;
 	 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
 	  __delay_cycles(100000);
  }//for end

  DTR_PORT |= DTR_PIN ; //Pull DTR pin to High


  //CCR0 = 1000;

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
} //Main end

// USCI A0/B0 Transmit ISR
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)

{
	//P1OUT ^= BIT6 ;
	//TX_Flag = 1;
	IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt

}

// USCI A0/B0 Receive ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	P1OUT ^= BIT0 ;
	if(j>90){
		for(s=0;s<91;s++){
			string1[s++] = 0;
		}
		j = 0 ;
	}
	string1[j++] = UCA0RXBUF;

}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)

{

  if (count1 == 10){
	  PWR_KEY_SW(); //Turn off the module
	  count1 = 0 ;
  }
  CCR1 = TAR + 2000 ;                              // Add Offset to CCR1
  count1++ ;
}

// Timer_A2 Interrupt Vector (TA0IV) handler
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)

{
  switch( TA0IV )
  {
  case  2:
	  //CCR1 += 1000 ;                    // Add Offset to CCR1
	  if(count2 == 20){
		  PWR_KEY_SW(); //Turn on the module
		  count2 = 0;
	  }
	  P1OUT ^= BIT0 ;
	  CCR0 = TAR + 1000   ;
	  count2++ ;

           break;
  case  4:
	  //CCR2 += 1000;                    // Add Offset to CCR2
             break;
  case 10:
	  //P1OUT ^= 0x01;                   // Timer_A3 overflow
           break;
 }
}


// Function Defnition Start here


void configure_CLK(void){
	  if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	  {
	    while(1);                               // do not load, trap CPU!!
	  }
	  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	  DCOCTL = CALDCO_1MHZ;
	  // Settings specific to Timer A
	  BCSCTL1 |= DIVA_3;                        // ACLK/8
	  BCSCTL3 |= LFXT1S_2;                      // ACLK = VLO
}


void configure_GPIO(void){
	  P1DIR = 0xFF;                             // All P1.x outputs
	  P1OUT = 0;                                // All P1.x reset
	  P1SEL = BIT1 + BIT2 ;               // P1.1 = RXD, P1.2=TXD
	  P1SEL2 = BIT1 + BIT2;                     // P1.4 = SMCLK, others GPIO
	  P2DIR = 0xFF;                             // All P2.x outputs
	  P1DIR &= ~BIT4;							// Input for VDD Ext
	  P2OUT = 0;                                // All P2.x reset
	  P3DIR = 0xFF;                             // All P3.x outputs
	  P3OUT = 0;                                // All P3.x reset

}
//PWR_KEY S/L
void PWR_KEY_SW(void){
	PWR_KEY_PORT = PWR_KEY_PIN ;
   __delay_cycles(2500000);
   PWR_KEY_PORT &= ~ PWR_KEY_PIN ;
}

//Initialisation function start here
void initialise(void){
	 for (x=0; x < sizeof at; x++){
	 		  UCA0TXBUF = at[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
	 for (x=0; x < sizeof ATV1; x++){
	 		  UCA0TXBUF = ATV1[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
	 for (x=0; x < sizeof ATE1; x++){
	 		  UCA0TXBUF = ATE1[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
	 for (x=0; x < sizeof cmee; x++){
	 		  UCA0TXBUF = cmee[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
	 for (x=0; x < sizeof ipr; x++){
	 		  UCA0TXBUF = ipr[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
	 for (x=0; x < sizeof ipr1; x++){
	 		  UCA0TXBUF = ipr[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
	 for (x=0; x < sizeof w; x++){
	 		  UCA0TXBUF = w[x] ;
	 		  TX_Flag = 0;
	 		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	 		  __delay_cycles(100000);

	 	  }//for end
}
//Initialisation function end here

