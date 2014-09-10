# Simpfand
### Simple ThinkPad fan control daemon.

## Prequesite
The option "fan\_control=1" needs to be enabled for the module: thinkpad\_acpi

Example:

	# echo options thinkpad_acpi fan_control=1 > /etc/modprobe.d/thinkpad_fan.conf

## Install
On Arch Linux: [AUR](https://aur.archlinux.org/packages/simpfand-git/)

Also installs a systemd service file and a restart on resume script.

Otherwise:

	make install

## Usage
Simpfand is not a true daemon. It needs a manager like systemd. A systemd
service is provided. The Makefile as is will not install any systemd scripts.

__Note__: As of version 4.1+, fan levels of 0 are allowed. As of version 4.2,
[temperature hysteresis](http://en.wikipedia.org/wiki/Hysteresis) is implemented
thanks to [MarcusMoeller](https://github.com/MarcusMoeller). Without his testing
and suggestions, this would not exist. The default config has been setup as
such now:

    default diagram:
             55 --> BASE  --> 65 --> INC_L --> 75 --> INC_H --> 85 --> INC_M
    BASE <-- 55 <-- DEC_L <-- 65 <-- DEC_H <-- 75 <-- DEC_M <--
    
    translates into:
    
          55 --> 1 --> 65 --> 2 --> 75 --> 4 --> 85 --> 6
    1 <-- 55 <-- 2 <-- 65 <-- 4 <-- 75 <-- 6 <--

## Author
Hong Shick Pak <hong@hspak.com>

## License
MIT
