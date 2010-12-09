opt=$1
iptables $opt INPUT -p tcp --sport 80 -i lo -j NFQUEUE --queue-num 666
iptables $opt OUTPUT -p tcp --dport 80 -o lo -j NFQUEUE --queue-num 666
iptables $opt INPUT -p tcp --dport 80 -i lo -j NFQUEUE --queue-num 666
iptables $opt OUTPUT -p tcp --sport 80 -o lo -j NFQUEUE --queue-num 666

