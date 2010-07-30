APXS=apxs2
APACHECTL=apache2ctl
#DEFS=-Dmy_define=my_value
#INCLUDES=-Imy/include/dir
#LIBS=-Lmy/lib/dir -lmylib

install:
	$(APXS) -cia mod_tcpcrypt.c

#   cleanup
clean:
	-rm -f mod_tcpcrypt.o mod_tcpcrypt.lo mod_tcpcrypt.slo mod_tcpcrypt.la 

#   simple test
configtest:
	-cp test/test-tcpcrypt-site /etc/apache2/sites-available/
	-sudo a2ensite test-tcpcrypt-site

test: configtest reload
	lynx -mime_header http://localhost:7777/tcpcrypt

#   install and activate shared object by reloading Apache to
#   force a reload of the shared object file
reload: install restart

#   the general Apache start/restart/stop
#   procedures
start:
	sudo $(APACHECTL) start
restart:
	sudo $(APACHECTL) restart
stop:
	sudo $(APACHECTL) stop

