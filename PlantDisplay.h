
#define N_TABS 9

// #include "i2c_display.h"


class PlantDisplay
{
	public:
		PlantDisplay ();
		void update();
		void switchTab (uint8_t n);
		void nextTab();
		void countDown (uint32_t ms);
		void loop();

		boolean autodisplay;
	private:
		uint8_t curr_tab, minutes, seconds;
		uint8_t humidity, state;
};



PlantDisplay::PlantDisplay ()
{
	autodisplay = false;

	// init. i2c display
}


void PlantDisplay::update()
{
	// layout
	// display current tab
	// display minutes, seconds (countdown)
	// display humidity
}

void PlantDisplay::switchTab (uint8_t n)
{
	curr_tab = n;
}

void PlantDisplay::nextTab ()
{
	curr_tab++;
	update();
}

void PlantDisplay::countDown (uint32_t ms)
{
	minutes = ms / 60000;
	seconds = (ms / 1000) % 60;
}
