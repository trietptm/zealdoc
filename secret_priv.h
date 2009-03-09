#ifndef __SECRET_PRIV_H_
#define __SECRET_PRIV_H_

#include <secret.h>
#include <dfastm.h>
#include <logger.h>
#include <module.h>
#include <eloop.h>
#include <panic.h>
#include <uiserv.h>

#define SECRET_MATCH_EXACT	2
#define SECRET_MATCH_WILD	1
#define SECRET_MATCH_NONE	0

typedef struct _secret_trans_t secret_trans_t;
typedef struct _secret_source_t secret_source_t;
typedef struct _secret_format_t secret_format_t;
typedef struct _secret_profile_t secret_profile_t;

struct _secret_profile_t {
	const char *name;
	int type;
	const char *path;
	ui_entry_t *cs;

	atomic_t refcnt;
	list_t link;
};

struct _secret_source_t {
	const char *name;
#define SECRET_SOURCE_FILE	"file"
#define SECRET_SOURCE_CARD	"card"

	/* start transaction context, return transaction context */
	void *(*init_trans)(void);
	/* stop transaction by context */
	void (*exit_trans)(void *trans);
	/* start search transaction */
	int (*start_search)(void *trans,
			    const char *domain, const char *user,
			    const char *format, char *secret);
	/* start delete transaction */
	int (*start_delete)(void *trans,
			    const char *domain, const char *user,
			    const char *format);
	/* start update transaction */
	int (*start_update)(void *trans,
			    const char *domain, const char *user,
			    const char *format, const char *secret);
	list_t link;
};

struct _secret_trans_t {
	char *domain;
	char *user;
	char *format;
	char *secret;	/* readonly secret */
	
	int trans_type;
#define SECRET_TRANS_UPDATE	1
#define SECRET_TRANS_DELETE	2
#define SECRET_TRANS_SEARCH	3

	char *secret_res;	/* store found secret and return it to caller. */
	void *secret_ctx;
	complete_cb secret_comp;
	
	void *source_ctx;
	secret_source_t *source_inst;

	stm_instance_t *fsmi;

	list_t link;
};

struct _secret_format_t {
	const char *name;
	const char *desc;
#define SECRET_FORMAT_PLAIN	"plain"
	int print_able;
#define SECRET_FORMAT_PRINT	1
#define SECRET_FORMAT_NOPRINT	0
	int (*crypt)(const char *plain, uint8_t *, int maxlen);
	list_t link;
};

int secret_register_format(secret_format_t *f);
void secret_unregister_format(secret_format_t *f);

int secret_register_source(secret_source_t *s);
void secret_unregister_source(secret_source_t *s);

secret_format_t *secret_get_format(const char *name);
secret_format_t *secret_get_next_format(secret_format_t *format);
secret_format_t *secret_get_next_format_by_name(const char *name);

/* raise SSF event */
void secret_source_failure(void *source_ctx);
/* raise SSS event */
void secret_source_success(void *source_ctx);

#endif /* __SECRET_PRIV_H_ */
