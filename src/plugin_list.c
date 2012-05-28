#include "plugin_list.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "err.h"

/* expected shared symbols */
static const char AUG_PLUGIN_NAME[] = "aug_plugin_name";
static const char AUG_PLUGIN_INIT[] = "aug_plugin_init";
static const char AUG_PLUGIN_FREE[] = "aug_plugin_free";
static const char AUG_PLUGIN_MAJOR[] = "aug_plugin_api_version_major";

static const char ERR_VERSION_MISMATCH[] = "version mismatch";
static const char ERR_NAME_MISMATCH[] = "name mismatch";

void plugin_list_init(struct aug_plugin_list *pl) {
	list_head_init(&pl->head);
}

void plugin_list_free(struct aug_plugin_list *pl) {
	struct aug_plugin_item *next, *i;

	list_for_each_safe(&pl->head, i, next, node) {
		/* todo: call i->plugin.free here? */
		list_del(&i->node);
		free(i);
	}
}

/* returns non-zero and sets *err_msg* (if *err_msg* non-null) if
 * an error occurs while loading 
 */
int plugin_list_push(struct aug_plugin_list *pl, const char *path, const char *name, 
						size_t namelen, const char **err_msg) {
	void *handle;
	const char *err;
	char *so_name;
	struct aug_plugin_item *item;
	int (*major_version)();
	int (*init)( AUG_API_INIT_ARG_PROTO );
	void (*free)( AUG_API_FREE_ARG_PROTO );
	
#	define CHECK_DLERR() \
		do { \
		if( (err = dlerror() ) != NULL) \
			goto fail; \
		} while(0)

	
	dlerror();
	handle = dlopen(path, RTLD_NOW);
	CHECK_DLERR();

	major_version = dlsym(handle, AUG_PLUGIN_MAJOR);
	CHECK_DLERR();
	if((*major_version)() != AUG_API_VERSION_MAJOR) {
		err = ERR_VERSION_MISMATCH;
		goto fail;
	}
		
	so_name = dlsym(handle, AUG_PLUGIN_NAME);
	CHECK_DLERR();
	
	if(strncmp(name, so_name, namelen) != 0) {
		err = ERR_NAME_MISMATCH;
		goto fail;
	}
		
	init = dlsym(handle, AUG_PLUGIN_INIT);
	CHECK_DLERR();

	free = dlsym(handle, AUG_PLUGIN_FREE);
	CHECK_DLERR();

	item = malloc( sizeof(struct aug_plugin_item) );
	if(item == NULL)
		err_exit(0, "memory error!");

	/* hack to initialize the constant 
	 * members of the plugin struct */
	*( (void **) &item->plugin.name) = so_name;
	*( (void **) &item->plugin.init) = init;
	*( (void **) &item->plugin.free) = free;
	memset(&item->plugin.callbacks, 0, sizeof( struct aug_plugin_cb ) );
	
	list_add_tail(&pl->head, &item->node);

#	undef CHECK_DLERR

	return 0;
fail:
	if(err_msg != NULL)
		*err_msg = err;

	return -1;
}

void plugin_list_del(struct aug_plugin_list *pl, struct aug_plugin_item *item) {
	list_del_from(&pl->head, &item->node);
	free(item);
}