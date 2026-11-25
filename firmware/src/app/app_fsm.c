/*
 * app_fsm.c - application state machine, pure transition function
 *
 * See app_fsm.h for the transition table.
 */

#include "app_fsm.h"

enum app_state app_fsm_next(enum app_state state, enum app_event event)
{
	switch (state) {
	case APP_STATE_NO_FENCE:
		if (event == APP_EVENT_FENCE_CREATED)
			return APP_STATE_MONITORING;
		if (event == APP_EVENT_FENCE_LOADED || event == APP_EVENT_FENCE_UPDATED)
			return APP_STATE_WAIT_FIX;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		if (event == APP_EVENT_DEMO_TOGGLE)
			return APP_STATE_DEMO;
		return state;

	case APP_STATE_WAIT_FIX:
		if (event == APP_EVENT_FIX)
			return APP_STATE_MONITORING;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		if (event == APP_EVENT_DEMO_TOGGLE)
			return APP_STATE_DEMO;
		return state;

	case APP_STATE_MONITORING:
		if (event == APP_EVENT_FIX_TIMEOUT)
			return APP_STATE_FIX_LOST;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		if (event == APP_EVENT_DEMO_TOGGLE)
			return APP_STATE_DEMO;
		return state;

	case APP_STATE_FIX_LOST:
		if (event == APP_EVENT_FIX)
			return APP_STATE_MONITORING;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		if (event == APP_EVENT_DEMO_TOGGLE)
			return APP_STATE_DEMO;
		return state;

	case APP_STATE_FAULT:
		return event == APP_EVENT_DEMO_TOGGLE ? APP_STATE_DEMO : state;

	case APP_STATE_DEMO:
		/* Only the user leaves the demo; GNSS and fence events do not apply. */
		return event == APP_EVENT_DEMO_TOGGLE ? APP_STATE_NO_FENCE : state;

	default:
		/* An unknown state is treated as a fault. */
		return APP_STATE_FAULT;
	}
}

const char *app_fsm_state_name(enum app_state state)
{
	switch (state) {
	case APP_STATE_NO_FENCE:
		return "NO_FENCE";
	case APP_STATE_WAIT_FIX:
		return "WAIT_FIX";
	case APP_STATE_MONITORING:
		return "MONITORING";
	case APP_STATE_FIX_LOST:
		return "FIX_LOST";
	case APP_STATE_FAULT:
		return "FAULT";
	case APP_STATE_DEMO:
		return "DEMO";
	default:
		return "UNKNOWN";
	}
}
