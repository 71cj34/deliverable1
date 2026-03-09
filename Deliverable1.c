#include <stdint.h>

#include <math.h>

#include <stdlib.h>

#include "tm4c1294ncpdt.h"

#include "SysTick.h"

#include "PLL.h"

// NOTES AFTER DEMO
// return to home must turn around and go back
// do not use cumulative degrees for the 360 degree timeouts

void PortH_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7; // Activate the clock for Port E
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R7) == 0) {}; // Allow time for clock to stabilize

    GPIO_PORTH_DIR_R = 0b00001111; // Enable PE0 and PE1 as outputs
    GPIO_PORTH_AFSEL_R &= ~0x0F;
    GPIO_PORTH_DEN_R = 0b00001111; // Enable PE0 and PE1 as digital pins
    GPIO_PORTH_AMSEL_R &= ~0x0F;
    return;
}

void PortM_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11; // Activate the clock for Port M
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R11) == 0) {}; // Allow time for clock to stabilize

    GPIO_PORTM_DIR_R = 0b00000000; // Enable PM0 and PM1 as inputs
    GPIO_PORTM_DEN_R = 0b00000011; // Enable PM0 and PM1 as digital pins

    // pull down resistors for active-high buttons
    GPIO_PORTM_PDR_R = 0b00000000;
    return;
}

void PortJ_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8; // Activate the clock for Port M
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R8) == 0) {}; // Allow time for clock to stabilize

    GPIO_PORTJ_LOCK_R = 0x4C4F434B;
    GPIO_PORTJ_CR_R |= 0b011;
    GPIO_PORTJ_LOCK_R = 0;

    GPIO_PORTJ_DIR_R = 0b00000000; // Enable PM0 and PM1 as inputs
    GPIO_PORTJ_DEN_R = 0b00000011; // Enable PM0 and PM1 as digital pins

    // pull down resistors for active-high buttons
    GPIO_PORTJ_PUR_R = 0b00000011;
    return;
}

//Enable LED D1, D2. Remember D1 is connected to PN1 and D2 is connected to PN0
void PortN_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12; // Activate the clock for Port N
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R12) == 0) {}; // Allow time for clock to stabilize

    GPIO_PORTN_DIR_R = 0b00001111; // Enable PN0 and PN1 as outputs
    GPIO_PORTN_DEN_R = 0b00001111; // Enable PN0 and PN1 as digital pins
    return;
}

//Enable LED D3, D4. Remember D3 is connected to PF4 and D4 is connected to PF0
void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // Activate the clock for Port F
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0) {}; // Allow time for clock to stabilize

    GPIO_PORTF_DIR_R = 0b00010001; // Enable PF0 and PF4 as outputs
    GPIO_PORTF_DEN_R = 0b00010001; // Enable PF0 and PF4 as digital pins
    return;
}

uint8_t readState(void) {
    uint8_t buttons = 0;
    buttons |= (~GPIO_PORTM_DATA_R & 0x03); // PM0 (bit 0), PM1 (bit 1)
    buttons |= (~(GPIO_PORTJ_DATA_R & 0x03) << 2); // PJ0 (bit 2), PJ1 (bit 3)
    // j1 j0 m1 m0
    // b1 b0 b2 b3
    return buttons;
}

void spin(int steps, int dir) { // 2048 is one full rotation (?)
    // ENSURE SPIN IS ALWAYS CALLED WITH A MOD 4 STEPS (steps are not counted globally)
    int delay = 500;
    int seq[4] = {
        0b0011,
        0b0110,
        0b1100,
        0b1001
    };
    int seqCW[4] = {
        0b1001,
        0b1100,
        0b0110,
        0b0011
    };
    steps = abs(steps);
    for (int i = 0; i < steps; i++) {
        if (dir == 0) {
            GPIO_PORTH_DATA_R = seqCW[i % 4]; // check direction?
        } else {
            GPIO_PORTH_DATA_R = seq[i % 4];
        }
        SysTick_Wait10us(delay);
    }
}

int main(void) {
    PLL_Init();
    SysTick_Init();
    PortH_Init();
    PortM_Init();
    PortF_Init();
    PortN_Init();
    PortJ_Init();

    // statuses:
    // pn1 pn0 pf4 pf0

    // Turn off LEDs at reset
    GPIO_PORTF_DATA_R = 0b00000000;
    GPIO_PORTN_DATA_R = 0b00000000;

    int dir = 1; // 1 = cw, status on
//    GPIO_PORTN_DATA_R |= 0b00000001; // should this be here?
    int on = 0;
    double angle = 11.25; // ieee 754 has an exact repr of this decimal so floating point inacc should be fine
//    GPIO_PORTF_DATA_R |= 0b00010000;
    int stop = 0;
    float degreestraveled = 0;
    uint8_t last_bt = 0xFF;
    float cumdeg = 0;

    uint16_t pressed = 0;

    while (1) {
        SysTick_Wait10us(50);
        pressed = 0;

        uint8_t current_bt = readState();
        // gets rising edge
        pressed = current_bt & (~last_bt);
        last_bt = current_bt;

        // on button (this one saves all progress!)
        if (pressed & 0b0100) {
            on ^= 1;
            GPIO_PORTN_DATA_R ^= (1 << 1);

            if (on) {
                // sets defaults in case a STOP clear
                angle = 11.25;
                dir = 1;

                GPIO_PORTN_DATA_R |= 0b00000001;
                GPIO_PORTF_DATA_R |= 0b00010000;
            }
        }

        // shutdown sequence (happens only for 0x0001)
        if (stop) {
            // remainderf is negative when arg1 is closest to a higher arg2 multiple (for some reason)
            float degrees = -remainderf(degreestraveled, 360.0);
            int steps = (int)((degrees / 360.0) * 2048); // this is guaranteed to be an int % 4 since n11.25 % 360
            if (degrees > 0) {
                // POSITIVE = CW!! Make sure this is consistent
                spin(steps, 0);
            } else {
                spin(steps, 1);
            }

            // reset
            GPIO_PORTN_DATA_R &= ~(1 << 1);
            on = 0;
            stop = 0;
            degreestraveled = 0;
            cumdeg = 0.0;
        }

        // stop all detection/movement if off
        // only thing running is on-button detection
        // THIS IS NOT THE SAME THING AS STOP!!!!!! it just happens that stop sequence -> off loop
        if (!on) {
            continue;
        }

        // direction button
        if (pressed & 0b1000) {
            dir ^= 1;
            GPIO_PORTN_DATA_R ^= 1;
        }

        // angle button
        if (pressed & 0b0010) {
            if (fabs(angle) == 11.25) {
                angle = 45.0;
            } else {
                angle = 11.25;
            }

            if (fabs(angle) == 11.25) {
                GPIO_PORTF_DATA_R |= 0b00010000;
            } else {
                GPIO_PORTF_DATA_R &= ~(1 << 4);
            }
        }

        // return to home button
        if (pressed & 0b0001) {
            stop = 1;
            GPIO_PORTN_DATA_R = 0b00000000;
            GPIO_PORTF_DATA_R = 0b00000000;
            continue;
        }

        ////////// movement

        // for the case where angle = 45 but 360 - cumdeg < 45 (due to iterations with 11.25)
        if ((360.0 - cumdeg) <= fabs(angle)) {
            int steps = (int)(((360 - cumdeg) / 360.0) * 2048);
            spin(steps, dir);
            on = 0;
            GPIO_PORTN_DATA_R &= ~(1 << 1);
            cumdeg = 0.0;
            degreestraveled = 0.0;
					
					  // needs to flash one more time for the exact 11.25/45 degree cases
						if (fabs(fabs(360-cumdeg) - 11.25) < 0.01) {
							GPIO_PORTF_DATA_R ^= 0b0001;
							SysTick_Wait10ms(1);
							GPIO_PORTF_DATA_R ^= 0b0001;						
						} else if (fabs(fabs(360-cumdeg) - 45.0) < 0.01) {							
							GPIO_PORTF_DATA_R ^= 0b0001;
							SysTick_Wait10ms(1);
							GPIO_PORTF_DATA_R ^= 0b0001;	
						}


            // not sure if this is part of spec but its free + defaults get set if its turned back on so why not :/
            GPIO_PORTN_DATA_R = 0b00000000;
            GPIO_PORTF_DATA_R = 0b00000000;
            continue;
        }

        if (dir) {
            // spin handles negative angle args
            spin(2048 / (360 / angle), 1);
            angle = -fabs(angle);
        } else {
            spin(2048 / (360 / angle), 0);
            angle = fabs(angle);
        }
        GPIO_PORTF_DATA_R ^= 0b0001;
        SysTick_Wait10ms(1);
        GPIO_PORTF_DATA_R ^= 0b0001;

        degreestraveled += angle;
        cumdeg += fabs(angle);

    }
}