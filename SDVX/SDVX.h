#ifndef main_h
#define main_h

#define BTN_A_IDX 0
#define BTN_B_IDX 1
#define BTN_C_IDX 2
#define BTN_D_IDX 3
#define FX_L_IDX  4
#define FX_R_IDX  5
#define START_IDX 6
#define NUM_BUTTONS 7

#define BTN_D_LED 12
#define BTN_D_PIN 13
#define BTN_C_LED 14
#define BTN_C_PIN 15
#define BTN_B_LED 16
#define BTN_B_PIN 17
#define BTN_A_LED 18
#define BTN_A_PIN 19

#define FX_L_LED 0
#define FX_L_PIN 1
#define FX_R_LED 2
#define FX_R_PIN 3

#define START_LED 20
#define START_PIN 21

#define ENCODER_L_PIN_A PIN_D0
#define ENCODER_L_PIN_B 4
#define ENCODER_R_PIN_A PIN_D1
#define ENCODER_R_PIN_B 23

#define NUM_LEDS 20
#define LED_STRIP_PIN 22

#define NUM_SIDE_LEDS 12
#define SIDE_LED_STRIP_PIN 9

#define CW 1
#define CCW -1

#define GET_BIT(val, i) ((val>>i) & 1)
#define FLIP_BIT(val, i) ((val>>i) & iU

#include "SDVX_Value.h"
#include "SDVX_Leds.h"
#include "SDVX_Button.h"
#include "SDVX_Encoder.h"


struct updateable_values {
	SDVX_Value encL;
	SDVX_Value encR;
	SDVX_Value btnL;
	SDVX_Value btnR;
	bool edge;
  updateable_values(){}
};

class SDVX_Debug {
private:
	SDVX_Button *m_btns;
	bool m_debug_mode;
	uint8_t m_debug_state;
	uint8_t m_debug_pos;
	struct updateable_values m_debug_values[16];
public:
	SDVX_Debug(SDVX_Button *btns):
		m_btns(btns),
		m_debug_mode(0),
		m_debug_pos(0) {}

	void process() {
		uint8_t i;
		int8_t rising_edge = -1;
		uint8_t debug_sequence[8] = {BTN_A_IDX,START_IDX,BTN_B_IDX,START_IDX,BTN_C_IDX,START_IDX,BTN_D_IDX,START_IDX};

		//Cancel debug mode on start press
		if (m_debug_mode) {
			if(m_btns[START_IDX].edge() > 0)
				m_debug_mode = 0;
			return;
		}

		//Get 1 rising edge
		for(i=0; i<NUM_BUTTONS; i++) {
			if(m_btns[i].edge() > 0) {
				if(rising_edge >= 0) { //2nd raiding edge found
					rising_edge = NUM_BUTTONS; //Set incorrect button
					break;
				}
				rising_edge = i;
			}
		}

		//No edge found
		if (rising_edge == -1) return;

		if (rising_edge == debug_sequence[m_debug_pos]) //Correct Button
			m_debug_pos++;
		else //Incorrect button
			m_debug_pos = 0;

		if (m_debug_pos >= sizeof(debug_sequence)/sizeof(uint8_t)) {
			m_debug_pos = 0;
			m_debug_mode = 1;
		}
	}

	void update_and_display_state()
	{
		if (!m_debug_mode) {
			m_debug_state = 0;
			return;
		}

		//Toggle debug state bits on positve edges
		if (m_btns[BTN_A_IDX].edge() > 0) m_debug_state ^= 1 << 3;
		if (m_btns[BTN_B_IDX].edge() > 0) m_debug_state ^= 1 << 2;
		if (m_btns[BTN_C_IDX].edge() > 0) m_debug_state ^= 1 << 1;
		if (m_btns[BTN_D_IDX].edge() > 0) m_debug_state ^= 1 << 0;

		m_btns[START_IDX].set_led(1);
		m_btns[FX_L_IDX].set_led(1);
		m_btns[FX_R_IDX].set_led(1);
		//Show debug state on main buttons
		if (m_debug_state) {
			m_btns[BTN_A_IDX].set_led((m_debug_state>>3) & 1);
			m_btns[BTN_B_IDX].set_led((m_debug_state>>2) & 1);
			m_btns[BTN_C_IDX].set_led((m_debug_state>>1) & 1);
			m_btns[BTN_D_IDX].set_led((m_debug_state>>0) & 1);
		} else {
			m_btns[BTN_A_IDX].set_led(1);
			m_btns[BTN_B_IDX].set_led(1);
			m_btns[BTN_C_IDX].set_led(1);
			m_btns[BTN_D_IDX].set_led(1);
		}
	}

	void set_values(uint8_t state, SDVX_Value encL, SDVX_Value encR, SDVX_Value btnL, SDVX_Value btnR, bool edge=1)
	{
		m_debug_values[state].encL = encL;
		m_debug_values[state].encR = encR;
		m_debug_values[state].btnL = btnL;
		m_debug_values[state].btnR = btnR;
		m_debug_values[state].edge = edge;
	}
	struct updateable_values* get_values() { return &m_debug_values[m_debug_state];}
	bool mode(){return m_debug_mode;}
	uint8_t state(){return m_debug_state;}
	uint8_t m_debug_hue;
};

#endif
