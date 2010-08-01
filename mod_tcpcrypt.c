#define CORE_PRIVATE
#include <assert.h>
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_connection.h" // pre_connection hook
#include "http_request.h" // fixups hook
#include "http_log.h"
#include "ap_config.h"
#include "lib/tcpcrypt.h"
#include "tcpcrypt/tcpcrypt.h"
#include "tcpcrypt/tcpcrypt_ctl.h"

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
    char *tc_enable, *tc_sessid, *s;
    unsigned int len;
    struct fake_apr_socket_t *sock;

    /* TODO(sqs): find a cleaner way of getting this? */
    sock = (struct fake_apr_socket_t *)csd;
    
    len = sizeof(buf);
    if (tcpcrypt_getsockopt(sock->socketdes, IPPROTO_TCP, TCP_CRYPT_ENABLE,
                   buf, &len) == -1) {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
                      "getsockopt error for TCP_CRYPT_ENABLE: %s (%d)",
                      strerror(errno), errno);
        return DECLINED;
    }

    tc_enable = (buf[0] ? "1" : "0");

    len = sizeof(buf);
    if (tcpcrypt_getsockopt(sock->socketdes, IPPROTO_TCP, TCP_CRYPT_SESSID,
                   buf, &len) == -1) {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
                      "tcpcrypt_getsockopt error for TCP_CRYPT_SESSID: %s (%d)",
                      strerror(errno), errno);
        return DECLINED;
    }

    tc_sessid = apr_palloc(c->pool, len*2 + 1);
    assert(tc_sessid);
    s = tc_sessid;
    while (len--) {
        sprintf(s, "%.2X", *b++);
        s += 2;
    }

    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c, 
                  "mod_tcpcrypt: set_tcpcrypt_env " \
                  "socket={des=%d, type=%d, protocol=%d} " \
                  "TCP_CRYPT_ENABLE=%s, TCP_CRYPT_SESSID=%s",
                  sock->socketdes, sock->type, sock->protocol,
                  tc_enable, tc_sessid);
    
    /* TODO(sqs): addn doesnt copy, so the val must be around until the request occurs -- this should be ok, but check on it */
    apr_table_addn(c->notes, "TCP_CRYPT_ENABLE", tc_enable);
    apr_table_addn(c->notes, "TCP_CRYPT_SESSID", tc_sessid);

    return DECLINED;
}

static int set_tcpcrypt_env(request_rec *r)
{
    apr_table_setn(r->subprocess_env, "TCP_CRYPT_ENABLE", 
                   apr_table_get(r->connection->notes, "TCP_CRYPT_ENABLE"));
    apr_table_set(r->subprocess_env, "TCP_CRYPT_SESSID", 
                  apr_table_get(r->connection->notes, "TCP_CRYPT_SESSID"));
    
    return OK;
}

static void tcpcrypt_register_hooks(apr_pool_t *p)
{
    ap_hook_pre_connection(get_tcpcrypt_sockopts, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_fixups(set_tcpcrypt_env, NULL, NULL, APR_HOOK_FIRST);
}


