mod_tcpcrypt for Apache
=======================

This module gives scripts access to the following
[tcpcrypt]:(http://tcpcrypt.org) sockopts as environment variables:

* `TCP_CRYPT_ENABLE` (whether tcpcrypt is enabled for this request)
* `TCP_CRYPT_SESSID` (a unique identifier for this connection that can be used
   to authenticate the connection)

For example, a PHP script can access the tcpcrypt session ID as
`$_SERVER['TCP_CRYPT_SESSID']`.


Installation
============

You'll need Apache 2.2 and apxs2, the Apache tool for installing modules. On
Ubuntu and Debian, `apt-get install apache2-dev` should suffice.

Set the Makefile's `TCPCRYPT` variable to the path containing the [tcpcrypt
code](http://github.com/sorbo/tcpcrypt). Build tcpcrypt's `libtcpcrypt.so`
(see instructions for tcpcrypt).

Now, from this `mod_tcpcrypt` directory, run `make install` as root to build
and install the module.


Configuration
-------------

Right now, `mod_tcpcrypt` adds the `TCP_CRYPT_ENABLE` and `TCP_CRYPT_SESSID` to
every single request. This will change soon, but for now, no configuration is
needed other than installing the module.

However, if your Apache is listening on an IPv6 address (which occurs if you
specify an IPv6 address in the Listen directive **OR** if you simply omit an
address in Listen on some configs), then `mod_tcpcrypt` won't work. Run
`netstat -a` and check for `tcp6` to see if Apache is listening on an IPv6
address. You can work around this issue by specifying an explicit IPv4 address
in your Listen directives. For example, use `Listen 0.0.0.0:80` instead of
`Listen 80`.


Accessing tcpcrypt sockopts in scripts/Web apps
===============================================

The sockopts just are exposed as normal environment variables. Here are a few
language-specific examples.

* PHP: `$_SERVER['TCP_CRYPT_ENABLE']` and `$_SERVER['TCP_CRYPT_SESSID']`
* Python: `import os` and then `os.getenv('TCP_CRYPT_ENABLE')` and 
  `os.getenv('TCP_CRYPT_SESSID')` 
* Ruby: `ENV['TCP_CRYPT_ENABLE']` and `ENV['TCP_CRYPT_SESSID']`


Warnings
========

This module has only been tested on Linux. Currently not recommended for
production use.


More info
=========

For general info about tcpcrypt, see [tcpcrypt.org](http://tcpcrypt.org). The
tcpcrypt code is at
[http://github.com/sorbo/tcpcrypt](http://github.com/sorbo/tcpcrypt).

For more info about how to use tcpcrypt session IDs to authenticate
connections, see the [tcpcrypt paper](http://tcpcrypt.org/tcpcrypt.pdf).