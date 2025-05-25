#include "NU32dip.h"

//#########################################################################################
// Global and defined variables
#define NUMSAMPS 1000
#define PLOTPTS 500
#define SAMPLE_TIME 6 
#define DECIMATION 10

static volatile int Waveform[NUMSAMPS];
static volatile int ADCarray[PLOTPTS];
static volatile int REFarray[PLOTPTS];
static volatile int StoringData = 0;
static volatile float Kp = 0.5 , Ki = 0.015;
static volatile int u = 0, unew = 0, e = 0, Eint = 0;



unsigned int adc_sample_convert(int pin)
{
  unsigned int elapsed = 0, finish_time = 0;
  AD1CHSbits.CH0SA = pin;
  AD1CON1bits.SAMP = 1;
  elapsed = _CP0_GET_COUNT();
  finish_time = elapsed + SAMPLE_TIME;
  while (_CP0_GET_COUNT() < finish_time)
  {
    ;
  }
  AD1CON1bits.SAMP = 0;
  while (!AD1CON1bits.DONE)
  {
    ;
  }
  return ADC1BUF0;
}


void ADC_Startup(){
    AD1CON1bits.ASAM = 0b0;
    AD1CON1bits.SSRC = 0b000;
    ANSELAbits.ANSA1 = 1; // AN1 is an adc pin
    AD1CON3bits.ADCS = 1; // ADC clock period is Tad = 2*(ADCS+1)*Tpb =2*2*(1/48000000Hz) = 83ns > 75ns
    AD1CON1bits.ADON = 1;
}


// Generate the reference waveform
void makeWaveform() {
    int i = 0, center = 500, A = 300; 
    for (i = 0; i < NUMSAMPS; ++i) {
        if ( i < NUMSAMPS/2) {
            Waveform[i] = center + A;
        } else {
            Waveform[i] = center - A;
        }
    }
}

//ISR Function###############################################
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) { // _TIMER_2_VECTOR = 8
    static int counter = 0; // initialize counter once(This is for OCxRS changing)
    static int plotind = 0; // index for data arrays; counts up to PLOTPTS(Plotting)
    static int decctr = 0; // counts to store data one every DECIMATION
    static int adcval = 0;
// OCxRS updating section
    adcval = adc_sample_convert(1);
    e = Waveform[counter] - adcval;
    Eint += e;
    u = Ki * Eint + Kp * e;
    unew = u + 50;
    if (unew > 100.0) {
        unew = 100.0;
    } else if (unew < 0.0) {
        unew = 0.0;
    }

    // A line should be here for PI control
    OC1RS = (unsigned int) ((unew/100.0) * PR3);
// Data Collecting Section
    if (StoringData){
        decctr++;
        if (decctr == DECIMATION){
            ADCarray[plotind] = adcval;
            REFarray[plotind] = Waveform[counter];
            decctr = 0;
            plotind++;
        }
        if (plotind == PLOTPTS){
            StoringData = 0;
            plotind = 0;
        }    
    }
    counter++; // add one to counter every time ISR is entered
    if (counter == NUMSAMPS) {
        counter = 0; // roll the counter over when needed  
    }

    IFS0bits.T2IF = 0;
}
//##################################################################

int main(){

    NU32DIP_Startup();

    // Generate the reference waveform
    makeWaveform();

    char message[100]; // message to and from PC
    float kptemp = 0, kitemp = 0; // temporary local gains
    int i = 0; // plot data counter
    int datapoints; // number of requested point


    // Interrupt Setting
    __builtin_disable_interrupts();

    T2CONbits.TCS = 0;
    T2CONbits.TCKPS = 0b000;
    PR2 = 48000;
    TMR2 = 0; 
    IPC2bits.T2IP = 0b101;  
    IPC2bits.T2IS = 0b00; 
    IFS0bits.T2IF = 0;  
    IEC0bits.T2IE = 1;  
    T2CONbits.ON = 1;  

    __builtin_enable_interrupts();

    // Output Compare Setting
    ANSELA = 0;
    RPA0Rbits.RPA0R = 0b0101;
    T3CON = 0; 
    OC1CON = 0;
    OC1CONbits.OC32 = 0;
    OC1CONbits.OCTSEL = 1;
    T3CONbits.TCKPS = 0;
    PR3 = 2400;
    TMR3 = 0;
    OC1CONbits.OCM = 0b110;
    OC1RS = 1200;
    OC1R = 1200;
    T3CONbits.ON = 1;
    OC1CONbits.ON = 1;

    ADC_Startup();

    while (1){

        // wait for oscope.py to send a command and number of data points to collect
        NU32DIP_ReadUART1(message, 100);
        sscanf(message, "%f %f %d" , &kptemp, &kitemp, &datapoints);
        if (datapoints > PLOTPTS){
            datapoints = PLOTPTS;
            }

        __builtin_disable_interrupts(); // keep ISR disabled as briefly as possible
        Kp = kptemp; // copy local variables to globals used by ISR
        Ki = kitemp;
        __builtin_enable_interrupts(); // only 2 simple C commands while ISRs disabled

        StoringData = 1;
        while (StoringData) { // wait until ISR says data storing is done
                ; // do nothing
            }
            // print the data back
        for (i = 0; i < datapoints; i++){
            sprintf(message, "%d %d %d\r\n", PLOTPTS - i, ADCarray[i],REFarray[i]);
            NU32DIP_WriteUART1(message);
            }
        }

    return 0;
}
