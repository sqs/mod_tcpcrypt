mod_tcpcrypt for Apache
=======================

This module exposes the [tcpcrypt]:(http://tcpcrypt.org) session ID of the
current Apache HTTP connection to scripts and Web apps running on Apache.

The `TCP_CRYPT_SESSID` environment variable contains the tcpcrypt session ID
if a tcpcrypt connection was established; otherwise, the env var is unset. For
example, a PHP script can access the tcpcrypt session ID as
`$_SERVER['TCP_CRYPT_SESSID']`.

If both ends of a tcpcrypt connection see the same session ID, then an attacker
cannot eavesdrop on or undetectably tamper with traffic--i.e., there has not
been a man-in-the-middle attack. Also, session IDs are (with overwhelming
probability) unique over all time, even if one end of a connection is
malicious. Thus tcpcrypt session IDs can be used to mutually authenticate a
connection and all of the data passed over it. See section 4 of the [tcpcrypt
paper](http://tcpcrypt.org/tcpcrypt.pdf) for more info on using tcpcrypt
session IDs for authentication.

The tcpcrypt team is currently working on more examples of using tcpcrypt for
HTTP authentication. [Subscribe to
tcpcrypt-dev](https://mailman.stanford.edu/mailman/listinfo/tcpcrypt-dev) to
follow our progress or contribute.


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

Right now, `mod_tcpcrypt` adds the `TCP_CRYPT_SESSID` env var to **every**
request. This will change soon, but for now, no configuration is needed other
than installing the module.

**IPv6 note:** Tcpcrypt doesn't really work with IPv6 (yet). If Apache is
listening on an IPv6 address (check `netstat` for "tcp6"), mod_tcpcrypt will
probably work for IPv4 clients (who will appear to Apache as having
IPv4-compatible IPv6 addresses). If you're having problems and don't actually
care about IPv6, then change all of your Apache config Listen directives to
specify an IPv4 address (e.g., `Listen 80 -> `Listen 0.0.0.0:80`).


Accessing TCP_CRYPT_SESSID in scripts/Web apps
==============================================

The sockopt is just exposed as a normal environment variable. Here are a few
language-specific examples.

* PHP: `$_SERVER['TCP_CRYPT_SESSID']`
* Python: `import os` and then `os.getenv('TCP_CRYPT_SESSID')` 
* Ruby: `ENV['TCP_CRYPT_SESSID']`


Warnings
========

This module has only been tested on Linux (Debian Squeeze amd64). Currently not
recommended for production use.


More info
=========

For general info about tcpcrypt, see [tcpcrypt.org](http://tcpcrypt.org). The
tcpcrypt code is at
[http://github.com/sorbo/tcpcrypt](http://github.com/sorbo/tcpcrypt).

For more info about how to use tcpcrypt session IDs to authenticate
connections, see section 4 of the [tcpcrypt
paper](http://tcpcrypt.org/tcpcrypt.pdf).