mod_tcpcrypt.la: mod_tcpcrypt.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_tcpcrypt.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_tcpcrypt.la
