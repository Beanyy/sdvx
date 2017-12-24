#ifndef SDVX_Button_h
#define SDVX_Button_h

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

#endif