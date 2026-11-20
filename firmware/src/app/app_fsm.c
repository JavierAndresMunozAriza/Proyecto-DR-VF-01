/*
 * app_fsm.c - application state machine, pure transition function
 *
 * See app_fsm.h for the transition table.
 */

#include "app_fsm.h"

enum app_state app_fsm_next(enum app_state state, enum app_event event)
{
	switch (state) {
	case APP_STATE_SEARCHING:
		if (event == APP_EVENT_FENCE_CREATED)
			return APP_STATE_MONITORING;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		return state;

	case APP_STATE_MONITORING:
		if (event == APP_EVENT_FIX_TIMEOUT)
			return APP_STATE_FIX_LOST;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		return state;

	case APP_STATE_FIX_LOST:
		if (event == APP_EVENT_FIX)
			return APP_STATE_MONITORING;
		if (event == APP_EVENT_GNSS_FAILED)
			return APP_STATE_FAULT;
		return state;

	case APP_STATE_FAULT:
	default:
		/* FAULT is final, and an unknown state is treated as a fault. */
		return APP_STATE_FAULT;
	}
}
