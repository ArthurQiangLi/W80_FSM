# CodeGenerator

1. string.Template (built-in, no dependencies)

Comes with Python by default.

Very simple: placeholders like $states, $events.

Great if your FSM templates are mostly static C code with a few lists inserted.

```py

from string import Template

fsm = {
    "states": ["IDLE", "RUNNING", "ERROR"],
    "events": ["START", "STOP", "FAIL"],
}

c_template = Template("""
typedef enum {
    $states
} State;

typedef enum {
    $events
} Event;
""")

states = ",\n    ".join(f"STATE_{s}" for s in fsm["states"])
events = ",\n    ".join(f"EVENT_{e}" for e in fsm["events"])

c_code = c_template.substitute(states=states, events=events)
print(c_code)
```


Output:

```c

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

```