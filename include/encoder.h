#ifndef COMPONENTS_ESP32_ENCODER_INCLUDE_ENCODER_H_
#define COMPONENTS_ESP32_ENCODER_INCLUDE_ENCODER_H_

#include "driver/pcnt.h"

class Encoder
{
public:
	Encoder(int16_t pulsesPerRot = 0, int pcntFilter = 50);
	esp_err_t Install(gpio_num_t gpioA, gpio_num_t gpioB,
					  pcnt_unit_t pcnt_unit);
	int16_t GetCount();
	int GetRotations();

private:
	pcnt_unit_t _unit = PCNT_UNIT_MAX;
	const int _pcntFilter;

	const int16_t _pulsesPerRot;
	volatile int _rotations = 0;

	static void isr_handler(void *args);
};


#endif /* COMPONENTS_ESP32_ENCODER_INCLUDE_ENCODER_H_ */
