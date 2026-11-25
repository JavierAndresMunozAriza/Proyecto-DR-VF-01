/*
 * main.c - firmware entry point
 *
 * Every behavior lives in the modules under src/: the application controller
 * in src/app/, the fence in src/fence_manager/ and src/fence_codec/, the cloud
 * link in src/cloud/, the board adapters in src/gnss/, src/button/ and
 * src/led/. See app.h for how the application is split.
 */

#include "app.h"

int main(void)
{
	return app_run();
}
