\0;95;0c# Erlang NIF Resource Demo

Demonstrates how to use `enif_keep_resource` and
`enif_release_resource` to manage resource types whose underlying `C`
structures hold pointers to each other.


### Explanation

The NIF defines two structs, `child_t` and `parent_t`.  A `parent_t`
holds a pointer to an instance of `child_t`.

``` c
typedef struct child {
  int num;
} child_t;

typedef struct parent {
  child_t* child;
} parent_t;
```

When `foo::parent_t_init(Child)` is called, `enif_keep_resource` is
called on the child so that it will not be freed while the parent is
still alive.

``` c
static ERL_NIF_TERM
foo_parent_t_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  child_t *child;
  parent_t *parent;

  <snip>

  if (argc != 1 || !enif_get_resource(env, argv[0], CHILD_T_RESOURCE_TYPE, (void**)&child)) {
    return enif_make_badarg(env);
  }

  <snip>

  parent = enif_alloc_resource(PARENT_T_RESOURCE_TYPE, sizeof(parent_t));
  parent->child = child;
  enif_keep_resource(child);

  <snip>
}
```

When the `parent_t` is freed, `enif_release_resource` is called on the
child so that it can be freed (if no one else has a reference still
keeping it alive.)

``` c
static void
free_parent_t(ErlNifEnv* env, void* obj) {
  parent_t* parent = (parent_t*) obj;

  <snip>

  enif_release_resource(parent->child);

  <snip>
}
```

The `free_child_t` and `free_parent_t` functions do NOT call
`enif_free`.  That is only required if the memory was allocated with
`enif_alloc`. We use `enif_alloc_resource`; the system takes care of
freeing the memory for us.

### Usage

``` bash
cd c_src
make

cd ../src

erl
1> c(foo).
2> foo:test(15).
3> foo:test_keep_child(20).
```

`foo:test/1` demonstrates that the "child" resource is not freed when
the last Erlang term referencing it disappears, as long as the
"parent" resource still has a reference.  It is freed when the
"parent" resource is freed.

`foo:test_keep_reference/1` demonstrates that the "child" resource is
not freed when the "parent" resource is freed, if someone else still
has a refrence to the "child".  Only the parent is freed and its hold
on the child is released.
