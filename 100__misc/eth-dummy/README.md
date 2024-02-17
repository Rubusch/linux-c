# dummy ethernet driver
Shows the skeleton of an ethernet driver, conform the book Linux Device Driver Programming (refrences).  

## Build

Crosscompile the module(s) for the target platform, eg rpi 3b. On the target board execut the following.

## Usage

Before  
```
root@ctrl001:/home/pi# ip addr
    1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
        link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
        inet 127.0.0.1/8 scope host lo
           valid_lft forever preferred_lft forever
        inet6 ::1/128 scope host
           valid_lft forever preferred_lft forever
    2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
        link/ether b8:27:eb:b6:cb:51 brd ff:ff:ff:ff:ff:ff
        inet 10.1.10.203/8 brd 10.255.255.255 scope global eth0
           valid_lft forever preferred_lft forever
        inet6 fe80::ba27:ebff:feb6:cb51/64 scope link
           valid_lft forever preferred_lft forever
    3: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN group default qlen 1000
        link/ether b8:27:eb:e3:9e:04 brd ff:ff:ff:ff:ff:ff
```
Load the module, then  
```
# insmod ./eth-dummy.ko
# insmod ./eth-dummy-ins.ko

# ip addr
    1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
        link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
        inet 127.0.0.1/8 scope host lo
           valid_lft forever preferred_lft forever
        inet6 ::1/128 scope host
           valid_lft forever preferred_lft forever
    2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
        link/ether b8:27:eb:b6:cb:51 brd ff:ff:ff:ff:ff:ff
        inet 10.1.10.203/8 brd 10.255.255.255 scope global eth0
           valid_lft forever preferred_lft forever
        inet6 fe80::ba27:ebff:feb6:cb51/64 scope link
           valid_lft forever preferred_lft forever
    3: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN group default qlen 1000
        link/ether b8:27:eb:e3:9e:04 brd ff:ff:ff:ff:ff:ff
    4: eth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
        link/ether 00:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff

# ip l show eth1
    4: eth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
        link/ether 00:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff

# ip addr show eth1
    4: eth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
        link/ether 00:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff
```
Logs  
```
Feb 17 22:04:17 ctrl001 kernel: [ 1383.294724] eth_dummy: loading out-of-tree module taints kernel.

Feb 17 22:04:25 ctrl001 kernel: [ 1391.326727] eth_probe(): called
Feb 17 22:04:25 ctrl001 kernel: [ 1391.326854] eth_init(): called

Feb 17 22:15:26 ctrl001 kernel: [ 2051.677259] eth_remove(): called
```


## References
* Linux Device Driver Programming, J. Madieu, 2022
