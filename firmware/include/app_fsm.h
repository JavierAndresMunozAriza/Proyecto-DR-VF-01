/**
 * @file app_fsm.h
 * @brief Application state machine: pure transition function
 *
 * Pure C, no hardware dependency, so every transition is testable on the
 * host. The application owns the current state and applies the events.
 *
 * Transitions (any other combination keeps the state):
 *
 *   SEARCHING  --FENCE_CREATED--> MONITORING
 *   MONITORING --FIX_TIMEOUT----> FIX_LOST
 *   FIX_LOST   --FIX------------> MONITORING
 *   SEARCHING, MONITORING, FIX_LOST --GNSS_FAILED--> FAULT
 *   any state except DEMO --DEMO_TOGGLE--> DEMO
 *   DEMO       --DEMO_TOGGLE----> SEARCHING
 *
 * DEMO plays a simulated route and ignores every GNSS event, so it works
 * even when the GNSS is unavailable (FAULT) or has no sky view.
 */

#ifndef APP_FSM_H
#define APP_FSM_H

#ifdef __cplusplus
extern "C" {
#endif

enum app_state {
	APP_STATE_SEARCHING,  /* no fence yet, waiting for an accurate fix */
	APP_STATE_MONITORING, /* fence ready, fixes arriving */
	APP_STATE_FIX_LOST,   /* fence ready, fixes stopped arriving */
	APP_STATE_FAULT,      /* GNSS not available */
	APP_STATE_DEMO,       /* simulated route, no GNSS needed */
};

enum app_event {
	APP_EVENT_FENCE_CREATED, /* the first accurate fix defined the fence */
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

#ifdef __cplusplus
}
#endif

#endif /* APP_FSM_H */
