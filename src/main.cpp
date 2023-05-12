#include "main.h"
#include "asset.h"
#include "gif-pros/gifclass.hpp"

ASSET(chip_gif)

void initialize() { }

void opcontrol() {
	Gif gif(chip_gif, lv_scr_act());

	while (true) {
		pros::delay(20);
	}
}
