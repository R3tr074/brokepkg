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
[[ ${#magic_value} -lt 5 ]] && echo "Type at least 5 chars" >&2 && exit 2

read -ep "Magic number(ex: 1111): " magic_number
[[ -n ${magic_number//[0-9]/} ]] && echo "Type a valid number" >&2 && exit 1
[[ ${#magic_number} -lt 4 ]] && echo "Type at least 4 numbers" >&2 && exit 2

read -ep "Magic prefix(ex: br0k3): " magic_prefix
[[ ${#magic_prefix} -lt 5 ]] && echo "Type at least 5 chars" >&2 && exit 2

read -ep "Active debug?(y/n): " debug
[[ $debug =~ ^[Yy]$ ]] && debug_code="#define DEBUG"

cat >"$CONFIG_PATH" <<EOF
${debug_code}
#define PREFIX "${magic_prefix}"
#define MAGIC_VALUE "${magic_value}"
#define MAGIC_NUMBER ${magic_number}
EOF

echo -e "$bld$f1▬▬▬▬▬ $f2▬▬▬▬▬ $f3▬▬▬▬▬ $f4▬▬▬▬▬ $f5▬▬▬▬▬ $f6▬▬▬▬▬${NO_COLOR}"
echo -e "config.h content:"
cat "$CONFIG_PATH"
echo -e "$bld$f1▬▬▬▬▬ $f2▬▬▬▬▬ $f3▬▬▬▬▬ $f4▬▬▬▬▬ $f5▬▬▬▬▬ $f6▬▬▬▬▬${NO_COLOR}"

echo "$(tput bold; tput setaf 2)[+] Configurate with success$(tput sgr0)"
