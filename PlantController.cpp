
#include <Streaming.h>

#include "PlantController.h"

PlantController::PlantController (
	const uint8_t * pinout_sens, 
	const uint8_t * pinout_sens_e, 
	const uint8_t * pinout_relay, 
	const uint8_t plant_count) 
	: plant_count (plant_count)
{
	plants = (plant_t *) malloc (plant_count * sizeof (plant_t));
	for (uint8_t i=0; i < plant_count; i++)
	{
		plant_t * p = &plants[i];
		p -> thirsty = true;
		p -> error = false;
		p -> pin_sensor = pinout_sens[i];
		p -> pin_sensor_e = pinout_sens_e[i];
		p -> pin_relay = pinout_relay[i];
		pinMode (p -> pin_sensor_e, OUTPUT);
		pinMode (p -> pin_sensor, INPUT);
		pinMode (p -> pin_relay, OUTPUT);

		digitalWrite (p -> pin_relay, !SIGNAL_RELAY);
		digitalWrite (p -> pin_sensor_e, LOW);

		analogReference (EXTERNAL);
	}
	
	tick = millis();

	wateringMode();
  
}

void PlantController::checkNow()
{
  Serial << "Замер датчиков: \r\n";
  // Serial << "Measuring sensors: \r\n";
	for (uint8_t i=0; i < plant_count; i++)
		digitalWrite (plants[i].pin_sensor_e, HIGH);

	delay (200);
	for (uint8_t i=0; i < plant_count; i++)
	{
		//Serial << "PLANT #" << i+1 << ": ";
		Serial << "Куст #" << i+1 << ": ";

		plant_t * p = &plants[i];

		uint16_t rval = constrain (analogRead (p -> pin_sensor), RV_WATER_FULL, RV_WATER_EMPTY);
		//uint16_t rval = analogRead(p->pin_sensor);
		p -> humidity = map ((uint16_t)rval, RV_WATER_FULL, RV_WATER_EMPTY, 100, 0);

		Serial << p->humidity << "% ";
		if (p->humidity > WATERING_LEVEL)
		{
			p-> thirsty = false;
			Serial << "(нужно полить)";
		}
		else
		{
			p-> thirsty = true;
			Serial << "(ок)";
		}
		Serial << "\r\n";

		if (watering_cycle == 0)
			p -> init_humidity = p -> humidity;
	}
	for (uint8_t i=0; i < plant_count; i++)
		digitalWrite (plants[i].pin_sensor_e, LOW);
   Serial << "\r\n";
}


void PlantController::wateringMode()
{
	Serial << "\r\nПерехожу в режим полива\r\n";
	// Serial << "\r\nGoing to watering mode\r\n";
	watering = true;
	watering_cycle = 0;
	t_next_cycle_timeout = 0;
	water_cycle_running = false;
}

void PlantController::waitingMode ()
{
	Serial << "\r\nПерехожу в режим ожидания\r\n";
	//Serial << "\r\nGoing to waiting mode\r\n";
	t_waiting = WAITING_TIME_S;
	watering = false;
}


void PlantController::loop()
{
	// переодический таймер в 1 сек
	uint32_t t  = millis();
	if (t - tick >= 1000)
	{	
		tick = millis();

		if (t_next_cycle_timeout > 0)
			t_next_cycle_timeout--;

		// активен режим поливания
		if (watering)
		{
			// цикл полива окончен
			if (!water_cycle_running)
			{
				// цикл окончен но минмальная пауза не выдержана
				// надо еще подождать
				if (t_next_cycle_timeout > 0)
				{
					Serial << ".";
					return;
				}
				// перейти к следующему
				runWateringCycle (watering_cycle ++);
				// если последний, то перейти в режим ожидания
				if (watering_cycle >= WATERING_CYCLES)
					waitingMode();
				return;
			}

			// в процессе полива
			else
			{
				if (runPlanter (current_plant))
				{
					if (++current_plant >= plant_count)
					{
						water_cycle_running = false;
						Serial << "Последний куст полит.\r\n";
						// Serial << "Last plant has been watered.\r\n";

						if (t_next_cycle_timeout > 0)
							// Serial << "Need to wait for " << t_next_cycle_timeout << "/" \
							<< CYCLE_TIMEOUT_S << " s";
							Serial << "Последний цикл окончен. Нужно подождать еще " << \
							t_next_cycle_timeout << " / " << CYCLE_TIMEOUT_S << " сек";
					}
					/*
					else
						// Serial << "Начинаю поливать куст №" << current_plant+1 << "\r\n";
						Serial << "Begin watering plant #" << current_plant+1 << "\r\n";
					*/
				}
			}
		}
		// активен режим ожидания
		else if (t_waiting > 0)
		{
			Serial << ".";
			if (t_waiting % 10 == 0)
				Serial << "\r\n" << t_waiting;
			t_waiting --;
		}
		else
			wateringMode();

	} // конец таймера
}

void PlantController::runWateringCycle(uint8_t n)
{
	Serial << "\r\n\r\n*** Цикл полива №" << n+1 << "***\r\n";
	//Serial << "\r\n\r\n*** Water cycle #" << n+1 << " ***\r\n";
	checkNow();
	current_plant = 0;
	water_cycle_running = true;

	t_next_cycle_timeout = CYCLE_TIMEOUT_S;

	for (uint8_t i=0; i < plant_count; i++)
	{
		plant_t * p = &plants[i];
		p -> eow = false;

		// нужна вода хотябы 1 кусту
		if (p -> thirsty && !p -> error)
			watering = true;
		else
			continue;

		// первый цикл полива
		if (n == 0 && !p -> error)
			p -> first_wcycle = true;

		if ((n >= 3) && 
		(p -> humidity <=  p-> init_humidity + 2))
		{
			p -> error = true;
			p -> eow = true;
			Serial << "Пропущен изза ошибки: влажность не изменилась в течении 3х циклов ";
			// Serial << "skipped upon error ";
			Serial << "(была: " << p->init_humidity << ", ";
			Serial << "стала: " << p->humidity << ")\r\n";
			continue;
		}
		else if (!p->thirsty)
		{
			Serial << "в поливе не нуждается. \r\n";
			// Serial << "no need watering\r\n";
		}
	}
}


// функция для полива куста
boolean PlantController::runPlanter (uint8_t n) 
{
	plant_t * p = &plants[n];
	// если ошибка куста или он уже полит выйти
	if (p-> error || p-> eow || !p->thirsty)
	{
		Serial << "Куст №" << n+1 << " пропущен: ";
		if (p -> error)
			Serial << " ошибка - показания не изменились.";
		else if (p-> eow)
			Serial << " уже полит.";
		else if (!p-> thirsty)
			Serial << " в поливе не нуждается.";
		Serial << "\r\n";
		return true;
	}

	// отсчет таймера
	else if (p->timer > 0)
	{
		p-> timer --;
		Serial << ".";
		return false;
	}
	
	// если первый полив за цикл
	if (p-> first_wcycle)
	{
		p->first_wcycle = false; // он уже не первый 
		p->eow = false; 
		p->is_watering = false;
		p->timer = 0; //начать поливать его немедленно
	}

	// куст еще не поливается
	if (!p->is_watering)
	{
		digitalWrite (p->pin_relay, SIGNAL_RELAY); // включить реле
		Serial << "Поливаю куст №" << n+1 << "(" << WATERING_TIME_S << " сек)"; 
		// Serial << "Watering plant #" << n+1 << "(" << WATERING_TIME_S << "s)";
		//if (!p->eow)
		p->is_watering = true;
		p->timer = WATERING_TIME_S;		
	}
	// куст уже в процессе полива
	else
	{
		// полив: время вышло
		if (p->timer == 0)
		{
			// полив: время вышло
			digitalWrite (p->pin_relay, !SIGNAL_RELAY);	// отключить реле
			// Serial << "stopping watring plant #" << n+1 << "\r\n";
			Serial << "стоп\r\n";
			p->is_watering = false;
			p->eow = true;	// пометить куст как уже политый в этом цикле
			p->timer = 0;
			return true;
		}
	}

	return false;
}



void PlantController::setPlantFunction (void (*plant_function)(uint8_t n, boolean active))
{
	this -> plant_function = plant_function;
}


