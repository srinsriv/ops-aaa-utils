/* libtac.h
 *
 * Copyright (C) 2010, Pawel Krawczyk <pawel.krawczyk@hush.com> and
 * Jeroen Nijhof <jeroen@jeroennijhof.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program - see the file COPYING.
 *
 * See `CHANGES' file for revision history.
 */

#ifndef _LIB_TAC_H
#define _LIB_TAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#ifdef __linux__
#include <sys/cdefs.h>
#else
#include "cdefs.h"
#endif
#include "tacplus.h"

#if defined(DEBUGTAC) && !defined(TACDEBUG)
#define TACDEBUG(x) syslog x;
#else
//#define TACDEBUG(x) syslog x;
#define TACDEBUG(x)
#endif

#define TACSYSLOG(x) syslog x;

#if defined(TACDEBUG_AT_RUNTIME)
#undef TACDEBUG
#undef TACSYSLOG
#define TACDEBUG(x) if (tac_debug_enable) (void)logmsg x;
#define TACSYSLOG(x) (void)logmsg x;
extern int logmsg __P((int, const char*, ...));
#endif

#define TACC_CONN_TIMEOUT 60
#define TACACS        "TACACS"
#define AUTH_MODE_ENV "AUTH_MODE"
#define PRIV_LVL_ENV "PRIV_LVL"
#define PRIV_RET_STR  "priv-lvl"
#define PRV_LVL_LENGTH 3

#define PRINT(...)            printf(__VA_ARGS__)
#define VLOG_INFORMATION(...) VLOG_INFO(__VA_ARGS__)
#define VLOG_DEBUG(...)       VLOG_DBG(__VA_ARGS__)
#define VLOG_ERROR(...)       VLOG_ERR(__VA_ARGS__)

#define LOG(quiet, severity, ...) \
    if (!quiet) { \
        PRINT(__VA_ARGS__); \
    } else { \
        if (severity == VLL_INFO) { \
            VLOG_INFO(__VA_ARGS__); \
        } else if (severity == VLL_DBG) { \
            VLOG_DBG(__VA_ARGS__); \
        } else { \
            VLOG_ERR(__VA_ARGS__); \
        } \
    }

/* u_int32_t support for sun */
#ifdef sun
typedef unsigned int u_int32_t;
#endif

struct tac_attrib {
	char *attr;
	u_char attr_len;
	struct tac_attrib *next;
};

struct areply {
	struct tac_attrib *attr;
	char *msg;
	int status :8;
	int flags :8;
	int seq_no :8;
};

#define EXIT_OK         0  /* Authorized */
#define EXIT_FAIL       1  /* Authorization was denied */
#define EXIT_ADDR_ERR   2  /* Error when resolving TACACS server address */
#define EXIT_CONN_ERR   3  /* Connection to TACACS server failed */
#define EXIT_SEND_ERR   4  /* Error when sending authorization request */
#define EXIT_ERR        5  /* local error */

#ifndef TAC_PLUS_MAXSERVERS
#define TAC_PLUS_MAXSERVERS 8
#endif

#ifndef TAC_PLUS_MAX_PACKET_SIZE
#define TAC_PLUS_MAX_PACKET_SIZE 128000 /* bytes */
#endif

#ifndef TAC_PLUS_MAX_ARGCOUNT
#define TAC_PLUS_MAX_ARGCOUNT 100 /* maximum number of arguments passed in packet */
#endif

#ifndef TAC_PLUS_PORT
#define	TAC_PLUS_PORT 49
#endif

#define TAC_PLUS_READ_TIMEOUT  180    /* seconds */
#define TAC_PLUS_WRITE_TIMEOUT 180    /* seconds */

/* Internal status codes
 *   all negative, tacplus status codes are >= 0
 */

#define LIBTAC_STATUS_ASSEMBLY_ERR  -1
#define LIBTAC_STATUS_PROTOCOL_ERR  -2
#define LIBTAC_STATUS_READ_TIMEOUT  -3
#define LIBTAC_STATUS_WRITE_TIMEOUT -4
#define LIBTAC_STATUS_WRITE_ERR     -5
#define LIBTAC_STATUS_SHORT_HDR     -6
#define LIBTAC_STATUS_SHORT_BODY    -7
#define LIBTAC_STATUS_CONN_TIMEOUT  -8
#define LIBTAC_STATUS_CONN_ERR      -9

/* Runtime flags */

/* version.c */
extern int tac_ver_major;
extern int tac_ver_minor;
extern int tac_ver_patch;

/* header.c */
extern int session_id;
extern int tac_encryption;
extern const char *tac_secret;
extern char tac_login[64];
extern int tac_priv_lvl;
extern int tac_authen_method;
extern int tac_authen_service;

extern int tac_debug_enable;
extern int tac_readtimeout_enable;

/* connect.c */
extern int tac_timeout;

int tac_connect(struct addrinfo **, char **, int);
int tac_connect_single(const struct addrinfo *, const char *, struct addrinfo *,
		int);
char *tac_ntop(const struct sockaddr *);

int tac_authen_send(int, const char *, const char *, const char *, const char *,
		u_char);
int tac_authen_read(int, struct areply *);
int tac_cont_send_seq(int, char *, int);
#define tac_cont_send(fd, pass) tac_cont_send_seq((fd), (pass), 3)
HDR *_tac_req_header(u_char, int);
void _tac_crypt(u_char *, HDR *, int);
u_char *_tac_md5_pad(int, HDR *);
void tac_add_attrib(struct tac_attrib **, char *, char *);
void tac_free_attrib(struct tac_attrib **);
char *tac_acct_flag2str(int);
int tac_acct_send(int, int, const char *, char *, char *, struct tac_attrib *);
int tac_acct_read(int, struct areply *);
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t);
char *xstrcpy(char *, const char *, size_t);
char *_tac_check_header(HDR *, int);
int tac_author_send(int, const char *, char *, char *, struct tac_attrib *);
int tac_author_read(int, struct areply *);
void tac_add_attrib_pair(struct tac_attrib **, char *, char, char *);
int tac_read_wait(int, int, int, int *);
int tac_cmd_author(const char *tac_server_name, const char *tac_secret,
                   const char *user, char * tty, char *remote_addr,
                   char *service, char *protocol, char *command,
                   int timeout, bool quiet, const char *source_ip,
                   const char *src_namespace, const char *dst_namespace );
char * get_ip_port_tuple(struct sockaddr *sa, char *ip,
                         unsigned short *port, size_t maxlen,
                         bool quiet);
int get_priv_level(struct addrinfo *tac_server, const char *tac_secret,
                    char *user, char *tty, char *remote_addr,
                    bool quiet);

/* Prototypes for a few program-wide used functions.  */
extern void *xmalloc (size_t n)
__attribute_malloc__ __attribute_alloc_size__ ((1));
extern void *xcalloc (size_t n, size_t s)
__attribute_malloc__ __attribute_alloc_size__ ((1, 2));
extern void *xrealloc (void *o, size_t n)
__attribute_malloc__ __attribute_alloc_size__ ((2));
extern char *xstrdup (const char *) __attribute_malloc__;

/* magic.c */
u_int32_t magic(void);

#ifdef __cplusplus
}
#endif

#endif
