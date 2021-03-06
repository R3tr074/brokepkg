#!/usr/bin/env bash

RED="\033[38;5;196m"
ORANGE="\033[38;5;202m"
GREEN="\033[38;5;118m"
BOLD="\033[1m"
NO_COLOR="\033[0m"

# messages wrappers
err() {
    echo -e "${RED}${BOLD}[-] ${*}${NO_COLOR}" >&2
}
warn() {
    echo -e "${ORANGE}${BOLD}[!] Warning: ${*}${NO_COLOR}" >&2
}
msg() {
    echo -e "${GREEN}${BOLD}[+] ${*}${NO_COLOR}"
}

ctrl_c_handler() {
    echo -e "${RED}${BOLD}[-] Ctrl + c detect, aborting...${NO_COLOR}"
    exit 0
}

check_install_err() {
    if [[ "$1" != 0 ]]; then
        err "Some error in install dependencies with $2"
        exit 2
    fi
}

trap ctrl_c_handler INT

SUDO=
# If not are root
if [[ "$(id -u)" -ne 0 ]];then
  # Try find sudo path
  SUDO=$(command -v sudo 2>/dev/null)

  # If not found sudo and not are root
  [[ -z $SUDO ]] && { err "Error: please, run as root" ;exit 4; }

  while [[ $(sudo -n id -u 2>&1) != 0 ]]; do
    sudo -v -p "Password for $(whoami): " &>/dev/null
  done
fi

msg "Installing build-essential, libncurses, linux-headers and socat"

if [[ ! -z $(command -v apt-get) ]]; then
    msg "Detect apt package manager"

    $SUDO apt-get -y update
    $SUDO apt-get -y install build-essential libncurses-dev linux-headers-$(uname -r) socat

    check_install_err $? "apt"

elif [[ ! -z $(command -v apk) ]]; then
    msg "Detect apk package manager"

    $SUDO apk --update add gcc make g++ ncurses-dev linux-headers socat

    check_install_err $? "apk"
elif [[ ! -z $(command -v pacman) ]]; then
    msg "Detect pacman package manager"

    $SUDO pacman -Sy
    $SUDO pacman -S --noconfirm base-devel ncurses linux-headers socat
    check_install_err $? "pacman"
elif [[ ! -z $(command -v yum) ]]; then
    msg "Detect yum package manager"

    $SUDO yum -y install gcc gcc-c++ make ncurses-devel kernel-devel kernel-headers socat
    check_install_err $? "yum"
elif [[ ! -z $(command -v dnf) ]]; then
    msg "Detect dnf package manager"

    $SUDO dnf -y install ncurses-devel make automake gcc gcc-c++ kernel-devel kernel-headers socat
    check_install_err $? "dnf"
else
    warn "Not found package manager, you need install manually the \"build-essential, libncurses-dev, socat and linux-headers\""
    exit 1
fi

msg "All dependencies installed with success"
