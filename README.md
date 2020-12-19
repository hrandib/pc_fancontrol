# pc_fancontrol
Linux 'fancontrol' package replacement with extended functionality.
Original package have some disadvantages. Aims of this alternative:
- Provide persistent configuration across hardware updates, as hwmon could be reenumerated by the system at any moment.
- Provide moving average buffering and different control algorithms: multi-point, proportional-integral, two-point
- Bind multiple temperature sources to the multiple PWMs.
- Get temperature from arbitrary source, can be retrieved via shell command execution

Full features description could be found in [the sample configuration file](config/fancontrol.yaml)
