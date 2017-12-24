#ifndef SDVX_Leds_h
#define SDVX_Leds_h

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

#endif