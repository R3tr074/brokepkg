#!/usr/bin/env bash

cd $(dirname "$0")

CONFIG_PATH="../include/config.h"

RED="\033[38;5;198m"
PURPLE="\033[38;5;57m"
NO_COLOR="\033[0m"

f=3 b=4
for j in f b; do
  for i in {0..7}; do
    printf -v $j$i %b "\e[${!j}${i}m"
  done
done
bld=$'\e[1m'
rst=$'\e[0m'
inv=$'\e[7m'

BANNER=$(cat <<EOF
$bld$f1▬▬▬▬▬ $f2▬▬▬▬▬ $f3▬▬▬▬▬ $f4▬▬▬▬▬ $f5▬▬▬▬▬ $f6▬▬▬▬▬
${RED}\
,d8  b.d88b,
8888  888888
'Y888  888Y'
  'Y88  Y'${PURPLE}   // Written by R3tr0${RED}
    'Y'
$bld$f1▬▬▬▬▬ $f2▬▬▬▬▬ $f3▬▬▬▬▬ $f4▬▬▬▬▬ $f5▬▬▬▬▬ $f6▬▬▬▬▬
${NO_COLOR}
EOF
)

echo -e "$BANNER"



read -ep "Magic value(ex: br0k3): " magic_value
[[ ! ${magic_value} ]] && magic_value="br0k3"
[[ ${#magic_value} -lt 5 ]] && echo "Type at least 5 chars" >&2 && exit 2

read -ep "Magic number(ex: 1111): " magic_number
[[ ! ${magic_number} ]] && magic_number="1111"
[[ -n ${magic_number//[0-9]/} ]] && echo "Type a valid number" >&2 && exit 1
[[ ${#magic_number} -lt 4 ]] && echo "Type at least 4 numbers" >&2 && exit 2

read -ep "Magic hide(ex: br0k3_n0w_h1dd3n): " magic_hide
[[ ! ${magic_hide} ]] && magic_hide="br0k3_n0w_h1dd3n"
[[ ${#magic_hide} -lt 5 ]] && echo "Type at least 5 chars" >&2 && exit 2

read -ep "Active debug?(y/n): " debug
[[ ! ${debug} ]] && debug="n"

config_header=""
config_header+="#define MAGIC_HIDE \"${magic_hide}\"\n"
config_header+="#define MAGIC_VALUE \"${magic_value}\"\n"
config_header+="#define MAGIC_NUMBER ${magic_number}\n"
[[ $debug =~ ^[Yy]$ ]] && config_header+="#define DEBUG\n"

echo -ne "${config_header}" > "$CONFIG_PATH"

echo -e "$bld$f1▬▬▬▬▬ $f2▬▬▬▬▬ $f3▬▬▬▬▬ $f4▬▬▬▬▬ $f5▬▬▬▬▬ $f6▬▬▬▬▬${NO_COLOR}"
echo -e "config.h content:"
cat "$CONFIG_PATH"
echo -e "$bld$f1▬▬▬▬▬ $f2▬▬▬▬▬ $f3▬▬▬▬▬ $f4▬▬▬▬▬ $f5▬▬▬▬▬ $f6▬▬▬▬▬${NO_COLOR}"

echo "$(tput bold; tput setaf 2)[+] Configurate with success$(tput sgr0)"
