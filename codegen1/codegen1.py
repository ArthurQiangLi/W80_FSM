import json
from string import Template
from datetime import datetime
import os

# Load configuration
with open("config.json", "r") as f:
    config = json.load(f)

fsm = config["fsm"]
output_file_c = config["output_file"]
output_file_h = os.path.splitext(output_file_c)[0] + ".h"

# Load templates
with open("template.c", "r") as tpl_file:
    c_template = Template(tpl_file.read())

with open("template.h", "r") as tpl_file:
    h_template = Template(tpl_file.read())

# Prepare data
states = ",\n    ".join(f"STATE_{s}" for s in fsm["states"])
events = ",\n    ".join(f"EVENT_{e}" for e in fsm["events"])
generation_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Substitute into templates
c_code = c_template.substitute(
    states=states,
    events=events,
    time=generation_time
)

h_code = h_template.substitute(
    states=states,
    events=events,
    time=generation_time
)

# Write to .c file
with open(output_file_c, "w") as f:
    f.write(c_code)

# Write to .h file
with open(output_file_h, "w") as f:
    f.write(h_code)

print(f"Code generated in {output_file_c} and {output_file_h}")
