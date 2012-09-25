Simple ThinkPad fan control daemon.

Prequesite:
	this option needs to be enabled for the module: thinkpad_acpi
		fan_control=1

Current settings:
	When temperature change is < 2 C
		temp > 77 C  ==> level 7
		temp > 60 C  ==> level 5
		temp > 50 C  ==> level 4
		else		 ==> level 1
	Otherwise
		temp <= 55 C ==> level 1
		temp <= 65 C ==> level 2
		temp <= 82 C ==> level 4
		else		 ==> level 7

Config file is on the way.
