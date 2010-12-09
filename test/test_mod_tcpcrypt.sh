#!/bin/bash
testname=$1
url=$2
curlout=/tmp/curlout-$$


case $testname in
    on)
        echo -n Testing mod_tcpcrypt with server tcpcryptd running...
        curl --tcp-nodelay --limit-rate 30 $url > $curlout 2>/dev/null &
        sleep 0.5
        sessid=$(tcnetstat | tail -n 1 | rev | cut -d ' ' -f 1 | rev)
        wait
        grep "${sessid/[^\w\d]/}" $curlout && \
            echo "PASS" && exit 0
        echo "FAIL"
        echo Got
        cat $curlout
        exit 1
        ;;
    off)
        echo -n Testing mod_tcpcrypt with server tcpcryptd off...
        curl $url > $curlout 2>/dev/null
        grep "TCP_CRYPT_SESSID = <none>" $curlout && \
            echo "PASS" && exit 0
        echo "FAIL"
        echo got
        cat $curlout
        exit 1
        ;;
esac
# TODO: test with server tcpcryptd on, client tcpcryptd off
