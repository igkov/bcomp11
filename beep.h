#ifndef __BEEP_H__
#define __BEEP_H__

void beep_init(void);
void beep(int time, int freq);
void beep_play(int *melody);
int beep_is_play(void);

#endif
