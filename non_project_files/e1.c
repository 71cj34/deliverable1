//#define PERIOD 1000
//#include <stdint.h>
//#include "tm4c1294ncpdt.h"
//#include "PLL.h"
//#include "SysTick.h"


// void PortM_Init(void){
// 	//Use PortM pins (PM0-PM3) for output
// 	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;		// activate clock for Port M
// 	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};	// allow time for clock to stabilize
// 	GPIO_PORTM_DIR_R |= 0x0F;        			// configure Port M pins (PM0-PM3) as output
// 	GPIO_PORTM_AFSEL_R &= ~0x0F;     				// disable alt funct on Port M pins (PM0-PM3)
// 	GPIO_PORTM_DEN_R |= 0x0F;        				// enable digital I/O on Port M pins (PM0-PM3)
// 													// configure Port M as GPIO
// 	GPIO_PORTM_AMSEL_R &= ~0x0F;     				// disable analog functionality on Port M pins (PM0-PM3)
// 	return;
// }

//void DutyCycle_Percent(int duty_val) {
//    int PctTimeOn = (duty_val * PERIOD) / 255;
//    int PctTimeOff = PERIOD - PctTimeOn;

//    GPIO_PORTM_DATA_R |= 0b00000111;
//    // setup another pin out here
//    SysTick_Wait10us(PctTimeOn);

//    // Toggle OFF
//    GPIO_PORTM_DATA_R &= ~0b00000111;
//    SysTick_Wait10us(PctTimeOff);
//}


//void IntensitySteps(void) {
//    int duty;
//    int i;

//    // INCREASING
//    for (int step = 0; step <= 10; step++) {
//        duty = (step * 255) / 10;

//        for (i = 0; i < 10; i++) {
//            DutyCycle_Percent((int)duty);
//        }
//    }

//    // DECREASING
//    for (int step = 9; step >= 0; step--) {
//        duty = (step * 255) / 10;

//        for (i = 0; i < 10; i++) {
//            DutyCycle_Percent((int)duty);
//        }
//    }
//}


//int main(void) {
//    PLL_Init();
//    SysTick_Init();
//    PortM_Init();
//    while (1) {
//        IntensitySteps();
//    }
//}


/////////////////

 #include <stdint.h>
 #include "tm4c1294ncpdt.h"
 #include "PLL.h"
 #include "SysTick.h"


 void PortL_Init(void){
 	//Use PortL pins (PM0-PM3) for output
 	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R10;		// activate clock for Port M
 	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R10) == 0){};	// allow time for clock to stabilize
 	GPIO_PORTL_DIR_R |= 0x0F;        			// configure Port M pins (PM0-PM3) as output
 	GPIO_PORTL_AFSEL_R &= ~0x0F;     				// disable alt funct on Port M pins (PM0-PM3)
 	GPIO_PORTL_DEN_R |= 0x0F;        				// enable digital I/O on Port M pins (PM0-PM3)
 													// configure Port M as GPIO
 	GPIO_PORTL_AMSEL_R &= ~0x0F;     				// disable analog functionality on Port M pins (PM0-PM3)
 	return;
 }

 // 0 = CW, 1 = CCW, steps should % 512
 void spin(int steps, int dir){
 	int delay = 300;					// !! turn this down to find min value (this is 10ms per) (2)
	int seq[4] = {0b0011, 0b0110, 0b1100, 0b1001};
	int seqCW[4] = {0b1001,0b1100,0b0110,0b0011};
	for(int i=0; i< steps; i++){  // steps should be 1024 (3)
		if (dir == 0) {
		    GPIO_PORTL_DATA_R = seq[i % 4];
		} else {
		    GPIO_PORTL_DATA_R = seqCW[i % 4];
		}
		SysTick_Wait10ms(delay);
	}
	if (dir == 1) {
		 	for(int i=0; i< steps/4; i++){												// What should the upper-bound of i be for one complete rotation of the motor shaft?
		GPIO_PORTL_DATA_R = 0b00000011;
		SysTick_Wait10ms(delay);											// What if we want to reduce the delay between steps to be less than 10 ms?
		GPIO_PORTL_DATA_R = 0b0110;													// Complete the missing code.
		SysTick_Wait10ms(delay);
		GPIO_PORTL_DATA_R = 0b1100;													// Complete the missing code.
		SysTick_Wait10ms(delay);
		GPIO_PORTL_DATA_R = 0b1001;													// Complete the missing code.
		SysTick_Wait10ms(delay);
	}
	} else {
		 	for(int i=0; i< steps/4; i++){												// What should the upper-bound of i be for one complete rotation of the motor shaft?
		GPIO_PORTL_DATA_R = 0b1001;
		SysTick_Wait10ms(delay);											// What if we want to reduce the delay between steps to be less than 10 ms?
		GPIO_PORTL_DATA_R = 0b1100;													// Complete the missing code.
		SysTick_Wait10ms(delay);
		GPIO_PORTL_DATA_R = 0b0110;													// Complete the missing code.
		SysTick_Wait10ms(delay);
		GPIO_PORTL_DATA_R = 0b00000011;													// Complete the missing code.
		SysTick_Wait10ms(delay);
	}}
 }


 int main(void){
 	PLL_Init();						// Default Set System Clock to 120MHz
 	SysTick_Init();					// Initialize SysTick configuration
 	PortL_Init();					// Initialize Port L
// 	while (1) {spin(512, 0);}							// Call function spin
 	// use this for (4)
 	spin(4096, 0);
 	spin(2048, 1);
 	return 0;
 }
