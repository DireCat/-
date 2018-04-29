#ifndef PLANT_CONTROLLER_H
#define PLANT_CONTROLLER_H

#include "Config.h"

class PlantController
{
	public:
		PlantController (
			const uint8_t * pinout_sens,
			const uint8_t * pinout_sens_e,
			const uint8_t * pinout_relay,
			const uint8_t plant_count);
		void setPlantFunction (void (*plant_function)(uint8_t n, boolean active));
		void loop();

		void checkNow ();
		void wateringMode();
		void waitingMode();

		struct plant_t 
		{
			boolean thirsty, error;
			boolean first_wcycle;
			boolean is_watering;
			boolean eow;
			uint8_t pin_sensor;
			uint8_t pin_sensor_e;
			uint8_t pin_relay;
			uint8_t humidity;
			uint8_t init_humidity;
			uint8_t timer;

		} * plants;

	private:
		void (*plant_function) (uint8_t n, boolean state);
		
		boolean watering, water_cycle_running;
		uint32_t tick;
		uint32_t t_check;

		uint16_t t_waiting;
		uint8_t t_next_cycle_timeout;
		uint8_t t_water_timeout, t_watering, t_watering_timeout;

		uint8_t watering_cycle, current_plant;

		boolean runPlanter (uint8_t n);
    	void runWateringCycle(uint8_t n);

		const uint8_t plant_count;
};


#endif // PLANT_CONTROLLER_H