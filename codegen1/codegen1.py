import json
from string import Template
from datetime import datetime

# Load configuration
with open("config.json", "r") as f:
    config = json.load(f)

fsm = config["fsm"]
output_file = config["output_file"]

# Load template from file
with open("template.c", "r") as tpl_file:
    c_template = Template(tpl_file.read())

# Prepare data
states = ",\n    ".join(f"STATE_{s}" for s in fsm["states"])
events = ",\n    ".join(f"EVENT_{e}" for e in fsm["events"])
generation_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Substitute into template
c_code = c_template.substitute(
    states=states,
    events=events,
    time=generation_time
)

# Write to .c file
with open(output_file, "w") as f:
    f.write(c_code)

print(f"Code generated in {output_file}")
