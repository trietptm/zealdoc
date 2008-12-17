#! /bin/bash
echo 1 > /proc/sys/net/ipv4/ip_forward
iptables --flush
iptables -t nat -A POSTROUTING -s 192.168.100.0/24 -j MASQUERADE
iptables -t nat -A PREROUTING -d 10.241.110.78 -s ! 192.168.100.100 -j DNAT --to 192.168.100.100
