/**
 * @file app_fsm.h
 * @brief Application state machine: pure transition function
 *
 * Pure C, no hardware dependency, so every transition is testable on the
 * host. The application owns the current state and applies the events.
 *
 * Transitions (any other combination keeps the state):
 *
 *   NO_FENCE   --FENCE_CREATED--> MONITORING  (the first fix made the fence)
 *   NO_FENCE   --FENCE_LOADED---> WAIT_FIX    (a stored fence was loaded)
 *   NO_FENCE   --FENCE_UPDATED--> WAIT_FIX    (a fence arrived from the cloud)
 *   WAIT_FIX   --FIX------------> MONITORING
 *   MONITORING --FIX_TIMEOUT----> FIX_LOST
 *   FIX_LOST   --FIX------------> MONITORING
 *   NO_FENCE, WAIT_FIX, MONITORING, FIX_LOST --GNSS_FAILED--> FAULT
 *   any state except DEMO --DEMO_TOGGLE--> DEMO
 *   DEMO       --DEMO_TOGGLE----> NO_FENCE
 *
 * A new fence replaces the current one in place: FENCE_UPDATED never changes
 * the state once a fence exists. The application releases the demo fence when
 * it leaves the DEMO, so it returns to NO_FENCE and reloads the stored fence,
 * if there is one, by dispatching FENCE_LOADED.
 *
 * DEMO plays a simulated route and ignores every GNSS and fence event, so it
 * works even when the GNSS is unavailable (FAULT) or has no sky view.
 */

#ifndef APP_FSM_H
#define APP_FSM_H

#ifdef __cplusplus
extern "C" {
#endif

enum app_state {
	APP_STATE_NO_FENCE,   /* no fence yet: waiting for one, or for the first fix */
	APP_STATE_WAIT_FIX,   /* fence ready, waiting for the first fix */
	APP_STATE_MONITORING, /* fence ready, fixes arriving */
	APP_STATE_FIX_LOST,   /* fence ready, fixes stopped arriving */
	APP_STATE_FAULT,      /* GNSS not available */
	APP_STATE_DEMO,       /* simulated route, no GNSS needed */
};

enum app_event {
	APP_EVENT_FENCE_CREATED, /* the first accurate fix defined the fence */
	APP_EVENT_FENCE_LOADED,  /* a fence stored in flash was loaded at boot */
	APP_EVENT_FENCE_UPDATED, /* a new fence arrived and was accepted */
	APP_EVENT_FIX,           /* a fix was checked against the fence */
	APP_EVENT_FIX_TIMEOUT,   /* no fix for too long */
	APP_EVENT_GNSS_FAILED,   /* the GNSS could not start or stopped working */
	APP_EVENT_DEMO_TOGGLE,   /* the user asked to enter or leave the demo */
};

/**
 * @brief State reached from @p state when @p event happens
 * @return The next state; @p state itself if the event does not apply
 */
enum app_state app_fsm_next(enum app_state state, enum app_event event);

/**
 * @brief Name of a state, for logs and reports
 * @return The state name, such as "MONITORING"; "UNKNOWN" if @p state is not valid
 */
const char *app_fsm_state_name(enum app_state state);

#ifdef __cplusplus
}
#endif

#endif /* APP_FSM_H */
