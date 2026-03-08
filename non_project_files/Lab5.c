#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "Systick.h"
#include "PLL.h"

void PortE_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;		          // Activate the clock for Port E
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R4) == 0){};	      // Allow time for clock to stabilize

	GPIO_PORTE_DIR_R = 0b00001111;							  // Enable PE0 and PE1 as outputs
	GPIO_PORTE_DEN_R = 0b00001111;                        	  // Enable PE0 and PE1 as digital pins
	return;
	}

void PortM_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;                 // Activate the clock for Port M
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R11) == 0){};      // Allow time for clock to stabilize

	GPIO_PORTM_DIR_R = 0b00000000;       					  // Enable PM0 and PM1 as inputs
    GPIO_PORTM_DEN_R = 0b00001111;							  // Enable PM0 and PM1 as digital pins

	// pull up resistors, check the syntax for this
	GPIO_PORTM_PUR_R = 0b00001111;
	GPIO_PORTM_DEN_R = 0b00001111;
	return;
}

//Enable LED D1, D2. Remember D1 is connected to PN1 and D2 is connected to PN0
void PortN_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;                 // Activate the clock for Port N
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};		  // Allow time for clock to stabilize

	GPIO_PORTN_DIR_R=0b00001111;							  // Enable PN0 and PN1 as outputs
	GPIO_PORTN_DEN_R=0b00001111;							  // Enable PN0 and PN1 as digital pins
	return;
}

//Enable LED D3, D4. Remember D3 is connected to PF4 and D4 is connected to PF0
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;                 	// Activate the clock for Port F
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};		// Allow time for clock to stabilize

	GPIO_PORTF_DIR_R=0b00010001;							// Enable PF0 and PF4 as outputs
	GPIO_PORTF_DEN_R=0b00010001;							// Enable PF0 and PF4 as digital pins
	return;
}

uint8_t decode(uint8_t code) {
	switch(code) {
		case 0b11111111: return 0x0;
		default: return 0x00;
		// fill this out
	}
}

void milestone3(uint8_t key) {
	val = decode(key);
	// pn1 pn0 pf4 pf0
	// bit3 bit2 bit1 bit0
	GPIO_PORTN_DATA_R &= ~0x03; // clear pn1/pn0
	GPIO_PORTF_DATA_R &= ~0x11; // clear pf4/pf0

	GPIO_PORTN_DATA_R |= (val >> 2) & 0x03; // val bits 3/2 -> pn bits 1/0
	GPIO_PORTF_DATA_R |= (val & 0x02) << 3; // val bit 1 -> pf bit 4
	GPIO_PORTF_DATA_R |= (val & 0x01); // val bit 0 -> pf bit 0
	return;
}

uint8_t key = 0;
uint8_t val = 0;

int main(void){
    PLL_Init();
    PortE_Init();
    PortM_Init();
    PortF_Init();
    PortN_Init();

	// Turn off LEDs at reset
	GPIO_PORTF_DATA_R = 0b00000000;
	GPIO_PORTN_DATA_R = 0b00000000;


    while(1){
        for(int i = 0; i < 4; i++){
            // 1. drive one row LOW (PE0 -> PE1 -> PE2 -> PE3)
            // ~(1 << i) = 1110, 1101, 1011, 0111
			// this will nuke the other pins from existence...
            GPIO_PORTE_DATA_R = ~(1 << i) & 0x0F;

            SysTick_Wait(100);

            // 2. read the columns (PM3-0)
            uint8_t cols = GPIO_PORTM_DATA_R & 0x0F;

            // 3. press detected if this is not just 1111
            if(cols != 0x0F){
                // ?
                key = (cols << 4) | (GPIO_PORTE_DATA_R & 0x0F);

                // debounce
                while((GPIO_PORTM_DATA_R & 0x0F) != 0x0F);
            }

			// milestone3(key);
        }
    }
}
