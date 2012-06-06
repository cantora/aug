#ifndef AUG_STDERR_TAP_H
#define AUG_STDERR_TAP_H

#include <stdio.h>

/* from ccan/str/str.h */
#define stringify(expr)		stringify_1(expr)
#define stringify_1(expr)	#expr

#ifdef diag
#	undef diag
#endif
#define diag(...) \
	do { \
		fputc('#', stderr); \
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while(0)	

#ifdef ok1
#	undef ok1
#endif
#define ok1(e, ...) \
	do { \
		if(e) { \
			fprintf(stderr, "ok: "); \
		} \
		else { \
			fprintf(stderr, "failed: "); \
		} \
		fprintf(stderr, stringify(e) "\n"); \
	} while(0)	

#ifdef ok
#	undef ok
#endif
#define ok(e, ...) \
	do { \
		if(e) { \
			fprintf(stderr, "ok: "); \
		} \
		else { \
			fprintf(stderr, "failed: "); \
		} \
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while(0)	

#ifdef pass
#	undef pass
#endif
#define pass(...) ok(1, __VA_ARGS__)

#ifdef fail
#	undef fail
#endif
#define fail(...) ok(0, __VA_ARGS__)

#ifdef plan_tests
#	undef plan_tests
#endif
#define plan_tests(n) (void)(n)

#endif /* AUG_STDERR_TAP_H */