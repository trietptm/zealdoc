#include <secret.h>
#include <eloop.h>
#include <uiserv.h>
#include <strutl.h>
#include <service.h>
#include "secret_priv.h"

DECLARE_LIST(secret_formats);
DECLARE_LIST(secret_trans);
DECLARE_LIST(secret_profiles);
DECLARE_LIST(secret_profiles_gc);

secret_source_t *secret_source = NULL;

#define for_each_profile(t)			\
	list_for_each_entry(secret_profile_t, t, &secret_profiles, link)
#define for_each_profile_safe(t, n)		\
	list_for_each_entry_safe(secret_profile_t, t, n, &secret_profiles, link)

#define for_each_format(f)	\
	list_for_each_entry(secret_format_t, f, &secret_formats, link)
#define for_each_search(s)	\
	list_for_each_entry(secret_trans_t, s, &secret_trans, link)

void secret_trans_success(secret_trans_t *s);
void secret_trans_failure(secret_trans_t *s);
void secret_trans_raise_success(secret_trans_t *s);
void secret_trans_raise_failure(secret_trans_t *s);
void secret_trans_raise_open(secret_trans_t *s);
void secret_trans_raise_close(secret_trans_t *s);
void secret_trans_raise_wild(secret_trans_t *s);
void secret_trans_raise_exact(secret_trans_t *s);
void secret_trans_raise_event(secret_trans_t *sear, int event);

static void secret_trans_free_delay(void *eloop, void *user);
static void secret_trans_free(secret_trans_t *sear);

void secret_source_failure(void *lower)
{
	secret_trans_t *s;
	for_each_search(s) {
		if (s->source_ctx == lower)
			secret_trans_raise_failure(s);
	}
}

void secret_source_success(void *lower)
{
	secret_trans_t *s;
	for_each_search(s) {
		if (s->source_ctx == lower)
			secret_trans_raise_success(s);
	}
}

static void secret_stm_log(const stm_instance_t *fsmi,
			int level, const char *fmt, ...)
{
	int lvl;
	va_list ap;
	
	if (level == STM_LOG_ERR)
		lvl = LOG_ERR;
	else
		lvl = LOG_DEBUG;
	va_start(ap, fmt);
	loggingv(log_logger, lvl, fmt, ap);
	va_end(ap);
}

/* ============================================================ *
 * TRANS stm
 * ============================================================ */
#define SECRET_TRANS_EVENT_OPEN	0
#define SECRET_TRANS_EVENT_CLOSE	1
#define SECRET_TRANS_EVENT_SSS		2	/* Source-Trans-Success */
#define SECRET_TRANS_EVENT_SSF		3	/* Source-Trans-Failure */
#define SECRET_TRANS_EVENT_ESR		4	/* Exact-Search-Ready */
#define SECRET_TRANS_EVENT_WSR		5	/* Wild-Search-Ready */
#define SECRET_TRANS_EVENT_COUNT	6

#define	SECRET_TRANS_EVENT_NAMES {	\
	"open",				\
	"close",			\
	"success",			\
	"failure",			\
	"exact-ready",			\
	"wild-ready",			\
}

#define SECRET_TRANS_STATE_INIT		0
#define SECRET_TRANS_STATE_EXACT	1
#define SECRET_TRANS_STATE_WILD		2
#define SECRET_TRANS_STATE_EXIT		3
#define SECRET_TRANS_STATE_CONT		4
#define SECRET_TRANS_STATE_COUNT	5

#define SECRET_TRANS_STATE_NAMES {	\
	"init",				\
	"exact",			\
	"wild",				\
	"exit",				\
	"prep-context",			\
}

#define STATE(state)			SECRET_TRANS_STATE_##state
#define EVENT(event)			SECRET_TRANS_EVENT_##event
#define ACTION(stem)			\
	secret_trans_act_##stem,sizeof(secret_trans_act_##stem)/sizeof(stm_action_fn)

static const char *secret_trans_state_names[] = SECRET_TRANS_STATE_NAMES;
static const char *secret_trans_event_names[] = SECRET_TRANS_EVENT_NAMES;

/* Open-Source-Transaction */
static int secret_trans_action_ost(stm_instance_t *fsmi, void *insti)
{
	secret_trans_t *trans = (secret_trans_t *)insti;

	BUG_ON(!trans->source_ctx);

	switch (trans->trans_type) {
	case SECRET_TRANS_SEARCH:
		if (trans->source_inst->start_search)
			trans->source_inst->start_search(trans->source_ctx, 
					trans->domain ? trans->domain : "*",
			 		trans->user, trans->format, trans->secret_res);
		break;
	case SECRET_TRANS_UPDATE:
		if (trans->source_inst->start_update)
			/* If format is NULL, update all format in list. */
			trans->source_inst->start_update(trans->source_ctx, 
					trans->domain, trans->user, trans->format, trans->secret);
		break;
	case SECRET_TRANS_DELETE:
		if (trans->source_inst->start_delete)
			trans->source_inst->start_delete(trans->source_ctx, 
					trans->domain, trans->user, trans->format);
		break;
	default:
		log_kern(LOG_ERR, "Unknown trans type.");
		return 0;
	}
	return 1;
}

/* Init-Trans-Context */
static int secret_trans_action_itc(stm_instance_t *fsmi, void *insti)
{
	secret_trans_t *trans = (secret_trans_t *)insti;

	if (trans->domain && !streq(trans->domain, "*"))
		secret_trans_raise_exact(trans);
	else
		secret_trans_raise_wild(trans);
	return 1;
}

/* Next-Trans-Context */
static int secret_trans_action_ntc(stm_instance_t *fsmi, void *insti)
{
	secret_trans_t *sear = (secret_trans_t *)insti;

	free(sear->domain);
	sear->domain = NULL;
	return 1;
}

/* Exit-Trans-Context */
static int secret_trans_action_etc(stm_instance_t *fsmi, void *insti)
{
	secret_trans_t *sear = (secret_trans_t *)insti;

	if (sear->fsmi->state == SECRET_TRANS_STATE_EXIT)
		return 0;
	eloop_register_timeout(NULL, 0, 0, secret_trans_free_delay, NULL, sear);
	return 1;
}

/* Secret-Trans-Success */
static int secret_trans_action_sts(stm_instance_t *fsmi, void *insti)
{
	secret_trans_t *sear = (secret_trans_t *)insti;
	secret_trans_success(sear);
	return 1;
}

/* Secret-Trans-Failure */
static int secret_trans_action_stf(stm_instance_t *fsmi, void *insti)
{
	secret_trans_t *sear = (secret_trans_t *)insti;
	secret_trans_failure(sear);
	return 1;
}

static int secret_trans_action_null(stm_instance_t *fsmi, void *insti)
{
	return 1;
}

static const stm_action_fn secret_trans_act_null[] = {
	secret_trans_action_null,
};

static const stm_action_fn secret_trans_act_itc[] = {
	secret_trans_action_itc,
};

static const stm_action_fn secret_trans_act_ost[] = {
	secret_trans_action_ost,
};

static const stm_action_fn secret_trans_act_etc[] = {
	secret_trans_action_etc,
};

static const stm_action_fn secret_trans_act_ntc_ost[] = {
	secret_trans_action_ntc,
	secret_trans_action_ost,
};

static const stm_action_fn secret_trans_act_sts_etc[] = {
	secret_trans_action_sts,
	secret_trans_action_etc,
};

static const stm_action_fn secret_trans_act_stf_etc[] = {
	secret_trans_action_stf,
	secret_trans_action_etc,
};

static const stm_entry_t secret_stm_trans_entries[] = {
	/* state	event		action		new state */
	{ STATE(INIT),	EVENT(OPEN),	ACTION(itc),	STATE(CONT) },

	{ STATE(CONT),	EVENT(ESR),	ACTION(ost),	STATE(EXACT) },
	{ STATE(CONT),	EVENT(WSR),	ACTION(ost),	STATE(WILD) },
	{ STATE(CONT),	EVENT(CLOSE),	ACTION(etc),	STATE(EXIT) },
		
	{ STATE(EXACT),	EVENT(SSF),	ACTION(ntc_ost),STATE(WILD) },
	{ STATE(EXACT),	EVENT(SSS),	ACTION(sts_etc),STATE(EXIT) },
	{ STATE(EXACT),	EVENT(CLOSE),	ACTION(etc),	STATE(EXIT) },
	
	{ STATE(WILD),	EVENT(SSF),	ACTION(stf_etc),STATE(EXIT) },
	{ STATE(WILD),	EVENT(SSS),	ACTION(sts_etc),STATE(EXIT) },
	{ STATE(WILD),	EVENT(CLOSE),	ACTION(etc),	STATE(EXIT) },
	
	{ 0,		0,		NULL,		0 }
};

const stm_table_t secret_stm_trans_table = {
	"TRANS",
	secret_stm_log,
	SECRET_TRANS_STATE_COUNT,
	&secret_trans_state_names[0],
	SECRET_TRANS_EVENT_COUNT, 
	&secret_trans_event_names[0],
	secret_stm_trans_entries,
};

#undef STATE
#undef EVENT
#undef ACTION

#define SEARCH_FAILURE	0
#define SEARCH_SUCCESS	1

void secret_trans_success(secret_trans_t *trans)
{
	if (trans->secret_comp)
		trans->secret_comp(trans->secret_ctx, SEARCH_SUCCESS);
}

void secret_trans_failure(secret_trans_t *trans)
{
	if (trans->secret_comp)
		trans->secret_comp(trans->secret_ctx, SEARCH_FAILURE);
}

/* post an event */
void secret_trans_raise_event(secret_trans_t *sear, int event)
{
	assert(sear);
	eloop_schedule_event(NULL, sear->fsmi, event, sear);
}

void secret_trans_raise_failure(secret_trans_t *s)
{
	secret_trans_raise_event(s, SECRET_TRANS_EVENT_SSF);
}

void secret_trans_raise_success(secret_trans_t *s)
{
	secret_trans_raise_event(s, SECRET_TRANS_EVENT_SSS);
}

void secret_trans_raise_exact(secret_trans_t *s)
{
	secret_trans_raise_event(s, SECRET_TRANS_EVENT_ESR);
}

void secret_trans_raise_wild(secret_trans_t *s)
{
	secret_trans_raise_event(s, SECRET_TRANS_EVENT_WSR);
}

void secret_trans_raise_open(secret_trans_t *s)
{
	secret_trans_raise_event(s, SECRET_TRANS_EVENT_OPEN);
}

void secret_trans_raise_close(secret_trans_t *s)
{
	secret_trans_raise_event(s, SECRET_TRANS_EVENT_CLOSE);
}

static void secret_trans_free_delay(void *eloop, void *user)
{
	secret_trans_t *sear = (secret_trans_t *)user;

	eloop_cancel_timeout(NULL, secret_trans_free_delay, NULL, sear);
	secret_trans_free(sear);
}

static int secret_source_init(secret_trans_t *trans)
{
	if (trans->source_inst->init_trans) 
		trans->source_ctx = trans->source_inst->init_trans();
	return (trans->source_ctx ? 0 : -1);
}

static void secret_source_exit(secret_trans_t *trans)
{
	if (trans->source_inst->exit_trans) 
		trans->source_inst->exit_trans(trans->source_ctx);
}

static void *secret_trans_start(const char *domain, const char *user,
			 const char *format, char *secret, void *ctx, complete_cb comp,
			 int trans_type)
{
	secret_trans_t *sear = NULL;
	int res;

	if (!user)
		return NULL;

	sear = malloc(sizeof (secret_trans_t));
	if (sear) {
		memset(sear, 0, sizeof (secret_trans_t));
		
		if (!secret_source) {
			log_kern(LOG_INFO, "SECRET: no source registered.");
			goto out_free;
		}
		sear->source_inst = secret_source;

		res = secret_source_init(sear);
		if (res) {
			log_kern(LOG_ERR, "SECRET: init source=%s failure.", secret_source->name);
			goto out_free;
		}
		sear->domain = domain ? strdup(domain) : NULL;
		sear->format = format ? strdup(format) : NULL;
		sear->user = strdup(user);
		/* XXX: different trans use different secret. */
		if (trans_type == SECRET_TRANS_SEARCH)
			/* search: writable secret, caller should free it */
			sear->secret_res = secret;
		else
			/* update/delete: readonly secret */
			sear->secret = secret ? strdup(secret) : NULL;

		sear->trans_type = trans_type;
		sear->secret_ctx = ctx;
		sear->secret_comp = comp;
		
		sear->fsmi = eloop_create_automaton(&secret_stm_trans_table, user,
						    SECRET_TRANS_STATE_INIT);

		list_init(&sear->link);
		list_insert_before(&sear->link, &secret_trans);

		/* start the state machine */
		secret_trans_raise_open(sear);
		return sear;
	}

out_free:
	/* Only need free sear */
	if (sear) free(sear);
	return NULL;
}

static void secret_trans_stop(secret_trans_t *trans)
{
	if (trans)
		/* Stop the state machine from outside. */
		secret_trans_raise_close(trans);
}

void *secret_search_start(const char *domain, const char *user,
			  const char *format, char *secret, void *ctx, complete_cb comp)
{
	return secret_trans_start(domain, user, format, secret, ctx, comp,
		SECRET_TRANS_SEARCH);
}

void secret_search_stop(void *trans)
{
	secret_trans_stop(trans);
}

static void secret_trans_free(secret_trans_t *trans)
{
	if (trans) {
		/* XXX: do not put this in secret_trans_stop */
		secret_source_exit(trans);
		if (trans->domain) free(trans->domain);
		if (trans->user)   free(trans->user);
		if (trans->secret) free(trans->secret);
		if (trans->format) free(trans->format);

		trans->secret_ctx = NULL;
		trans->secret_res = NULL;
		trans->secret_comp = NULL;
		list_delete(&trans->link);

		if (trans->fsmi) {
			eloop_delete_automaton(trans->fsmi);
			trans->fsmi = NULL;
		}
		free(trans);
	}
}

int secret_register_source(secret_source_t *s)
{
	if (s)
		secret_source = s;
	return 0;
}

void secret_unregister_source(secret_source_t *s)
{
	secret_source = NULL;
}

secret_format_t *secret_get_format(const char *name)
{
	secret_format_t *s;
	
	for_each_format(s) {
		if (strcasecmp(s->name, name) == 0)
			return s;
	}
	return NULL;
}

int secret_register_format(secret_format_t *s)
{
	if (s) {
		if (secret_get_format(s->name)) {
			log_kern(LOG_ERR,
				"SECRET: already registered format, format=%s",
				s->name);
			return 0;
		}
		list_init(&s->link);
		list_insert_before(&s->link, &secret_formats);
	}
	return 0;
}

void secret_unregister_format(secret_format_t *s)
{
	if (s)
		list_delete_init(&s->link);
}

secret_format_t *secret_get_next_format(secret_format_t *format)
{
	secret_format_t *f;
	int same_format = 0;

	for_each_format(f) {
		/* Get the 1st format_t if param is NULL. */
		if (!format)
			return f;

		/* get format_t after @format. */
		if (f == format) {
			same_format = 1;
			continue;
		}
		if (same_format && strcasecmp(format->name, f->name) != 0)
			return f;
	}
	return NULL;
}

secret_format_t *secret_get_next_format_by_name(const char *name)
{
	secret_format_t *f = secret_get_format(name);
	if (f)
		return secret_get_next_format(f);
	return NULL;
}

/************************************************************************/
/*          UI                                                          */
/************************************************************************/
string_map_t secret_trans_t_map[] = {
	{ "UPDATE",	SECRET_TRANS_UPDATE, },
	{ "SEARCH",	SECRET_TRANS_SEARCH, },
	{ "DELETE",	SECRET_TRANS_DELETE, },
	{ NULL, 0, },
};

#if 1
static void *secret_ui_start_trans(const char *domain, const char *user, 
				   const char *format, char *secret, int type)
{
	BUG_ON(type != SECRET_TRANS_UPDATE && 
	       type != SECRET_TRANS_DELETE &&
	       type != SECRET_TRANS_SEARCH);

	return secret_trans_start(domain, user, format, secret, NULL, NULL, type);
}

/* UI END */

static int secret_ui_update_entry(ui_session_t *sess, ui_entry_t *inst,
			       void *ctx, int argc, char **argv)
{
	char *domain, *user, *secret;

	if (streq(argv[0], "*"))
		return -1;

	domain = argv[0];
	user   = argv[1];
	secret = argv[2];
	/* domain, user, secret */
	secret_ui_start_trans(domain, user, NULL, secret, SECRET_TRANS_UPDATE);
	return 0;
}

static int secret_ui_delete_entry(ui_session_t *sess, ui_entry_t *inst,
			       void *ctx, int argc, char **argv)
{
	char *domain, *user, *format;

	if (streq(argv[2], "*"))
		return -1;

	format = argv[0];
	domain = argv[1];
	user   = argv[2];
	
	/* domain, user, format */
	secret_ui_start_trans(domain, user, format, NULL, SECRET_TRANS_DELETE);
	return 0;
}

static void secret_display_result(void *ctx, int res)
{
	secret_trans_t *trans = (secret_trans_t *)ctx;
	secret_format_t *format = secret_get_format(trans->format);

	if (format && res) {
		if (format->print_able) 
			log_kern(LOG_INFO, "search result:\ndomain\tuser\tformat\tsecret\n%s\t%s\t%s\t%s", 
				trans->domain, trans->user, trans->format, trans->secret_res);
		else
			log_kern(LOG_INFO, "search result:\ndomain\tuser\tformat\tsecret\n%s\t%s\t%s\t***",
					trans->domain, trans->user, trans->format);
	} else {
		log_kern(LOG_INFO, "search result: None");
	}
	/* only UI search need do it */
	free(trans->secret_res);

	/* Since STM may do search twice - wild and exact.
	 * And search from UI only need exact one.
	 * So we close STM at here. */
	secret_trans_stop(trans);
}

/* search and display */
static int secret_search_entry(ui_session_t *sess, ui_entry_t *inst,
			       void *ctx, int argc, char **argv)
{
	char *domain, *user, *format;
	secret_trans_t *trans = NULL;
	char *secret;

	/* XXX: free it after display */
	secret = malloc(sizeof (char) * MAX_SECRET_WORD_LEN);

	if (!secret)
		return -1;

	domain = argv[0];
	user   = argv[1];
	format = argv[2];
	
	if (user[0] == '*')
		return -1;
	/* domain, user, format secret */
	trans = secret_ui_start_trans(domain, user, format, secret, SECRET_TRANS_SEARCH);
	
	if (trans) {
		trans->secret_comp = secret_display_result;
		trans->secret_ctx = trans;
	} else
		free(secret);
	return 0;
}

#define SECRET_DESC	"Access account secrets"

#define SECRET_DOMAIN	0x01
#define SECRET_USER	0x02
#define SECRET_PASS	0x03
#define SECRET_FORMAT	0x04

string_map_t secret_param_map[] = {
	{ "domain",	SECRET_DOMAIN,	"domain" },
	{ "user",	SECRET_USER,	"user" },
	{ "pass",	SECRET_PASS,	"password" },
	{ "format",	SECRET_FORMAT,	"format"},
	{ NULL,		0,		NULL},
};

static void service_format_foreach(ui_choice_t *choice,
				    ui_iterate_fn func, void *data)
{
	secret_format_t *format;
	
	for_each_format(format) {
		func(choice, format, data);
	}
}

static const char *service_format_name(const void *iter)
{
	const secret_format_t *format = (const secret_format_t *)iter;
	return format->name;
}

static const char *service_format_desc(const void *iter)
{
	const secret_format_t *format = (const secret_format_t *)iter;
	return format->desc;
}

ui_choice_t secret_format_choice = {
	"secret_formats_choice",
	NULL,
	service_format_foreach,
	service_format_name,
	service_format_desc,
};

ui_argument_t secret_update_args[] = {
	{ "domain", "Input domain", NULL, UI_TYPE_STRING, },
	{ "user", "Input user", NULL, UI_TYPE_STRING, },
	{ "pass", "Input password", NULL, UI_TYPE_STRING, },
};

ui_command_t secret_update_command = {
	"update",
	"Update entry",
	".secret",
	UI_CMD_SINGLE_INST,
	secret_update_args,
	3,
	LIST_HEAD_INIT(secret_update_command.link),
	secret_ui_update_entry,
};

ui_argument_t secret_delete_args[] = {
	{ "format",
	  "Select a format in system",
	  "secret_formats_choice",
	  UI_TYPE_CHOICE, },
	{ "domain",
	  "Input domain",
	  NULL,
	  UI_TYPE_STRING, },
	{ "user",
	  "Input user",
	  NULL,
	  UI_TYPE_STRING, },
};

ui_command_t secret_delete_command = {
	"delete",
	"Delete entry",
	".secret",
	UI_CMD_SINGLE_INST,
	secret_delete_args,
	3,
	LIST_HEAD_INIT(secret_delete_command.link),
	secret_ui_delete_entry,
};

/* TODO: ui search */
ui_argument_t secret_search_args[] = {
	{ "domain",
	  "Input domain",
	  NULL,
	  UI_TYPE_STRING, },
	{ "user",
	  "Input user",
	  NULL,
	  UI_TYPE_STRING, },
	{ "format",
	  "Input format",
	  NULL,
	  UI_TYPE_STRING, },
};

ui_command_t secret_search_command = {
	"search",
	"Search entry",
	".secret",
	UI_CMD_SINGLE_INST,
	secret_search_args,
	3,
	LIST_HEAD_INIT(secret_search_command.link),
	secret_search_entry,
};

ui_schema_t secret_schema[] = {
	/* .secret */
	{ UI_TYPE_CLASS, UI_FLAG_SINGLE | UI_FLAG_EXTERNAL,
	  UI_TYPE_STRING, NULL, NULL,
	  ".secret", "secret", SECRET_DESC },

	/* .secret.profile */
	{ UI_TYPE_CLASS, 0,
	  UI_TYPE_STRING, NULL, NULL,
	  ".secret.profile", "profile", "secret profile" },

	{ UI_TYPE_NONE },
};

ui_parser_t secret_profile_parser[] = {
	{ ".secret.profile.path", NULL,
	  offsetof(secret_profile_t, path), },
	{ NULL },
};

ui_entry_t *secret_main_section = NULL;

void secret_prof_free(secret_profile_t *p)
{
	if (!p)
		return;
	
	if (atomic_dec_and_test(&p->refcnt)) {
		log_kern(LOG_DEBUG, 
			  "PROF: profile refcnt is clean, prof=%s", p->name);
		if (p->cs)
			ui_inst_delete_conf(p->cs);
		list_delete(&p->link);
		free(p);
	}
}

secret_profile_t *secret_profile_get(secret_profile_t *profile)
{
	atomic_inc(&profile->refcnt);
	return profile;
}

void secret_profile_put(secret_profile_t *profile)
{
	secret_prof_free(profile);
}

static secret_profile_t *secret_profile_by_name(const char *name)
{
	secret_profile_t *c;

	for_each_profile(c) {
		if (strcasecmp(name, c->name) == 0)
			return c;
	}
	return NULL;
}

secret_profile_t *secret_profile_get_by_name(const char *name)
{
	secret_profile_t *p = secret_profile_by_name(name);

	if (p)
		return secret_profile_get(p);
	return NULL;
}

static secret_profile_t *secret_profile_new(const char *name)
{
	secret_profile_t *p;

	p = calloc(1, sizeof(secret_profile_t));
	if (!p) {
		log_kern(LOG_ERR, "PROF: out of memory.");
		return NULL;
	}
	p->name = name;
	atomic_set(&p->refcnt, 1);
	list_init(&p->link);
	list_insert_before(&p->link, &secret_profiles);
	return p;
}

static secret_profile_t *secret_conf_prof(ui_entry_t *cs)
{
	secret_profile_t *c;
	const char *name = NULL;
	int rc;

	name = ui_get_conf_value(cs);
	if (!name) {
		log_kern(LOG_ERR, "PROF: missing client name");
		return NULL;
	}

	c = secret_profile_new(name);
	if (!c) {
		log_kern(LOG_ERR, "PROF: create profile fail");
		return NULL;
	}

	rc = ui_load_single_values(cs, c, secret_profile_parser);
	if (rc) {
		log_kern(LOG_ERR,
			 "PROF: cannot load profile, profile=%s", name);
		secret_prof_free(c);
		return NULL;
	}

	if (!c->path) {
		log_kern(LOG_ERR, "PROF: missing ifname.");
		goto err;
	}

	c->cs = ui_inst_get(cs);

	/* profile's acname may empty since we don't know server's name.
	 * But when session is established, we should set it in session.
	 */
	log_kern(LOG_DEBUG, "PROF: profile ready, prof=%s", c->name);
	return c;

err:
	secret_prof_free(c);
	return NULL;
}

static int secret_prof_start(void)
{
	secret_profile_t *c;
	ui_entry_t *cs = NULL;

	if (secret_main_section) {
		while ((cs = ui_minst_iterate_conf(secret_main_section,
						   "profile", cs)) != NULL) {
			ui_inst_hold(cs);
			c = secret_conf_prof(cs);
			ui_inst_delete_conf(cs);
		}
	}

	return 0;
}

static void secret_prof_stop(void)
{
	secret_profile_t *c;
	list_t *pos, *n;

	list_iterate_forward(pos, n, &secret_profiles) {
		c = list_entry(pos, secret_profile_t, link);
		list_delete_init(&c->link);
		/* phase may refer this now */
		list_insert_before(&c->link, &secret_profiles_gc);
		secret_prof_free(c);
	}
}

static int secret_conf_start(void)
{
	ui_entry_t *cs;
	const char *oid = "secret";

	if (secret_main_section) return 0;

	cs = ui_inst_lookup_conf(NULL, oid, NULL);
	if (!cs) {
		log_kern(LOG_ERR,
			  "CONF: cannot find instance, oid=%s", oid);
		return 1;
	}
	/* already get cs instance */
	secret_main_section = cs;

	return 0;
}

static void secret_conf_stop(void)
{
	if (secret_main_section) {
		ui_inst_delete_conf(secret_main_section);
		secret_main_section = NULL;
	}
}

static int secret_start(void)
{
	if (secret_conf_start() ||
	    secret_prof_start())
		return -1;
	return 0;
}

static void secret_stop(void)
{
	secret_prof_stop();
	secret_conf_stop();

}

handle_t secret_handle = NULL;

service_t secret_service = {
	"secret",
	SECRET_DESC,
	SERVICE_UP_ALWAYS,
	SERVICE_FLAG_SYSTEM,
	LIST_HEAD_INIT(secret_service.depends),
	secret_start,
	secret_stop,
};

int __init secret_init(void)
{
	ui_register_choice(&secret_format_choice);
	ui_register_schema(secret_schema);

	ui_register_command(&secret_update_command);
	ui_register_command(&secret_delete_command);
	ui_register_command(&secret_search_command);

	secret_handle = register_service(&secret_service);

	return secret_handle ? 0 : 1;
}

void __exit secret_exit(void)
{
	unregister_service(secret_handle);
	
	ui_unregister_command(&secret_search_command);
	ui_unregister_command(&secret_delete_command);
	ui_unregister_command(&secret_update_command);

	ui_unregister_schema(secret_schema);
	ui_unregister_choice(&secret_format_choice);
}
#endif
