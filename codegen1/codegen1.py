import json
from string import Template
from datetime import datetime
import os
import shutil
############################################################################

# - "$state_file" will look for placeholder 'state_file'
# - "${state}_file" inserts the 'state' value plus literal '_file'
templates = ["fsm_xxx.c", 
             "fsm_xxx.h", 
             "fsm_xxx_action.c",
             "fsm_xxx_action.h",
             "fsm_xxx_event.c",
             "fsm_xxx_event.h"
             ]# list of template files

def check_json_syntax(filename):
    try:
        with open(filename, "r", encoding="utf-8") as f:
            json.load(f)  # Try parsing
        print("config.JSON syntax is valid.")
    except json.JSONDecodeError as e:
        print(f"config.JSON syntax error: {e}")
############################################################################

# Load configuration
try:
    with open("config.json", "r", encoding="utf-8") as f:
        config = json.load(f)  # Try parsing
    print("config.JSON syntax is valid.")
except json.JSONDecodeError as e:
    print(f"\r\n**********\r\n config.JSON syntax error. {e} \r\n**********\r\n")

############################################################################

# Prepare substitution data
generation_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
generation_date = datetime.now().strftime("%Y-%m-%d")
STATES = ",\n    ".join(f"{config['XXX']}_STATE_{s}" for s in config["STATES"])
events = ",\n    ".join(f"EVENT_{e}" for e in config["events"])

make_trans_table_for_states = "\n\n".join(
    f'static const fsme_trans_t _trans_{sss}[] =\n'
    '{\n'
    + "".join(
        f'    {{{config["xxx"]}_event_{eee},        {config["XXX"]}_STATE_{config["STATES"][0]}}},\n'
        for eee in config["events"]
    ) +
    '};'
    for sss in config["states"]
)

make_state_table = "\n".join(
    f'    {{{config["xxx"]}_{sss}_entry,        {config["xxx"]}_{sss}_action,            {config["xxx"]}_{sss}_exit,          ARRAY_AND_SIZE(_trans_{sss}),                "{SSS}"}},'
    for sss, SSS in zip(config["states"], config["STATES"])
)

make_action_func_list = "\n".join(
    f'void {config["xxx"]}_{sss}_entry(void* fsmp);\n'
    f'void {config["xxx"]}_{sss}_exit(void* fsmp);\n'
    f'void {config["xxx"]}_{sss}_action(void* fsmp);\n'  
    for sss in config["states"])

make_action_func_bodies = "\n\n".join(
'/*******************************************************************************\n'
f'  * @brief  {config["xxx"]} [{sss}] state actions \n'
'  * @param  *fsmp , to access the state machine\n'
'  * @retval none\n'
'  *****************************************************************************/\n'
f'void {config["xxx"]}_{sss}_entry(void *fsmp)\n'
'{\n'
'    fsme_action_printf("[%s/%s] entry.", FSM_NAME(fsmp), CURR_STATE_NAME(fsmp));\n'
'}\n\n'
f'void {config["xxx"]}_{sss}_exit(void *fsmp)\n'
'{\n'
'    fsme_action_printf("[%s/%s] exit.", FSM_NAME(fsmp), CURR_STATE_NAME(fsmp));\n'
'}\n\n'
f'void {config["xxx"]}_{sss}_action(void *fsmp)\n'
'{\n'
'    fsme_action_printf("[%s/%s] action.", FSM_NAME(fsmp), CURR_STATE_NAME(fsmp));\n'
'}\n'
for sss in config["states"]
)

make_event_func_list = "\n".join(
    f'int32_t {config["xxx"]}_event_{eee}(void* fsmp);'
    for eee in config["events"]
)

make_event_func_bodies = "\n\n".join(
    '/*******************************************************************************\n'
    f'* @brief  if {eee}\n'
    '* @param  void *fsmp, to access the state machine\n'
    '* @retval 1:condition is met\n'
    '*****************************************************************************/\n'
    f'int32_t {config["xxx"]}_event_{eee}(void* fsmp)\n'
    '{\n'
    '    if (0) {\n'
    f'        fsme_event_printf("%s, event [{eee}].", ((fsme_t*)fsmp)->name);\n'
    '        return 1;\n'
    '    }\n'
    '    else {\n'
    '        return 0;\n'
    '    }\n'
    '}'
    for eee in config["events"]
)

substitutions = {
    "STATES": STATES,
    "events": events,
    "time": generation_time,
    "date": generation_date,
    "xxx": config["xxx"], # ${xxx} is used for making 1. event func names; 2. action entry/exit/action func names
    "XXX": config["XXX"], # ${XXX} is used for making 1. states' names and name strings
    "STATE_0": config['STATE_0'],
    "state_0": config['state_0'],
    "trans_table": make_trans_table_for_states,
    "state_table": make_state_table,
    "action_func_list_for_states": make_action_func_list,
    "action_func_bodies": make_action_func_bodies,
    "event_func_list": make_event_func_list,
    "event_func_bodies": make_event_func_bodies,
    "version": config["codegen1_version"],
}

############################################################################
# Process each template
base_folder = "generated"
if os.path.exists(base_folder):# Delete 'generated' folder if it exists
    shutil.rmtree(base_folder)

for tpl in templates:
    with open(tpl, "r") as tpl_file:
        template = Template(tpl_file.read())

    code = template.substitute(substitutions)# generate code
  
    out_file = tpl.replace("xxx", config["xxx"])# Build output file name (replace xxx with actual name)
    out_file = os.path.join("generated", out_file) 
    os.makedirs(os.path.dirname(out_file), exist_ok=True)# Ensure the parent directory exists

    with open(out_file, "w") as f:# write file
        f.write(code)

    print(f">> {out_file}")
print(f"---- codegen1({config['codegen1_version']}) creates {len(templates)} files in total.")


############################################################################

