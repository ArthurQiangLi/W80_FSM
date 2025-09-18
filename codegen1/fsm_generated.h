/* 
 * Auto-generated FSM header
 * Generation time: 2025-09-18 08:42:18
 */

#ifndef FSM_GENERATED_H
#define FSM_GENERATED_H

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
} State;

typedef enum {
    EVENT_START,
    EVENT_STOP,
    EVENT_FAIL
} Event;

#endif /* FSM_GENERATED_H */
