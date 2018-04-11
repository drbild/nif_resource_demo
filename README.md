# Erlang NIF Resource Demo

Demonstrates how to use `enif_keep_resource` and
`enif_release_resource` to manage resource types who underlying `C`
structures hold pointers to each other.


### Explanatnion

The NIF defines two structs, `child_t` and `parent_t`.  A `parent_t`
holds a pointer to an instance of `child_t`.

When `foo::parent_t_init(Child)` is called, `enif_keep_resource` is
called on the child so that it will not be freed while the parent is
still alive.

When the `parent_t` is freed, `enif_release_resource` is called on the
child so that it can be freed (if no one else has a reference still
keeping it alive.)

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
