#include <stdio.h>
#include <string.h>

#include <erl_nif.h>

//==================== C Libary ==================

typedef struct child {
  int num;
} child_t;

typedef struct parent {
  child_t* child;
} parent_t;


//==================== NIF ==================

ErlNifResourceType* CHILD_T_RESOURCE_TYPE;
ErlNifResourceType* PARENT_T_RESOURCE_TYPE;

static void
free_child_t(ErlNifEnv* env, void* obj) {
  fprintf(stderr, "free child_t@%p\n\r", obj);

  // Don't need to call enif_free, since child_t was
  // allocated with enif_alloc_resource, not enif_alloc.
}

static void
free_parent_t(ErlNifEnv* env, void* obj) {
  parent_t* parent = (parent_t*) obj;

  fprintf(stderr, "free parent_t@%p\n\r", parent);

  // Don't need to call enif_free, since child_t was
  // allocated with enif_alloc_resource, not enif_alloc.

  // Must call enif_release_resource on the child, since we called
  // enif_keep_resource when we added it to the parent.
  fprintf(stderr, "release child_t@%p from parent_t@%p\n\r", parent, parent->child);
  enif_release_resource(parent->child);
}

static int
load(ErlNifEnv* env, void** priv, ERL_NIF_TERM load_info) {
  const char* module = "foo";

  CHILD_T_RESOURCE_TYPE  = enif_open_resource_type(env, module, "child_t",  free_child_t,  ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER, NULL);
  if (NULL == CHILD_T_RESOURCE_TYPE)
    return -1;

  PARENT_T_RESOURCE_TYPE = enif_open_resource_type(env, module, "parent_t", free_parent_t, ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER, NULL);
  if (NULL == PARENT_T_RESOURCE_TYPE)
    return -1;

  return 0;
}

static ERL_NIF_TERM
foo_child_t_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int num;
  child_t* child;
  ERL_NIF_TERM child_resource;

  if (argc != 1 || !enif_get_int(env, argv[0], &num)) {
    return enif_make_badarg(env);
  }

  child = enif_alloc_resource(CHILD_T_RESOURCE_TYPE, sizeof(child_t));
  child->num = num;

  child_resource = enif_make_resource(env, child);
  enif_release_resource(child);

  return child_resource;
}

static ERL_NIF_TERM
foo_parent_t_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  child_t *child;
  parent_t *parent;
  ERL_NIF_TERM parent_resource;

  if (argc != 1 || !enif_get_resource(env, argv[0], CHILD_T_RESOURCE_TYPE, (void**)&child)) {
    return enif_make_badarg(env);
  }

  parent = enif_alloc_resource(PARENT_T_RESOURCE_TYPE, sizeof(parent_t));
  parent->child = child;
  enif_keep_resource(child);

  parent_resource = enif_make_resource(env, parent);
  enif_release_resource(parent);

  return parent_resource;
}

static ERL_NIF_TERM
foo_parent_t_num(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  parent_t* parent;
  int num;

  if (argc != 1 || !enif_get_resource(env, argv[0], PARENT_T_RESOURCE_TYPE, (void**)&parent)) {
    return enif_make_badarg(env);
  }

  num = parent->child->num;

  return enif_make_int(env, num);
}

static ErlNifFunc funcs[] = {
  {"child_t_init", 1, foo_child_t_init, 1},
  {"parent_t_init", 1, foo_parent_t_init, 1},
  {"parent_t_num", 1, foo_parent_t_num, 1}
};

ERL_NIF_INIT(foo, funcs, &load, NULL, NULL, NULL);
