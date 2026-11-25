cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dr_vf_01)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE
	src/main.c
	src/alert/alert_color.c
	src/app/app.c
	src/app/app_demo.c
	src/app/app_fix.c
	src/app/app_fsm.c
	src/app/app_led.c
	src/app/app_sync.c
	src/button/button.c
	src/demo/demo_route.c
	src/fence/fence_square.c
	src/fence_codec/fence_codec.c
	src/fence_manager/fence_manager.c
	src/geofence/geofence.c
	src/gnss/gnss_nrf91.c
	src/led/rgb_led.c
	src/telemetry/telemetry.c
	src/timing/timing.c
)

target_sources_ifdef(CONFIG_DRVF_FENCE_STORAGE app PRIVATE
	src/storage/fence_store_settings.c
)

target_sources_ifdef(CONFIG_DRVF_GEOFENCE_BENCHMARK app PRIVATE
	src/bench/bench_polygon.c
	src/bench/geofence_bench.c
)

target_sources_ifdef(CONFIG_DRVF_CLOUD app PRIVATE
	src/cloud/cloud_nrfcloud.c
	src/cloud/cloud_sync.c
)
