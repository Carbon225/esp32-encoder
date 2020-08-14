#include "encoder.h"
#include "error_check_return.h"

#include "esp_log.h"
static const char *TAG = "encoder";

Encoder::Encoder(int16_t pulsesPerRot, int pcntFilter)
	: _pcntFilter(pcntFilter)
	, _pulsesPerRot(pulsesPerRot)
{

}

esp_err_t Encoder::Install(gpio_num_t gpioA, gpio_num_t gpioB,
						   pcnt_unit_t pcntUnit)
{
	_unit = pcntUnit;

	pcnt_config_t pcnt_config = {
			.pulse_gpio_num = gpioA,
			.ctrl_gpio_num = gpioB,
			.lctrl_mode = PCNT_MODE_KEEP,
			.hctrl_mode = PCNT_MODE_REVERSE,
			.pos_mode = PCNT_COUNT_INC,
			.neg_mode = PCNT_COUNT_DEC,
			.counter_h_lim = _pulsesPerRot,
			.counter_l_lim = (int16_t) -_pulsesPerRot,
			.unit = pcntUnit,
			.channel = PCNT_CHANNEL_0
	};

	ESP_ERROR_CHECK_RETURN(pcnt_unit_config(&pcnt_config));

	ESP_ERROR_CHECK_RETURN(pcnt_set_filter_value(pcntUnit, _pcntFilter));
	ESP_ERROR_CHECK_RETURN(pcnt_filter_enable(pcntUnit));

	ESP_ERROR_CHECK(pcnt_event_enable(pcntUnit, PCNT_EVT_H_LIM));
	ESP_ERROR_CHECK(pcnt_event_enable(pcntUnit, PCNT_EVT_L_LIM));

	esp_err_t ret = pcnt_isr_service_install(ESP_INTR_FLAG_LEVEL1);
	if (ret != ESP_ERR_INVALID_STATE && ret != ESP_OK)
		return ret;

	ESP_ERROR_CHECK_RETURN(pcnt_isr_handler_add(pcntUnit, isr_handler, this));

	ESP_ERROR_CHECK(pcnt_intr_enable(pcntUnit));

	ESP_ERROR_CHECK_RETURN(pcnt_counter_clear(pcntUnit));

	return ESP_OK;
}

int16_t Encoder::GetCount()
{
	int16_t count = -1;
	ESP_ERROR_CHECK_WITHOUT_ABORT(pcnt_get_counter_value(_unit, &count));
	return count;
}

int Encoder::GetRotations()
{
	return _rotations;
}

void Encoder::isr_handler(void *args)
{
	Encoder *obj = (Encoder*) args;

	if (PCNT.status_unit[obj->_unit].h_lim_lat)
		obj->_rotations++;

	if (PCNT.status_unit[obj->_unit].l_lim_lat)
		obj->_rotations--;
}
