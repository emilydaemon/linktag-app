#ifndef HWBUTTON_H
#define HWBUTTON_H

void console_power_callback();
void console_reset_callback();
void remote_power_callback();

void init_hwbutton();

void hwbutton_check();

#endif
