# Simpfand
### Simple ThinkPad fan control daemon.

## Prequesite
The option "fan\_control=1" needs to be enabled for the module: thinkpad\_acpi 

##Install
On Arch Linux:
https://aur.archlinux.org/packages.php?ID=63124
Also installs a systemd service file.

Otherwise:

	make install

##Usage
Simpfand is not a true daemon. It needs a manager like systemd. You must write your own daemon/service file.

##Author
Hong Shick Pak  hongshick.pak@gmail.com 

