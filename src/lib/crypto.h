#include <openssl/ssl.h>
#include <openssl/sha.h>
#include "config.h"

#define dexpd_read BIO_gets
#define dexpd_write BIO_puts
#define dexpd_flush BIO_flush
#define dexpd_iostream BIO

//SSL specific variables:
BIO *bio_err;
static char *pass;

//SSL specific functions:
int berr_exit (char *string);
int err_exit(char *string);

SSL_CTX *initialize_ctx(char *keyfile, char *password);
void destroy_ctx(SSL_CTX *ctx);
void check_cert(SSL *ssl, char *host);
void load_dh_params(SSL_CTX *ctx,char *file);
void generate_eph_rsa_key(SSL_CTX *ctx);
static int password_cb(char *buf,int num, int rwflag,void *userdata);


