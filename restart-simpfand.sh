#!/bin/sh

# if waking up, restart simpfand
[[ "post" == "$1" ]] && systemctl restart simpfand
