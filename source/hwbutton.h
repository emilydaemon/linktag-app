#ifndef HWBUTTON_H
#define HWBUTTON_H

void console_power_callback();
void console_reset_callback(u32 irq, void* ctx);
void remote_power_callback(int channel);

void init_hwbutton();

void hwbutton_check();

#endif
