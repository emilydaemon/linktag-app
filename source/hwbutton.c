#include <gccore.h>
#include <wiiuse/wpad.h>

#include "util.h"

#include "hwbutton.h"

int hwbutton = -1;

void console_power_callback() {
	hwbutton = SYS_POWEROFF;
}
void console_reset_callback() {
	hwbutton = SYS_RETURNTOMENU;
}
void remote_power_callback() {
	hwbutton = SYS_POWEROFF;
}

void init_hwbutton() {
	SYS_SetPowerCallback(console_power_callback);
	SYS_SetResetCallback(console_reset_callback);
	WPAD_SetPowerButtonCallback(remote_power_callback);
}

void hwbutton_check() {
	if (hwbutton != -1) {
		softquit();
		SYS_ResetSystem(hwbutton, 0, 0);
	}
}
