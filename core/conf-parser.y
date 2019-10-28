%{
	#include <linux/limits.h>
	#include <sys/inotify.h>
	#include <stdio.h>
	#include <string.h>
        #include <stdlib.h>

        #include "core/conf.h"
	#include "core/io.h"
	#include "core/notify.h"
	#include "mem/alloc.h"

	static int conf_lex(FILE *fp);
	static void conf_error(FILE *fp, const char *err);
	static bool in_regex = false;
 %}

%union {
	ustr *str;
	int num;
}

%define api.prefix {conf_}
%locations
%define parse.trace
%define parse.error verbose

%parse-param {FILE *fp}
%lex-param {FILE *fp}

%token <num> NUM
%token <str> STR

%type <num> events_list

%printer { fprintf(yyo, "%X", $$); } NUM;
%printer { fprintf(yyo, "%s", $$); } STR;

/* Grammar */
%%
START:
%empty
{
	in_regex = false;
}
|
START lines
;

lines:
'\n'
|
expr '\n'
;

expr:
STR ':' { in_regex = true; } STR { in_regex = false; } ':' events_list ':' STR
{
	struct list_head *head;
	struct conf *conf;
	regex_t *preg;
	int err;

	preg = xmalloc(sizeof(regex_t));
	err = regcomp(preg, $4, REG_EXTENDED | REG_NOSUB);
	if (err) {
		char buf[1096];
		regerror(err, preg, buf, sizeof(buf));
		error("Fail to compile regex %s - %s", $4, buf);
		regfree(preg);
		goto end;
	}

	notify_get($1, $7, &head);

	conf = xmalloc(sizeof(struct conf));
	INIT_LIST_HEAD(&conf->node);
	list_add(&conf->node, head);
	conf->cmd = $9;
	conf->regex = preg;
end:
	{};
}
;

events_list:
NUM { $$ = $1; }
|
events_list '|' NUM
{
	$$ = $1 | $3;
}
;


%%

__notnull
static void conf_error(FILE *fp, const char *err)
{
	(void)fp;
	error("%s at %d.%d-%d.%d\n",
	      err,
	      yylloc.first_line,
	      yylloc.last_line,
	      yylloc.first_column,
	      yylloc.last_column);
}

__notnull
static int char_get(FILE *fp)
{
	char tmp = fgetc(fp);
	if (tmp == '\n') {
		yylloc.first_line = ++yylloc.last_line;
		yylloc.last_column = 0;
	}
	else {
		++yylloc.last_column;
	}
	return tmp;
}

__notnull
static void char_unget(FILE *fp, char c)
{
	ungetc(c, fp);
}

__notnull
static int str_to_event(const char *str, usize len)
{
	struct event {
		const char *name;
		int value;
	};
	static const struct event events[] = {
		{"ACCESS", IN_ACCESS},
		{"ATTRIB", IN_ATTRIB},
		{"CLOSE_WRITE", IN_CLOSE_WRITE},
		{"CLOSE_NOWRITE", IN_CLOSE_NOWRITE},
		{"CREATE", IN_CREATE},
		{"DELETE", IN_DELETE},
		{"DELETE_SELF", IN_DELETE_SELF},
		{"MODIFY", IN_MODIFY},
		{"MOVE_SELF", IN_MOVE_SELF},
		{"MOVED_FROM", IN_MOVED_FROM},
		{"MOVED_TO", IN_MOVED_TO},
		{"OPEN", IN_OPEN},
		{"ALL_EVENTS", IN_ALL_EVENTS}
	};
	for (usize i=0; i<array_size(events); ++i)
		if (0 == strncmp(events[i].name, str, len))
			return events[i].value;
	return -1;
}

__const
static bool char_is_sep(char c)
{
	switch (c) {
	case '|':
		return !in_regex;
	case ':':
	case '\n':
	case '\0':
	case EOF:
		return true;
	default:
		return false;
	}
}

__notnull
static int conf_lex(FILE *fp)
{
	static char buf[PATH_MAX];

	char c = char_get(fp);
	if (c == EOF)
		return 0;
	if (!char_is_sep(c)) {
		usize i = 0;
		int event;
		do {
			buf[i++] = c;
			c = char_get(fp);
		} while (!char_is_sep(c) && i < sizeof(buf));
		char_unget(fp, c);
		event = str_to_event(buf, i);
		if (event == -1) {
			yylval.str = ustr_mkn(buf, i);
			return STR;
		}
		yylval.num = event;
		return NUM;
	}
	return c;
}
