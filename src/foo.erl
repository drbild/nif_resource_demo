-module(foo).
-on_load(init/0).

-export([child_t_init/1, parent_t_init/1, parent_t_num/1, test/1, test_keep_child/1]).

init() ->
  erlang:load_nif("foo", 0).

child_t_init(_Num) ->
  erlang:nif_error(undef).

parent_t_init(_Child) ->
  erlang:nif_error(undef).

parent_t_num(_Parent) ->
  erlang:nif_error(undef).

test(Num) ->
  Child = child_t_init(Num),
  Parent = parent_t_init(Child),
  test_1(Parent).

test_1(Parent) ->
  io:format("Calling GC. Child will not be freed because Parent has reference.~n"),
  erlang:garbage_collect(),
  Value = parent_t_num(Parent),
  io:format("Child Num is ~B.~n", [Value]),
  test_2().

test_2() ->
  io:format("Calling GC.  Parent will be freed and release child. Child will be freed because no one has reference.~n"),
  erlang:garbage_collect().

test_keep_child(Num) ->
  Child = child_t_init(Num),
  Parent = parent_t_init(Child),
  Value = parent_t_num(Parent),
  io:format("Child Num is ~B.~n", [Value]),
  test_keep_child_1(Child).

test_keep_child_1(Child) ->
  io:format("Calling GC. Parent will be freed and will release child. Child will not be freed because we still have reference.~n"),
  erlang:garbage_collect(),
  test_keep_child_2(Child).

test_keep_child_2(_Child) ->
  io:format("Calling GC. Child will be freed because no one has reference.~n"),
  erlang:garbage_collect().
