#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>
#include "FastLED_RGBW.h"

#define ENCODER_OPTIMIZE_INTERRUPTS
#define ENCODER_USE_INTERRUPTS
#include "Encoder.h"
#include "SDVX.h"


enum animation_type {
	ANI_DEFAULT = 0,
	ANI_RAINBOW,
	ANI_RAINBOW_HALF,
	ANI_SPARKLE,
	//Place all animations with motion before here
	ANI_CLEAR,
	ANI_CLEAR2,
	ANI_METER,
	ANI_MAX
};

SDVX_Encoder encL(ENCODER_L_PIN_A, ENCODER_L_PIN_B);
SDVX_Encoder encR(ENCODER_R_PIN_A, ENCODER_R_PIN_B);
SDVX_Button btns[NUM_BUTTONS] =
{
	SDVX_Button(BTN_A_PIN, BTN_A_LED, 1),
	SDVX_Button(BTN_B_PIN, BTN_B_LED, 2),
	SDVX_Button(BTN_C_PIN, BTN_C_LED, 3),
	SDVX_Button(BTN_D_PIN, BTN_D_LED, 4),
	SDVX_Button(FX_L_PIN, FX_L_LED,   5),
	SDVX_Button(FX_R_PIN, FX_R_LED,   6),
	SDVX_Button(START_PIN, START_LED, 7)
};
SDVX_Leds<WS2812B, LED_STRIP_PIN, RGB, NUM_LEDS> encoder_leds;
SDVX_Leds<WS2812B, SIDE_LED_STRIP_PIN, RGB, NUM_SIDE_LEDS> side_leds;
SDVX_Debug debug(btns);

uint8_t led_ani_delays[ANI_MAX] = {20, 0, 40, 10, 50, 50, 50};
uint8_t side_ani_delays[ANI_MAX] = {100, 0, 40, 10, 50, 50, 50};

CHSV right_led_color;
CHSV left_led_color;
uint8_t right_led_white;
uint8_t left_led_white;

CHSV debug_color[2];
uint8_t encoder_sensitivity = 1;
uint8_t min_delay = 3;
uint8_t sparkle_count = 3;
uint8_t encoder_main_ani = ANI_DEFAULT;
uint8_t side_main_ani = ANI_DEFAULT;
SDVX_Value *meter_value = NULL;

void setup() {
	encL.m_color = CHSV(HUE_BLUE, 255, 255);
	encR.m_color = CHSV(HUE_PURPLE, 255, 255);

	right_led_color = CHSV(HUE_GREEN, 255, 255);
	left_led_color = CHSV(HUE_GREEN, 255, 255);

	debug_color[0] = CHSV(255, 255, 255);
	debug_color[1] = CHSV(255, 255, 255);
	//Animation Speed (3), Encoder sensitivity, min delay value, light enablement, button LEDs
	//4 active: animation sel(buttons only)
	//3 active: Animation delays Default, rainbow, rainbow half, sparkle
	//2 active: sensitivity, mindelay, sparkle count
	//1 active: 0x1: encL/R+val 0x2: sideL/R+val 0x4: debug[0:1]+val 0x8: sideL/R White
	//0 active: presentation mode
	debug.set_values(0x0, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL));

	debug.set_values(0x1, SDVX_Value(&encL.m_color.hue), SDVX_Value(&encR.m_color.hue), SDVX_Value(&encL.m_color.val, 0, 255, -1), SDVX_Value(&encL.m_color.val, 0, 255, 1), 0);
	debug.set_values(0x2, SDVX_Value(&left_led_color.hue), SDVX_Value(&right_led_color.hue), SDVX_Value(&left_led_color.val, 0, 255, -1), SDVX_Value(&left_led_color.val, 0, 255, 1), 0);
	debug.set_values(0x4, SDVX_Value(&(debug_color[0].hue)), SDVX_Value(&(debug_color[1].hue)), SDVX_Value(&(debug_color[0].val), 0, 255, -1), SDVX_Value(&(debug_color[0].val), 0, 255, 1), 0);
	debug.set_values(0x8, SDVX_Value(&left_led_white, 0, 255, 1), SDVX_Value(&right_led_white, 0, 255, 1), SDVX_Value(NULL), SDVX_Value(NULL));

	debug.set_values(0x3, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(&encoder_sensitivity, 0, 10, -1), SDVX_Value(&encoder_sensitivity, 0, 10, 1), 1);
	debug.set_values(0x5, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(&min_delay, 0, 10, -1), SDVX_Value(&min_delay, 0, 10, 1), 1);
	debug.set_values(0x6, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(&sparkle_count, 0, 10, -1), SDVX_Value(&sparkle_count, 0, 10, 1), 1);
	debug.set_values(0x9, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL));
	debug.set_values(0xA, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL));
	debug.set_values(0xC, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(NULL));

	debug.set_values(0x7, SDVX_Value(NULL), SDVX_Value(&side_ani_delays[ANI_DEFAULT], 0, 100, -1), SDVX_Value(&led_ani_delays[ANI_DEFAULT], 0, 100, 1), SDVX_Value(&led_ani_delays[ANI_DEFAULT], 0, 100, -1), 1);
	debug.set_values(0xB, SDVX_Value(&led_ani_delays[ANI_RAINBOW], 0, 100, -1), SDVX_Value(&side_ani_delays[ANI_RAINBOW], 0, 100, -1), SDVX_Value(NULL), SDVX_Value(NULL));
	debug.set_values(0xD, SDVX_Value(&led_ani_delays[ANI_RAINBOW_HALF], 0, 100, -1), SDVX_Value(&side_ani_delays[ANI_RAINBOW_HALF], 0, 100, -1), SDVX_Value(NULL), SDVX_Value(NULL));
	debug.set_values(0xE, SDVX_Value(&led_ani_delays[ANI_SPARKLE], 0, 100, -1), SDVX_Value(&side_ani_delays[ANI_SPARKLE], 0, 100, -1), SDVX_Value(NULL), SDVX_Value(NULL));

	debug.set_values(0xF, SDVX_Value(NULL), SDVX_Value(NULL), SDVX_Value(&encoder_main_ani), SDVX_Value(&side_main_ani), 1);
}

void loop() {
	uint8_t encoder_animation = encoder_main_ani % 6;
	uint8_t side_animation = side_main_ani % 6;
	uint32_t cur_time = millis();
	float encoder_output_multipler = (0.89f * (encoder_sensitivity+1))/2;
	static int32_t encL_pos = 0;
	static int32_t encR_pos = 0;
	static uint32_t led_prev_time;
	static uint32_t side_prev_time;
	static uint8_t side_led_hue;

	for(int i=0; i<NUM_BUTTONS; i++)
		btns[i].update();

	debug.process(); //Check if we enter/exit debug mode

	//This only really does stuff if we're in debug mode. But we do it anyways to determine the sensitivity to use
	debug.update_and_display_state();
	struct updateable_values* update_vals = debug.get_values();

	// Lower sensitivity if the encoders are going to be used to modify a value
	int x_move = encL.update(update_vals->encL.m_value ? encoder_sensitivity*8 : encoder_sensitivity);
	int y_move = encR.update(update_vals->encR.m_value ? encoder_sensitivity*8 : encoder_sensitivity);

	if (!debug.mode()) { //Regular Operation
		for(int i=0; i<NUM_BUTTONS; i++) {
			btns[i].set_led(btns[i].state());
			if (btns[i].edge())
				btns[i].output(btns[i].edge() == 1);
		}
		if (x_move) {
			encL_pos -= x_move;
			Joystick.X(((int)(encL_pos*encoder_output_multipler))&0x3FF); 
		}
		if (y_move) {
			encR_pos -= y_move;
			Joystick.Y(((int)(encR_pos*encoder_output_multipler))&0x3FF); 
		}
		if (btns[START_IDX].state()) //Start button animation
			encoder_animation = ANI_RAINBOW_HALF;

	} else { //Debug Mode

		update_vals->encL.update(x_move);
		update_vals->encR.update(y_move);

		if (update_vals->edge) {
			update_vals->btnL.update(btns[FX_L_IDX].edge() > 0);
			update_vals->btnR.update(btns[FX_R_IDX].edge() > 0);
		} else {
			update_vals->btnL.update(btns[FX_L_IDX].state());
			update_vals->btnR.update(btns[FX_R_IDX].state());
		}

		//Determine animations based on debug state
		switch(debug.state()) {
		case 0xF:
			break;
		case 0x1:
		case 0x2:
		case 0x8:
			encoder_animation = ANI_CLEAR2; side_animation = ANI_CLEAR2;
			encR.m_color.val = encL.m_color.val;
			right_led_color.val = left_led_color.val;
			break;
		case 0x4:
			encoder_animation = ANI_CLEAR; side_animation = ANI_CLEAR;
			break;
		case 0x3:
		case 0x5:
			encoder_animation = ANI_METER;
			meter_value = &(update_vals->btnL);
			break;
		case 0x7:
			encoder_animation = ANI_DEFAULT; side_animation = ANI_DEFAULT;
			break;
		case 0xB:
			encoder_animation = ANI_RAINBOW; side_animation = ANI_RAINBOW;
			break;
		case 0xD:
			encoder_animation = ANI_RAINBOW_HALF; side_animation = ANI_RAINBOW_HALF;
			break;
		case 0x6:
		case 0xE:
			encoder_animation = ANI_SPARKLE; side_animation = ANI_SPARKLE;
			break;
		case 0x0:
			encoder_animation = ANI_RAINBOW; side_animation = ANI_CLEAR2;
			break;
		}
	}

	//Execute led animations
	uint32_t led_elapsed_time = cur_time - led_prev_time;
	if(led_elapsed_time >= led_ani_delays[encoder_animation]) {
		led_prev_time = cur_time;
		switch(encoder_animation) {
		case ANI_RAINBOW:
			encoder_leds.rainbow();
			break;
		case ANI_RAINBOW_HALF:
			encoder_leds.color_clear(CRGB::Black);
			encoder_leds.rainbow_half();
			break;
		case ANI_SPARKLE:
			encoder_leds.sparkle(sparkle_count);
			break;
		case ANI_CLEAR:
			encoder_leds.color_clear(debug_color[0]);
			break;
		case ANI_CLEAR2:
			encoder_leds.color_clear_2(CRGBW(encL.m_color), CRGBW(encR.m_color));
			break;
		case ANI_METER:
			encoder_leds.meter(meter_value);
			break;
		default:
			encoder_leds.color_clear(CRGB::Black);
			encL.update_led(encoder_leds.leds(), encoder_leds.numLeds());
			encR.update_led(encoder_leds.leds(), encoder_leds.numLeds());
			break;
		}
		encoder_leds.showLeds();
	}

	//Execute side led animations
	uint32_t side_elapsed_time = cur_time - side_prev_time;
	if(side_elapsed_time >= side_ani_delays[side_animation]) {
		side_prev_time = cur_time;
		switch(side_animation) {
		case ANI_RAINBOW:
			side_leds.rainbow();
			break;
		case ANI_RAINBOW_HALF:
			side_leds.color_clear(CRGB::Black);
			side_leds.rainbow_half();
			break;
		case ANI_SPARKLE:
			side_leds.sparkle(sparkle_count);
			break;
		case ANI_CLEAR:
			side_leds.color_clear(debug_color[1]);
			break;
		case ANI_CLEAR2:
			side_leds.color_clear_2(CRGBW(left_led_color, left_led_white), CRGBW(right_led_color, right_led_white));
			break;
		default:
			CHSV lled = left_led_color; CHSV rled = right_led_color;
			lled.h += side_led_hue; rled.h += side_led_hue;
			side_leds.color_clear_2(CRGBW(lled, left_led_white), CRGBW(rled, right_led_white));
			side_led_hue++;
			break;
		}
		side_leds.showLeds();
	}


	uint32_t duration = millis() - cur_time;
	if (duration < min_delay)
		delay(min_delay-duration);
}
