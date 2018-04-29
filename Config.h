#ifndef CONFIG_H
#define CONFIG_H

// кол-во кустов
#define PLANT_COUNT 2

// кол-во циклов полива
#define WATERING_CYCLES 5

// поддерживаемый процент воды
#define WATERING_LEVEL 50

// время в секундах
#define WAITING_TIME_S 20
#define WATERING_TIME_S 2
#define CYCLE_TIMEOUT_S 12


// настройки для железа уровень датчиков и активный сигнал для реле
#define RV_WATER_EMPTY 1023
#define RV_WATER_FULL 250
#define SIGNAL_RELAY HIGH


#endif // CONFIG_H
