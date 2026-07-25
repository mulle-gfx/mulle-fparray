# Patterns

<!-- Keywords: init, add, guarantee, advance, enumerate, copy, static-storage -->

## Create, populate, read, destroy (wrapper)

From `test/10-float/floatarray.c`:

```c
struct mulle_floatarray  *array;
unsigned int             i;

array = mulle_floatarray_create( 10, NULL);         // initial capacity 10, default allocator

for( i = 0; i < 1000; i++)
   _mulle_floatarray_add( array, (float) i + 1848); // use _ variant to preserve 0.0

// random-access read
for( i = 0; i < 1000; i++)
   assert( _mulle_floatarray_get( array, i) == (float) i + 1848);

mulle_floatarray_destroy( array);
```

## Forward and reverse enumeration

```c
float   value;
struct mulle_floatarrayenumerator  rover;

rover = mulle_floatarray_enumerate( array);
while( _mulle_floatarrayenumerator_next( &rover, &value))
   printf( "%.2f\n", value);
_mulle_floatarrayenumerator_done( &rover);   // no-op, still call it
```

Macro form — declares a hidden `rover__` variable:

```c
mulle_floatarray_for( array, value)
{
   printf( "%.2f\n", value);
}

mulle_floatarray_for_reverse( array, value)
{
   printf( "%.2f\n", value);
}
```

## Static storage (no heap until it overflows)

```c
struct mulle_floatarray  array;
float                    storage[ 32];          // stack buffer

mulle_floatarray_init_with_static_storage( &array, 32, storage, NULL);

for( i = 0; i < n; i++)
   _mulle_floatarray_add( &array, values[ i] * 2.0f);

// any add beyond 32 automatically mallocs a larger buffer
mulle_floatarray_done( &array);                // free if heap was allocated
```

From `README.md` test example — `_init_with_static_storage` uses caller buffer
until exhausted, then transparently switches to dynamic allocation.

## Reserve + guaranteed add (batch hot path)

```c
_mulle_floatarray_guarantee( array, count);            // ensure room

for( i = 0; i < count; i++)                            // no growth check in loop
   _mulle_floatarray_add_guaranteed( array, values[ i]);
```

## Advance + direct write (bulk insertion)

```c
float  *dst;

dst = _mulle_floatarray_advance( array, 10, allocator);// guarantees + advances _curr
for( i = 0; i < 10; i++)
   dst[ i] = (float) i * 1.5f;                          // write into reserved slot
```

## Copy and range-append

From `test/10-float/floatarray-copy.c`:

```c
struct mulle_floatarray  *other;

other = mulle_floatarray_create( 10, NULL);
for( i = 0; i < 10; i++)
   _mulle_floatarray_add( other, (float) i + 1848);

// copy items 1..8 (inclusive) into array
mulle_floatarray_add_floatarray( array, other, mulle_range_make( 1, 8));

// full copy
mulle_floatarray_copy( array, other);   // resets array first, then copies all

mulle_floatarray_destroy( other);
```

## Base API with custom allocator

```c
struct mulle__floatarray   array;
float                      *ptr;

_mulle__floatarray_init( &array, 100, my_allocator);

ptr = _mulle__floatarray_advance( &array, 10, my_allocator);
for( i = 0; i < 10; i++)
   ptr[ i] = (float) i * 1.5f;

_mulle__floatarray_done( &array, my_allocator);
```

## NULL-guarded operations on optional arrays

```c
mulle_floatarray_add( array, 3.14f);   // safe: no-op if array is NULL
mulle_floatarray_get_count( array);    // returns 0 if array is NULL
mulle_floatarray_get_first( array);    // returns NAN if array is NULL
mulle_floatarray_done( array);         // safe: no-op if array is NULL
mulle_floatarray_destroy( array);      // safe: no-op if array is NULL

mulle_floatarray_reserve( array);      // returns NULL if array is NULL
```

From `test/10-float/floatarray.c` `null()` function — all NULL-guarded calls
are tested against a NULL pointer.

## Memory stealing

Transfer ownership of internal storage to a `mulle_data` — useful to pass
array contents without copying:

```c
struct mulle_data  data;

data = _mulle__floatarray_extract_data( &array, allocator);
// array is now empty; caller owns data.bytes and must free via allocator
```
