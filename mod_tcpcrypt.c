#define CORE_PRIVATE
#include <assert.h>
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_connection.h" // pre_connection hook
#include "http_request.h" // fixups hook
#include "http_log.h"
#include "ap_config.h"
#include <tcpcrypt/tcpcrypt.h>


/* WARNING: Tcpcrypt doesn't really work with IPv6 (yet). If Apache is
   listening on an IPv6 address (check `netstat` for "tcp6"), mod_tcpcrypt will
   probably work for IPv4 clients (who will appear to Apache as having
   IPv4-compatible IPv6 addresses). If you're having problems and don't
   actually care about IPv6, then change all of your Apache config Listen
   directives to specify an IPv4 address (e.g., `Listen 80 -> `Listen
   0.0.0.0:80`). */

/* TODO(sqs): find a cleaner way of getting access to socketdes */
struct fake_apr_socket_t {
    apr_pool_t *pool;
    int socketdes;
    int type;
    int protocol;
};

static void tcpcrypt_register_hooks(apr_pool_t *p);
static int get_tcpcrypt_sockopts(conn_rec *c, void *csd);
static int set_tcpcrypt_env(request_rec *r);

module AP_MODULE_DECLARE_DATA tcpcrypt_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    tcpcrypt_register_hooks  /* register hooks                      */
};

static int get_tcpcrypt_sockopts(conn_rec *c, void *csd) {
    unsigned char buf[1024];
    unsigned char *b = (unsigned char *)buf;
    char *tc_sessid, *s;
    unsigned int len;
    struct fake_apr_socket_t *sock;

    sock = (struct fake_apr_socket_t *)csd;
    
    len = sizeof(buf);

    if (tcpcrypt_getsockopt(sock->socketdes, IPPROTO_TCP, TCP_CRYPT_SESSID,
                            buf, &len) == -1) {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
                      "tcpcrypt_getsockopt error for TCP_CRYPT_SESSID: "
                      "%s (%d) [fd=%d]",
                      strerror(errno), errno, sock->socketdes);
        return DECLINED;
    }

    if (len) {
        tc_sessid = apr_palloc(c->pool, len*2 + 1);
        tc_sessid[0] = '\0';
        assert(tc_sessid);
        s = tc_sessid;
        while (len--) {
            sprintf(s, "%.2X", *b++);
            s += 2;
        }

        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c, 
                      "mod_tcpcrypt: got TCP_CRYPT_SESSID=%s "
                      "[fd=%d, type=%d, protocol=%d] ",
                      tc_sessid[0] ? tc_sessid : "<none>",
                      sock->socketdes, sock->type, sock->protocol);
        apr_table_addn(c->notes, "TCP_CRYPT_SESSID", tc_sessid);
    }

    return DECLINED;
}

static int set_tcpcrypt_env(request_rec *r)
{
    const char *tc_sessid = apr_table_get(r->connection->notes, "TCP_CRYPT_SESSID");

    if (tc_sessid && tc_sessid[0]) {
        apr_table_set(r->subprocess_env, "TCP_CRYPT_SESSID", tc_sessid);
    }
    
    return OK;
}

static void tcpcrypt_register_hooks(apr_pool_t *p)
{
    ap_hook_pre_connection(get_tcpcrypt_sockopts, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_fixups(set_tcpcrypt_env, NULL, NULL, APR_HOOK_FIRST);
}


