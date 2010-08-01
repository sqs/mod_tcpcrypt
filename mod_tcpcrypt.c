#define CORE_PRIVATE
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
    unsigned char tcp_crypt_sessid[1024], tcp_crypt[1024];
    unsigned int len;
    struct fake_apr_socket_t *sock;

    sock = (struct fake_apr_socket_t *)csd;
    
    len = sizeof(tcp_crypt);
    if (tcpcrypt_getsockopt(sock->socketdes, IPPROTO_TCP, TCP_CRYPT_ENABLE,
                   tcp_crypt, &len) == -1) {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
                      "getsockopt error for TCP_CRYPT_ENABLE: %s (%d)",
                      strerror(errno), errno);
        return DECLINED;
    }

    len = sizeof(tcp_crypt_sessid);

    if (tcpcrypt_getsockopt(sock->socketdes, IPPROTO_TCP, TCP_CRYPT_SESSID,
                   tcp_crypt_sessid, &len) == -1) {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
                      "tcpcrypt_getsockopt error for TCP_CRYPT_SESSID: %s (%d)",
                      strerror(errno), errno);
        return DECLINED;
    }

    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c, 
                  "mod_tcpcrypt: set_tcpcrypt_env " \
                  "socket={des=%d, type=%d, protocol=%d} " \
                  "TCP_CRYPT=%s, TCP_CRYPT_SESSID=%s",
                  sock->socketdes, sock->type, sock->protocol,
                  tcp_crypt, tcp_crypt_sessid);

    return DECLINED;
}

static int set_tcpcrypt_env(request_rec *r)
{
    apr_table_setn(r->subprocess_env, "TCP_CRYPT", "1");
    apr_table_set(r->subprocess_env, "TCP_CRYPT_SESSID", "abcd");
    
    return OK;
}

static void tcpcrypt_register_hooks(apr_pool_t *p)
{
    ap_hook_pre_connection(get_tcpcrypt_sockopts, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_fixups(set_tcpcrypt_env, NULL, NULL, APR_HOOK_FIRST);
}


