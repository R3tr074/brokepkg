#!/usr/bin/env bash

err_log() {
  echo >&2 "$(tput bold; tput setaf 1)[-] ${*}$(tput sgr0)"
}
error=$(insmod $(find . -name '*.ko') 2>&1)

if [ $? -ne 0 ];then
  err_log ${error}
  err_log "run \"dmesg -Hw\" to see logs"
  exit 3 
fi

echo "$(tput bold; tput setaf 2)[+] Brokepkg installed with success$(tput sgr0)"

