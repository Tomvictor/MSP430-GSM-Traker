//******************************************************************************
//  MSP430G2553 Code for GSM Tracking Device
//
//  Description: M66 - GSM/GPRS Module by Quectel
//  > Turn the device in to sleep mode for 30 mints (Timer A)
//  > Turn the device to power off for 2 Hr (Timer B)
//  >
//
//  By
//	  Tomvictor
//    Technorip, Cochin
//	  +91 999 50 600 98
//    June 2016
//    Built with CCS Version 6
//******************************************************************************

#include <msp430g2553.h>

//void UART_Tx(void);
void initialise(void);
void configure_GPIO(void);
void configure_CLK(void);

char string1[90], TxByte, RxByte;
const char sleep[] = { "at+qsclk=1\n" },
		sleep_Check[]= { "at+qsclk?\n" },
		at[] = { "at\n"},ATV1[] = { "ATV1\n"},
		ATE1[] = { "ATE1\n"},
		cmee[] = { "AT+CMEE=2\n"},
		ipr[] = { "AT+IPR?\n"},
		ipr1[] = { "AT+IPR=115200\n"},
		w[] = { "AT&W\n"}    ;
unsigned int i, s=0, sl_flag = 0, x = 0, count1 = 0, count2, TX_Flag=0;
char j = 0, y = 0;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  //configure_CLKs
  configure_CLK();
  //ConfigureGPIOs
  configure_GPIO();
  //UART Codes
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 8;                              // 1MHz 115200
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  // ok program starts here
  __bis_SR_register(GIE);       //  interrupts enabled
  // 2.5 Sec delay to PWR Key Pin
  P2OUT = BIT3 ;
  __delay_cycles(2500000);
  P2OUT &= ~BIT3 ;
  initialise();
 //Sleep code
 for (x=0; x < sizeof sleep; x++){
	  UCA0TXBUF = sleep[x] ;
	  TX_Flag = 0;
	 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
	  __delay_cycles(100000);
 }//for end

 	 P2OUT |= BIT2 ; //Pull DTR pin to High

while(1){

	  for (x=0; x < sizeof sleep_Check; x++){
		  UCA0TXBUF = sleep_Check[x] ;
		  TX_Flag = 0;
		 // while(~TX_Flag) ;//wait for Tx_Flag to set indicating the transmision is complete, better option than delay
		  __delay_cycles(100000);

	  }//for end

	  __delay_cycles(5000000); //5 sec delay

		}//while end
}//main end

// USCI A0/B0 Transmit ISR
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)

{
	//P1OUT ^= BIT6 ;
	TX_Flag = 1;
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

void configure_CLK(void){
	  if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	  {
	    while(1);                               // do not load, trap CPU!!
	  }
	  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	  DCOCTL = CALDCO_1MHZ;
}


void configure_GPIO(void){
	  P1DIR = 0xFF;                             // All P1.x outputs
	  P1OUT = 0;                                // All P1.x reset
	  P1SEL = BIT1 + BIT2 + BIT4;               // P1.1 = RXD, P1.2=TXD
	  P1SEL2 = BIT1 + BIT2;                     // P1.4 = SMCLK, others GPIO
	  P2DIR = 0xFF;                             // All P2.x outputs
	  P1DIR &= ~BIT4;							// Input for VDD Ext
	  P2OUT = 0;                                // All P2.x reset
	  P3DIR = 0xFF;                             // All P3.x outputs
	  P3OUT = 0;                                // All P3.x reset
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

