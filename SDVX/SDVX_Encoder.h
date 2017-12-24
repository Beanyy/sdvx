#ifndef SDVX_Encoder_h
#define SDVX_Encoder_h

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

#endif