# About 'codegen1'

## Feature List

1. Read FSM data from config.json.
1. Generate 6 files.

## How to use it?

```sh
1-set parameters in config.json
2-run:
python codegen1.py
```

## How it works?

### Step1: formating the 'states'

This about how 'substitution' works in Python.

```py
states = ",\n    ".join(f"STATE_{s}" for s in fsm["states"])
-->
states = "STATE_IDLE,\n    STATE_RUNNING,\n    STATE_ERROR"
```
Explaination: 

1. fsm["states"] is a list (["IDLE","RUNNING","ERROR"]).
2. f"STATE_{s}" for s in ... produces ["STATE_IDLE", "STATE_RUNNING", "STATE_ERROR"].
3. ",\n ".join(...) concatenates them into a single string with , then `states = "STATE_IDLE,\n    STATE_RUNNING,\n    STATE_ERROR"`

### Step2: subsitute call

The python code:
```py
c_code = c_template.substitute(
    states=states, # replace '$states' with 'states' (string in Step1)
    events=events,
    time=generation_time
)
## equals to:
substs = {"states": states, "events": events, "time": generation_time}
c_code = c_template.substitute(substs)
```
1. The `c_template` here is the template file
2. The `substitue()` function applies to the template file, it scans the template string, finds each `$states` occurrence and replaces it with the `states` string you already have in Step1.

### Step3: compare template to output
Template.c
```c
/* 
 * Auto-generated FSM code
 * Generation time: $time
 */

typedef enum {
    $states  // is replaced by 'states' (the string in Step1)
} State;
```

Output generated.c:
```c
/* 
 * Auto-generated FSM code
 * Generation time: 2025-09-18 08:42:18
 */

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
} State;

```

- "$state_file" will look for placeholder 'state_file'
- "${state}_file" inserts the 'state' value plus literal '_file'


# ~End~