/*
 * Host unit tests for app_fsm_next(): every state/event combination.
 * Exits non-zero if any check fails.
 */

#include <stdio.h>

#include "app_fsm.h"

#define STATE_COUNT 4
#define EVENT_COUNT 4

static int failures;

static const char *const state_name[STATE_COUNT] = {
	"SEARCHING", "MONITORING", "FIX_LOST", "FAULT",
};
static const char *const event_name[EVENT_COUNT] = {
	"FENCE_CREATED", "FIX", "FIX_TIMEOUT", "GNSS_FAILED",
};

/* Expected next state for every (state, event) pair, in enum order. */
static const enum app_state expected[STATE_COUNT][EVENT_COUNT] = {
	/* SEARCHING  */ { APP_STATE_MONITORING, APP_STATE_SEARCHING,
			   APP_STATE_SEARCHING, APP_STATE_FAULT },
	/* MONITORING */ { APP_STATE_MONITORING, APP_STATE_MONITORING,
			   APP_STATE_FIX_LOST, APP_STATE_FAULT },
	/* FIX_LOST   */ { APP_STATE_FIX_LOST, APP_STATE_MONITORING,
			   APP_STATE_FIX_LOST, APP_STATE_FAULT },
	/* FAULT      */ { APP_STATE_FAULT, APP_STATE_FAULT,
			   APP_STATE_FAULT, APP_STATE_FAULT },
};

int main(void)
{
	for (int s = 0; s < STATE_COUNT; s++) {
		for (int e = 0; e < EVENT_COUNT; e++) {
			enum app_state got = app_fsm_next((enum app_state)s, (enum app_event)e);

			if (got != expected[s][e]) {
				printf("FAIL %s + %s: got %s want %s\n", state_name[s],
				       event_name[e], state_name[got],
				       state_name[expected[s][e]]);
				failures++;
			}
		}
	}

	/* The paths the application relies on, spelled out. */
	enum app_state s = APP_STATE_SEARCHING;

	s = app_fsm_next(s, APP_EVENT_FENCE_CREATED);
	s = app_fsm_next(s, APP_EVENT_FIX_TIMEOUT);
	s = app_fsm_next(s, APP_EVENT_FIX);
	if (s != APP_STATE_MONITORING) {
		printf("FAIL search -> fence -> lost -> recovered ends in %s\n", state_name[s]);
		failures++;
	}

	if (app_fsm_next((enum app_state)99, APP_EVENT_FIX) != APP_STATE_FAULT) {
		printf("FAIL an invalid state must fall to FAULT\n");
		failures++;
	}

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all %d transitions and 2 extra checks passed\n", STATE_COUNT * EVENT_COUNT);
	return 0;
}
