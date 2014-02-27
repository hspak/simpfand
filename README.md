# Simpfand
### Simple ThinkPad fan control daemon.

## Prequesite
The option "fan\_control=1" needs to be enabled for the module: thinkpad\_acpi

Example:

	# echo options thinkpad_acpi fan_control=1 > /etc/modprobe.d/thinkpad_fan.conf

## Install
On Arch Linux: [AUR](https://aur.archlinux.org/packages.php?ID=63124)

Also installs a systemd service file and a restart on resume script.

Otherwise:

	make install

## Usage
Simpfand is not a true daemon. It needs a manager like systemd. A systemd
service is provided. The Makefile as is will not install any systemd scripts.

## Author
Hong Shick Pak  hongshick.pak@gmail.com

## License
MIT
