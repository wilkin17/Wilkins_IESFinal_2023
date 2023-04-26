#include <msp430.h>
#include <stdint.h>

void GPIO();
void LEDSolid();
void LEDSlow();
void LEDFast();
void timerInit();
void ADCInit();
int ADCSingleRead();
//void SendRequest();
//void ReceiveRequest();

int const big_thresh_max = 110;
int const thresh_max = 45;
int const thresh_min = 10;
float initial_ADC = 0;
int ADC_reading = 0;
char state = 0;
char send_confirm = 0;
int speed = 10000;

char button = 0;

//main
int main(void){
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    GPIO();
    LEDSolid();
    timerInit();
    ADCInit();

    PM5CTL0 &= ~LOCKLPM5;
    P2IFG &= ~BIT3;                         // Clear P2.3 IFG
    P4IFG &= ~BIT1;                         // Clear P4.1 IFG
    __bis_SR_register(GIE);     // Enter LPM3 w/ interrupts

    while(1){
        switch(state){
        case 0:{ //read
            LEDSolid(); // Makes the LED Solid
            initial_ADC = ADCSingleRead();
            //ADC_reading = initial_ADC * 100;
            __delay_cycles(3000000);         // Delay for 3000000*(1/MCLK)=3s
            if ((ADC_reading > thresh_min) && (ADC_reading < thresh_max)){
                state = 1; // Set case to send case if the ADC_read value is within the threshold
            }
            break;
        }
        case 1:{ //send
            LEDSlow(); // Makes the LED blink slowly
            if (send_confirm){
                state = 2;     // If the waiter confirms the send by hitting 2.3, swap to receive state
            }
            break;
        }
        case 2:{ //receive
            send_confirm = 0; // Reset send_confirm
            LEDFast(); // Makes the led blink quickly
            initial_ADC = ADCSingleRead();
            //ADC_reading = initial_ADC * 100;
            __delay_cycles(3000000);          // Delay for 3000000*(1/MCLK)=3s
            if ((ADC_reading > thresh_max) && (ADC_reading < big_thresh_max)){
                state = 0; // Drink is back, so send back to read state
            }
            break;
        }
        }
    }
    return 0;
}

void GPIO(){
    P1OUT &= ~BIT0;  // RED LED on P1.0 as output
    P1DIR |= BIT0;

    P2OUT &= ~BIT0;  // External LED out
    P2DIR |= BIT0;

    P2OUT &= ~BIT2;  // Secondary output - if needed
    P2DIR |= BIT2;

    //P2OUT |= BIT3;   // ADC input
    //P2DIR &= ~BIT3;

    P1SEL0 |= BIT1;                         // Select P1.1 as OA0O function
    P1SEL1 |= BIT1;                         // OA is used as buffer for DAC


    P1SEL0 |= BIT5;   // ADC Port
    P1SEL1 |= BIT5;

    P1OUT &= ~BIT7;   // Transmit Port
    P1DIR |= BIT7;
    P1OUT |= BIT6;    // Receive Port
    P1DIR &= ~BIT6;

    // Configure Button on P2.3 as input with pullup resistor
    P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES &= ~BIT3;                         // P2.3 Low --> High edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

    // Configure Button on P4.1 as input with pullup resistor
    P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
    P4REN |= BIT1;                          // P4.1 pull-up register enable
    P4IES &= ~BIT1;                         // P4.1 Low --> High edge
    P4IE |= BIT1;                           // P4.1 interrupt enabled
}

void timerInit(){
    TB1CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
    TB1CCR0 = speed;                           // Triggers the timer every 50000 clock cycles
    TB1CTL = TBSSEL_1 | MC_2;                 // ACLK, continuous mode
}

void LEDSolid(){
    speed = 0;
    P1OUT |= BIT0;
}

void LEDSlow(){
    speed = 5000; // Make sure the red led is on.
}

void LEDFast(){
    speed = 2500; // temp
}

void ADCInit(){
    ADCCTL0 |= ADCSHT_2 | ADCON;                             // ADCON, S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;                                       // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                     // 12-bit conversion results
    ADCMCTL0 |= ADCINCH_1;                                   // A5 ADC input select; Vref=AVCC
}

int ADCSingleRead(){
    ADCCTL0 |= ADCENC | ADCSC; //Enable and start conversion
    while (!(ADCIFG & ADCIFG0));   // Wait for sample to be sampled and converted
    return ADCMEM0;
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                         // Clear P2.3 IFG
    if (state == 1){
        send_confirm = 1;
    }
}

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                         // Clear P4.1 IFG
    button++;
    if (button == 4){
        button = 1;
    }
    switch (button){
        case 1:{
            ADC_reading = 100;
            break;
        }
        case 2:{
            ADC_reading = 30;
            break;
        }
        case 3:{
            ADC_reading = 120;
            break;
        }
    }
}

#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void){
    if (speed > 0){
        P1OUT ^= BIT0;
        TB1CCR0 += speed; // Add offset to TB1CCR0
    }
}
