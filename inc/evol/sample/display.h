#ifndef _EVOL_SAMPLE_DISPLAY_H
#define _EVOL_SAMPLE_DISPLAY_H

#include <evol/sample/base.h>

#include <windows.h>

typedef void (*KEYDOWN_CALLBACK) (WPARAM vk);
typedef void (*KEYUP_CALLBACK) (WPARAM vk);

void display_start(KEYDOWN_CALLBACK kdCallback, KEYUP_CALLBACK kuCallback);

void display_view(GSA_VIEW * view);

void display_clean();

#endif // _EVOL_SAMPLE_DISPLAY_H
