#!/bin/sh

echo Content-Type: text/plain
echo
echo REMOTE_ADDR = ${REMOTE_ADDR:='unknown'}
echo TCP_CRYPT_ENABLE = ${TCP_CRYPT_ENABLE:='unknown'}
echo TCP_CRYPT_SESSID = ${TCP_CRYPT_SESSID:='unknown'}
