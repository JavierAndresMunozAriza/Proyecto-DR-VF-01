/*
 * Host unit tests for app_fsm_next(): every state/event combination.
 * Exits non-zero if any check fails.
 */

#include <stdio.h>
#include <string.h>

#include "app_fsm.h"

#define STATE_COUNT 6
#define EVENT_COUNT 7

static int failures;

static const char *const state_name[STATE_COUNT] = {
	"NO_FENCE", "WAIT_FIX", "MONITORING", "FIX_LOST", "FAULT", "DEMO",
};
static const char *const event_name[EVENT_COUNT] = {
	"FENCE_CREATED", "FENCE_LOADED", "FENCE_UPDATED", "FIX", "FIX_TIMEOUT",
	"GNSS_FAILED", "DEMO_TOGGLE",
};

#define NF APP_STATE_NO_FENCE
#define WF APP_STATE_WAIT_FIX
#define MO APP_STATE_MONITORING
#define FL APP_STATE_FIX_LOST
#define FA APP_STATE_FAULT
#define DE APP_STATE_DEMO

/*
 * Expected next state for every (state, event) pair, in enum order:
 * CREATED, LOADED, UPDATED, FIX, FIX_TIMEOUT, GNSS_FAILED, DEMO_TOGGLE.
 */
static const enum app_state expected[STATE_COUNT][EVENT_COUNT] = {
	/* NO_FENCE   */ { MO, WF, WF, NF, NF, FA, DE },
	/* WAIT_FIX   */ { WF, WF, WF, MO, WF, FA, DE },
	/* MONITORING */ { MO, MO, MO, MO, FL, FA, DE },
	/* FIX_LOST   */ { FL, FL, FL, MO, FL, FA, DE },
	/* FAULT      */ { FA, FA, FA, FA, FA, FA, DE },
	/* DEMO       */ { DE, DE, DE, DE, DE, DE, NF },
};

static void expect_state(const char *what, enum app_state got, enum app_state want)
{
	if (got != want) {
		printf("FAIL %s: got %s want %s\n", what, state_name[got], state_name[want]);
		failures++;
	}
}

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

	/* Previous behavior still holds: the fence made from the first fix. */
	enum app_state s = APP_STATE_NO_FENCE;

	s = app_fsm_next(s, APP_EVENT_FENCE_CREATED);
	s = app_fsm_next(s, APP_EVENT_FIX_TIMEOUT);
	s = app_fsm_next(s, APP_EVENT_FIX);
	expect_state("no fence -> created -> lost -> recovered", s, APP_STATE_MONITORING);

	/* The demo works even when the GNSS is unavailable, and leaves to NO_FENCE. */
	s = app_fsm_next(APP_STATE_FAULT, APP_EVENT_DEMO_TOGGLE);
	s = app_fsm_next(s, APP_EVENT_DEMO_TOGGLE);
	expect_state("fault -> demo -> toggle", s, APP_STATE_NO_FENCE);

	/* A stored fence: boot, load it, wait for a fix, monitor. */
	s = app_fsm_next(APP_STATE_NO_FENCE, APP_EVENT_FENCE_LOADED);
	expect_state("boot with a stored fence waits for a fix", s, APP_STATE_WAIT_FIX);
	s = app_fsm_next(s, APP_EVENT_FIX);
	expect_state("the first fix starts the monitoring", s, APP_STATE_MONITORING);

	/* A fence from the cloud replaces the current one without changing the state. */
	s = app_fsm_next(APP_STATE_MONITORING, APP_EVENT_FENCE_UPDATED);
	expect_state("update while monitoring", s, APP_STATE_MONITORING);
	s = app_fsm_next(APP_STATE_FIX_LOST, APP_EVENT_FENCE_UPDATED);
	expect_state("update while the fix is lost", s, APP_STATE_FIX_LOST);
	s = app_fsm_next(APP_STATE_DEMO, APP_EVENT_FENCE_UPDATED);
	expect_state("update during the demo is kept for later", s, APP_STATE_DEMO);

	for (int i = 0; i < STATE_COUNT; i++) {
		if (strcmp(app_fsm_state_name((enum app_state)i), state_name[i]) != 0) {
			printf("FAIL state %d is named %s, want %s\n", i,
			       app_fsm_state_name((enum app_state)i), state_name[i]);
			failures++;
		}
	}
	if (strcmp(app_fsm_state_name((enum app_state)99), "UNKNOWN") != 0) {
		printf("FAIL an invalid state must be named UNKNOWN\n");
		failures++;
	}

	expect_state("an invalid state must fall to FAULT",
		     app_fsm_next((enum app_state)99, APP_EVENT_FIX), APP_STATE_FAULT);

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all %d transitions and the name checks passed\n", STATE_COUNT * EVENT_COUNT);
	return 0;
}
