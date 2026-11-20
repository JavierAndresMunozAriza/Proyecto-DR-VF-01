cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dr_vf_01)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE
	src/main.c
	src/alert/alert_color.c
	src/geofence/geofence.c
	src/led/rgb_led.c
	src/timing/timing.c
)
