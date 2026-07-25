# Quirks

<!-- Keywords: pitfall, invariant, restriction, sentinel, zero-drop, guard, static-storage -->

## `mulle_floatarray_add` silently drops 0.0 and NAN

From `src/float/mulle-floatarray.h:252` — the NULL-guarded overload:

```c
if( array && item)           // item == 0.0f → false → skip!
   _mulle_floatarray_add( array, item);
```

Always use `_mulle_floatarray_add()` when zeros or NANs are valid data.

## `_`-prefixed functions require non-NULL

`_mulle_floatarray_add`, `_mulle_floatarray_get`, `_mulle_floatarray_init`,
etc. are decorated with `MULLE_C_NONNULL_FIRST` and skip all NULL checks.
Passing NULL is undefined behaviour — use the unprefixed variant when the
pointer can be NULL.

## `_done` only frees if no static storage

From `src/float/mulle--floatarray.h:108`:

```c
if( ! array->_initial_storage)
   mulle_allocator_free( allocator, array->_storage);
```

After `_init_with_static_storage`, `_initial_storage` is set to non-NULL, so
`_done` does NOT free. If the array later outgrew the static buffer,
`_sizeto_length` sets `_initial_storage` to NULL, making `_done` free the heap
storage. Always call `_done` regardless.

## `set` at out-of-bounds index zero-fills intermediates

`_mulle__floatarray_set` calls `_mulle__floatarray_zero_to_count(array, index + 1, ...)` when
`&_storage[index] >= _sentinel`. This allocates and zero-initialises every
element from current count up to `index`. The value is then written at
`index`. This can be expensive and hide bugs.

## `_guarantee` does not advance `_curr`

`_mulle__floatarray_guarantee` returns `_curr` and ensures capacity, but does
NOT advance `_curr`. After writing into the returned pointer, call
`_add_guaranteed` or manually advance `_curr`.

`_advance` and `_reserve` DO advance `_curr` — they are the preferred APIs
when you want to consume the slot immediately.

## Enumerator `_done` is a no-op

Both `_mulle__floatarrayenumerator_done` and
`_mulle__floatarrayreverseenumerator_done` are empty inline functions. Call
them anyway for consistency — they may gain meaning in future versions.

## `get_first` / `get_last` / `get_current` return NAN for empty arrays

From `src/float/mulle--floatarray.h:200`:

```c
return( array->_curr > array->_storage ? *array->_storage : NAN);
```

Always check with `isnan()` or guard with `get_count` before using the result.

## `get_current` requires prior `_guarantee`

From `src/float/mulle--floatarray.h:209`: reads `*array->_curr` with no bounds
check. Only valid to call after `_guarantee` has reserved space at `_curr`.

## `assert`-based bounds in debug builds

- `_mulle__floatarray_get(array, i)` asserts `address < array->_sentinel`
- `_mulle__floatarray_add_guaranteed` asserts `_curr < _sentinel`

Production builds with `NDEBUG` skip these checks — out-of-bounds access
becomes silent memory corruption.

## Growth doubles capacity

From `src/float/mulle--floatarray.c:58`:

```c
new_size = (old_size ? old_size : sizeof( float)) * 2;
```

Minimum growth from empty state creates space for `sizeof(float)` bytes (one
float). Wasted capacity up to 2× due to doubling.

## `_sizeto_length` aborts on shrink

From `src/float/mulle--floatarray.c:12`:

```c
if( new_size < old_size)
   abort();
```

Cannot be used to reduce capacity. Use `_mulle__floatarray_size_to_fit` or
`set_count` (which shrinks internally via `size_to_fit`) instead.

## Not thread-safe

No locking. Read-only access on a completed, non-concurrently-written array is
safe. Any writer or mixed read/write from multiple threads is UB.

## Base and wrapper structs are layout-compatible, but do not mix functions

Both use `MULLE__FLOATARRAY_BASE` as the leading fields, and the wrapper adds
`allocator` as the fifth field. You can cast between them, but using wrapper
functions on a base struct (or vice versa) will access the wrong allocator
field or miss it entirely.
