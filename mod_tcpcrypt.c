#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "ap_config.h"

static int set_tcpcrypt_env(request_rec *r)
{
    char *tcp_crypt = NULL, *tcp_crypt_sessid = NULL;

    apr_table_setn(r->subprocess_env, "TCP_CRYPT", "1");
    apr_table_set(r->subprocess_env, "TCP_CRYPT_SESSID", "abcd");
    
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, 
                  "mod_tcpcrypt: set_tcpcrypt_env " \
                  "TCP_CRYPT=%s, TCP_CRYPT_SESSID=%s",
                  tcp_crypt, tcp_crypt_sessid);
    
    return OK;
}

static void tcpcrypt_register_hooks(apr_pool_t *p)
{
    ap_hook_fixups(set_tcpcrypt_env, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA tcpcrypt_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    tcpcrypt_register_hooks  /* register hooks                      */
};

