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

struct debug_value {
	uint8_t *value;
	uint8_t increment;
	uint8_t edge;
};

class SDVX_Value{
	public:
		uint8_t *m_value;
		uint8_t m_min;
		uint8_t m_max;
		uint8_t m_wrap;
		int8_t m_increment;

	SDVX_Value(){}

		SDVX_Value(uint8_t *value, uint8_t min=0, uint8_t max=0, int8_t increment=1):
		 m_value(value),
		 m_min(min),
		 m_max(max),
		 m_wrap(min==max),
		 m_increment(increment) {}

		void update(int n) {
			if (!m_value || !n) return;

			int newValue = *m_value + (n*m_increment);
			if (!m_wrap && newValue > m_max)
				*m_value = m_max;
			else if (!m_wrap && newValue < m_min)
				*m_value = m_min;
			else
				*m_value = newValue;
		}
};

struct updateable_values {
	SDVX_Value encL;
	SDVX_Value encR;
	SDVX_Value btnL;
	SDVX_Value btnR;
	bool edge;
  updateable_values(){}
};

template<template<uint8_t DATA_PIN, EOrder RGB_ORDER>class CHIPSET, uint8_t DATA_PIN, EOrder RGB_ORDER, uint8_t NLEDS>
class SDVX_Leds{
private:
	CRGBW m_leds[NLEDS];
	uint8_t m_num_leds;
	uint8_t m_rainbow_hue;
	int8_t m_rainbow_pos;
	CLEDController *m_controller;
public:
	SDVX_Leds():
		m_num_leds(NLEDS),
		m_rainbow_hue(0),
		m_rainbow_pos(NLEDS/2-1),
		m_controller(&FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>((CRGB *)&m_leds[0], getRGBWsize(NLEDS)))
	{}

	void color_clear(CRGBW color)
	{
		for(int j = 0; j < m_num_leds; j++)
			m_leds[j] = color;
	}
	void color_clear(CRGB color)
	{
		for(int j = 0; j < m_num_leds; j++)
			m_leds[j] = color;
	}

	void color_clear_2(CRGBW color1, CRGBW color2)
	{
		int i;
		for(i=0; i<m_num_leds/2; i++)
			m_leds[i] = color1;
		for(i=m_num_leds/2; i<m_num_leds; i++)
			m_leds[i] = color2;
	}

	void rainbow_half()
	{
		CRGB color = CHSV((m_rainbow_pos * 256 / (m_num_leds/2)) + m_rainbow_hue, 255, 255);
		m_leds[m_rainbow_pos] = color;
		m_leds[m_num_leds-1-m_rainbow_pos] = color;
		m_rainbow_pos--;
		m_rainbow_hue++;
		if(m_rainbow_pos < 0)
			m_rainbow_pos= m_num_leds/2-1;
	}

	void rainbow()
	{
		for(int i = 0; i < m_num_leds; i++)
			m_leds[i] = CHSV((i * 256 / m_num_leds) + m_rainbow_hue, 255, 255);
		m_rainbow_hue++;
	}

	void meter(SDVX_Value *val)
	{
		if (!val) return;

		color_clear(CRGB::Black);
		float percentage = ((float)(*(val->m_value)))/((float)val->m_max);
		float leds = m_num_leds*percentage;
		int i;
		for(i = 0; i < (int)leds; i++)
			m_leds[i].w = 255;
		if(i < m_num_leds)
			m_leds[i].w = 255*(leds-(int)leds);
	}

	void sparkle(uint8_t num)
	{
		uint8_t i;
		color_clear(CRGB::Black);
		for(i=0; i<num; i++) {
			int rand = random(m_num_leds);
			m_leds[rand] = CHSV(random(256), 255, 255);
		}
	}

	CRGBW* leds(){ return m_leds;}
	uint8_t numLeds(){ return m_num_leds;}
	void showLeds() { m_controller->showLeds(); }
};

class SDVX_Button{
private:
	uint8_t m_btn_pin;
	uint8_t m_led_pin;
	uint8_t m_btn_val;
	uint8_t m_state;
	int8_t m_edge;
public:
	SDVX_Button(uint8_t button_pin, uint8_t led_pin, uint8_t btn_val):
		m_btn_pin(button_pin),
		m_led_pin(led_pin),
		m_btn_val(btn_val),
		m_state(0),
		m_edge(0)
	{
		pinMode(m_btn_pin, INPUT_PULLUP);
		pinMode(m_led_pin, OUTPUT);
	}

	void output(bool value)
	{
		Joystick.button(m_btn_val, value);
	}
	void update()
	{
		bool value = !digitalRead(m_btn_pin);

		if(m_state && !value)
			m_edge = -1;
		else if(!m_state && value)
			m_edge = 1;
		else
			m_edge = 0;

		m_state = value;
	}

	uint8_t state(){return m_state;}
	int8_t edge(){return m_edge;}
	void set_led(uint8_t value){digitalWrite(m_led_pin, value);}
};

class SDVX_Encoder: private Encoder {
private:
	int8_t m_dir;
	int8_t m_led_pos;
	int8_t m_led_dir;

public:
	SDVX_Encoder(uint8_t pin1, uint8_t pin2):
		Encoder(pin1, pin2),
		m_dir(0),
		m_led_pos(-1),
		m_led_dir(0),
		m_color(CHSV(0,0,0))
	{}

	int update(uint8_t sensitivity)
	{
		int pos = Encoder::read();
		if (pos >= (sensitivity+1) || pos <= (sensitivity+1)*-1) {
			int value = pos/(sensitivity+1);
			m_dir = (pos > 0)?CCW:CW;
			Encoder::write(pos-value*(sensitivity+1));
			return value;
		}
		m_dir = 0;
		return 0;
	}

	void update_led(CRGBW *leds, uint8_t max)
	{
		if (m_dir) {
			if (!m_led_dir)
				m_led_pos = (m_dir == CW)? -1 : max;
			m_led_dir = m_dir;
		}

		m_led_pos += m_led_dir;
		if (m_led_pos >= max || m_led_pos < 0)
			m_led_dir = 0;

		if(m_led_pos >=0 && m_led_pos < NUM_LEDS) {
			leds[m_led_pos] = leds[m_led_pos] + m_color;
		}
		if(m_led_pos >=1 && m_led_pos < NUM_LEDS) {
			leds[m_led_pos-1] = leds[m_led_pos-1] +(m_color/2);
		}

		if(m_led_pos >=0 && m_led_pos < NUM_LEDS-1) {
			leds[m_led_pos+1] = leds[m_led_pos+1] +(m_color/2);
		}
	}

	CHSV m_color;
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
