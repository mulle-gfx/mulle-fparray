# mulle-fparray Library Documentation for AI
<!-- Keywords: floating-point, dynamic-array, allocator, enumeration, graphics, animation -->

## 1. Introduction & Purpose

mulle-fparray is a C library providing dynamic, resizable arrays for floating-point
numbers. It is used as a known type in the MulleCA animation engine and closely
resembles `mulle-structarray` from mulle-container, but with compile-time
knowledge of its element type (`float`). It offers two tiers: an allocator-aware
convenience wrapper (`mulle_floatarray`) with embedded allocator, and a base
implementation (`mulle__floatarray`) requiring explicit allocator parameters —
trading convenience for control. Part of the mulle-gfx ecosystem; depends on
mulle-core for foundational types (`mulle_range`, `mulle_data`) and memory
management (`mulle_allocator`).

## 2. Key Concepts & Design Philosophy

- **Two-Tier Architecture**: `mulle__floatarray` (base) requires an explicit
  `struct mulle_allocator *` for every allocating operation. `mulle_floatarray`
  (wrapper) stores the allocator in the struct, so allocating functions omit the
  allocator parameter. Both tiers share the same memory layout via
  `MULLE__FLOATARRAY_BASE`.

- **Allocator Awareness**: All memory operations go through `mulle_allocator`,
  enabling custom strategies, debugging, and leak detection.

- **Growth Strategy**: Doubling on overflow. Capacity and count are tracked
  separately; `_add()` triggers growth automatically when full.

- **Static Storage Optimization**: `_init_with_static_storage()` uses a
  caller-provided buffer. No heap allocation until the static buffer is
  exhausted, at which point the array transparently switches to dynamic
  allocation (malloc).

- **Sentinel Implementation**: Three pointers — `_storage` (start), `_curr`
  (next write position), `_sentinel` (end of capacity) — enable fast boundary
  checks with pointer subtraction.

- **No-Callback Enumeration**: Forward and reverse enumerators use inline structs
  with pointer increment/decrement. No heap allocation, no function pointer
  overhead.

## 3. Core API & Data Structures

### 3.1. `mulle--floatarray.h` — Base Implementation

All functions in this header require an explicit `struct mulle_allocator *`
parameter for allocating operations.

#### `struct mulle__floatarray`
```c
struct mulle__floatarray
{
   float     *_storage;
   float     *_curr;
   float     *_sentinel;
   float     *_initial_storage;
};
```

 - **Lifecycle:**
   - `void _mulle__floatarray_init(struct mulle__floatarray *array, unsigned int capacity, struct mulle_allocator *allocator)` — allocate with given capacity.
   - `void _mulle__floatarray_init_with_static_storage(struct mulle__floatarray *array, unsigned int count, float *storage, struct mulle_allocator *allocator)` — use caller buffer of `count` floats as initial storage.
   - `void _mulle__floatarray_init_with_floats(struct mulle__floatarray *array, float *values, unsigned int n, struct mulle_allocator *allocator)` — allocate and copy `n` floats from `values`.
   - `struct mulle__floatarray *mulle__floatarray_alloc(struct mulle_allocator *allocator)` — heap-allocate empty struct (does not init storage).
   - `struct mulle__floatarray *mulle__floatarray_create(unsigned int capacity, struct mulle_allocator *allocator)` — heap-allocate struct and init in one call.
   - `void _mulle__floatarray_done(struct mulle__floatarray *array, struct mulle_allocator *allocator)` — free internal storage (unless static).
   - `void mulle__floatarray_destroy(struct mulle__floatarray *array, struct mulle_allocator *allocator)` — `_done` + free the struct itself.
   - `MULLE__FLOATARRAY_INIT(storage, type, count)` — compound-literal initializer for stack or static allocation.

- **Adding Elements:**
  - `void _mulle__floatarray_add(struct mulle__floatarray *array, float item, struct mulle_allocator *allocator)` — append; grows if full.
  - `void _mulle__floatarray_add_guaranteed(struct mulle__floatarray *array, float item)` — append without realloc check (asserts room exists).

- **Reserving Space (returns writable pointer):**
  - `float *_mulle__floatarray_guarantee(struct mulle__floatarray *array, unsigned int count, struct mulle_allocator *allocator)` — ensure room for `count` more items, returns `_curr`. Does NOT advance `_curr`.
  - `float *_mulle__floatarray_advance(struct mulle__floatarray *array, unsigned int count, struct mulle_allocator *allocator)` — guarantee + advance `_curr` by `count`. Returns start of reserved region.
  - `float *_mulle__floatarray_reserve(struct mulle__floatarray *array, struct mulle_allocator *allocator)` — advance by 1, return writable slot.

- **Accessors:**
  - `unsigned int _mulle__floatarray_get_count(struct mulle__floatarray *array)` — number of used elements.
  - `unsigned int _mulle__floatarray_get_used(struct mulle__floatarray *array)` — alias for `get_count`.
  - `unsigned int _mulle__floatarray_get_size(struct mulle__floatarray *array)` — current capacity in elements.
  - `size_t _mulle__floatarray_get_used_as_length(struct mulle__floatarray *array)` — used bytes.
  - `size_t _mulle__floatarray_get_size_as_length(struct mulle__floatarray *array)` — capacity bytes.
  - `float *_mulle__floatarray_get_floats(struct mulle__floatarray *array)` — raw `_storage` pointer (borrowed).
  - `float _mulle__floatarray_get(struct mulle__floatarray *array, unsigned int i)` — element at index (asserts in-bounds).
  - `float _mulle__floatarray_get_first(struct mulle__floatarray *array)` — first element, or NAN if empty.
  - `float _mulle__floatarray_get_last(struct mulle__floatarray *array)` — last element, or NAN if empty.
  - `float _mulle__floatarray_get_current(struct mulle__floatarray *array)` — value at `_curr` (use with guarantee).

- **Mutation:**
  - `void _mulle__floatarray_set(struct mulle__floatarray *array, unsigned int index, float item, struct mulle_allocator *allocator)` — write at index; if index >= capacity, zero-grows to index+1 first.
  - `unsigned int _mulle__floatarray_set_count(struct mulle__floatarray *array, unsigned int count, struct mulle_allocator *allocator)` — grow or shrink element count; returns number of elements added (0 if shrunk). Shrinking also calls `_size_to_fit`.
  - `void _mulle__floatarray_zero_to_count(struct mulle__floatarray *array, unsigned int count, struct mulle_allocator *allocator)` — like `set_count` but zero-initializes newly added elements.
  - `void _mulle__floatarray_reset(struct mulle__floatarray *array)` — reset `_curr` to `_storage`; does not free memory.
  - `void _mulle__floatarray_remove_last(struct mulle__floatarray *array)` — decrement `_curr` if not empty.

- **Copy / Transfer:**
  - `void _mulle__floatarray_add_floatarray(struct mulle__floatarray *array, struct mulle__floatarray *other, struct mulle_range range, struct mulle_allocator *allocator)` — append a validated range from `other`.
  - `void _mulle__floatarray_copy(struct mulle__floatarray *array, struct mulle__floatarray *other, struct mulle_allocator *allocator)` — reset `array`, then copy all of `other` in.
  - `struct mulle_data _mulle__floatarray_extract_data(struct mulle__floatarray *buffer, struct mulle_allocator *allocator)` — transfer ownership of internal storage into a `mulle_data`; leaves array empty but caller must eventually free via allocator.

- **Capacity Control (global symbols, not inline):**
  - `void _mulle__floatarray_grow(struct mulle__floatarray *array, struct mulle_allocator *allocator)` — double capacity.
  - `void _mulle__floatarray_sizeto(struct mulle__floatarray *array, unsigned int new_count, struct mulle_allocator *allocator)` — resize to exact element count.
  - `void _mulle__floatarray_size_to_fit(struct mulle__floatarray *array, struct mulle_allocator *allocator)` — shrink capacity to used count.

#### `struct mulle__floatarrayenumerator`
```c
struct mulle__floatarrayenumerator
{
   float     *_curr;
   float     *_sentinel;
};
```
 - `struct mulle__floatarrayenumerator _mulle__floatarray_enumerate(struct mulle__floatarray *array)` — init forward enumerator.
 - `int _mulle__floatarrayenumerator_next(struct mulle__floatarrayenumerator *rover, float *item)` — return 1 and set `*item`, or 0 on exhaustion.
 - `void _mulle__floatarrayenumerator_done(struct mulle__floatarrayenumerator *rover)` — no-op on this type.
 - `mulle__floatarrayenumerator_empty` — macro expanding to `((struct mulle__floatarrayenumerator) { 0 })` for nil-safe code.

#### `struct mulle__floatarrayreverseenumerator`
```c
struct mulle__floatarrayreverseenumerator
{
   float     *_curr;
   float     *_sentinel;
};
```
 - `struct mulle__floatarrayreverseenumerator _mulle__floatarray_reverseenumerate(struct mulle__floatarray *array)` — init reverse enumerator.
 - `int _mulle__floatarrayreverseenumerator_next(struct mulle__floatarrayreverseenumerator *rover, float *item)` — return 1 on backwards step, 0 when done.
 - `void _mulle__floatarrayreverseenumerator_done(struct mulle__floatarrayreverseenumerator *rover)` — no-op.
 - `mulle__floatarrayreverseenumerator_empty` — macro expanding to `((struct mulle__floatarrayreverseenumerator) { 0 })` for nil-safe code.

#### Macros
- `mulle__floatarray_for(array, item)` — forward iteration: declares an enumerator, calls `_next`.
- `mulle__floatarray_for_reverse(array, item)` — reverse iteration.

### 3.2. `mulle-floatarray.h` — Wrapper Implementation

```c
struct mulle_floatarray
{
   float     *_storage;          // MULLE__FLOATARRAY_BASE
   float     *_curr;
   float     *_sentinel;
   float     *_initial_storage;
   struct mulle_allocator   *allocator;
};
```
Identical memory layout to base, with an extra `allocator` field. Functions with no allocator parameter read from `array->allocator`. Functions prefixed with `_` skip NULL-guard for performance.

 - **Lifecycle:**
   - `struct mulle_floatarray *mulle_floatarray_alloc(struct mulle_allocator *allocator)` — heap-allocate empty struct (does not init storage).
   - `void _mulle_floatarray_init(struct mulle_floatarray *array, unsigned int capacity, struct mulle_allocator *allocator)` — init (no NULL guard).
   - `void mulle_floatarray_init(struct mulle_floatarray *array, unsigned int capacity, struct mulle_allocator *allocator)` — init with NULL guard.
   - `void _mulle_floatarray_init_with_static_storage(struct mulle_floatarray *array, unsigned int count, void *storage, struct mulle_allocator *allocator)` — init with pre-allocated buffer (no NULL guard).
   - `void mulle_floatarray_init_with_static_storage(struct mulle_floatarray *array, unsigned int count, void *storage, struct mulle_allocator *allocator)` — init with pre-allocated buffer (NULL-guarded).
   - `struct mulle_floatarray *mulle_floatarray_create(unsigned int capacity, struct mulle_allocator *allocator)` — alloc + init.
   - `MULLE_FLOATARRAY_INIT(storage, count, allocator)` — compound-literal initializer for stack or static allocation.
  - `void _mulle_floatarray_done(struct mulle_floatarray *array)` — free internal storage (no NULL guard).
  - `void mulle_floatarray_done(struct mulle_floatarray *array)` — free internal storage (NULL-guarded).
  - `void mulle_floatarray_destroy(struct mulle_floatarray *array)` — done + free struct.
  - `void _mulle_floatarray_reset(struct mulle_floatarray *array)` — reset count (no NULL guard).
  - `void mulle_floatarray_reset(struct mulle_floatarray *array)` — reset count (NULL-guarded).

- **Adding:**
  - `void _mulle_floatarray_add(struct mulle_floatarray *array, float item)` — append via embedded allocator.
  - `void mulle_floatarray_add(struct mulle_floatarray *array, float item)` — NULL-guarded; **WARNING: also skips `item == 0.0f` due to `if (array && item)` check.** Use `_mulle_floatarray_add` for zero/NAN values.
  - `void _mulle_floatarray_add_guaranteed(struct mulle_floatarray *array, float item)` — append without growth check.

- **Reserving:**
  - `float *_mulle_floatarray_guarantee(struct mulle_floatarray *array, unsigned int count)` — ensure room for `count` items.
  - `float *_mulle_floatarray_advance(struct mulle_floatarray *array, unsigned int count)` — guarantee + advance `_curr`.
  - `float *_mulle_floatarray_reserve(struct mulle_floatarray *array)` — advance by 1, return writable slot.
  - `float *mulle_floatarray_reserve(struct mulle_floatarray *array)` — NULL-guarded reserve.

- **Accessors:**
  - `unsigned int _mulle_floatarray_get_count(struct mulle_floatarray *array)` / `mulle_floatarray_get_count(struct mulle_floatarray *array)` — element count.
  - `unsigned int _mulle_floatarray_get_size(struct mulle_floatarray *array)` / `mulle_floatarray_get_size(struct mulle_floatarray *array)` — capacity.
  - `size_t _mulle_floatarray_get_used_as_length(struct mulle_floatarray *array)` — used bytes.
  - `size_t _mulle_floatarray_get_size_as_length(struct mulle_floatarray *array)` / `mulle_floatarray_get_size_as_length(struct mulle_floatarray *array)` — capacity bytes.
  - `struct mulle_allocator *_mulle_floatarray_get_allocator(struct mulle_floatarray *array)` / `mulle_floatarray_get_allocator(struct mulle_floatarray *array)` — embedded allocator.
  - `float *_mulle_floatarray_get_floats(struct mulle_floatarray *array)` — raw storage pointer.
  - `float _mulle_floatarray_get_first(struct mulle_floatarray *array)` / `mulle_floatarray_get_first(struct mulle_floatarray *array)` — first or NAN.
  - `float _mulle_floatarray_get(struct mulle_floatarray *array, unsigned int i)` / `mulle_floatarray_get(struct mulle_floatarray *array, unsigned int i)` — element at index (NAN if NULL array).
  - `float _mulle_floatarray_get_last(struct mulle_floatarray *array)` / `mulle_floatarray_get_last(struct mulle_floatarray *array)` — last or NAN.
  - `float _mulle_floatarray_get_current(struct mulle_floatarray *array)` / `mulle_floatarray_get_current(struct mulle_floatarray *array)` — value at `_curr`.
  - `void _mulle_floatarray_remove_last(struct mulle_floatarray *array)` / `mulle_floatarray_remove_last(struct mulle_floatarray *array)` — decrement `_curr`.

- **Mutation:**
  - `void _mulle_floatarray_set(struct mulle_floatarray *array, unsigned int index, float item)` / `mulle_floatarray_set(struct mulle_floatarray *array, unsigned int index, float item)` — write at index, grow if needed.
  - `void _mulle_floatarray_set_count(struct mulle_floatarray *array, size_t count)` — adjust count.
  - `void _mulle_floatarray_zero_to_count(struct mulle_floatarray *array, size_t count)` — adjust count + zero new elements.

- **Copy:**
  - `void _mulle_floatarray_add_floatarray(struct mulle_floatarray *array, struct mulle_floatarray *other, struct mulle_range range)` / `mulle_floatarray_add_floatarray(struct mulle_floatarray *array, struct mulle_floatarray *other, struct mulle_range range)` — append range from other.
  - `void mulle_floatarray_copy(struct mulle_floatarray *array, struct mulle_floatarray *other)` — reset + copy all of `other`.

- **Capacity:**
  - `void _mulle_floatarray_size_to_fit(struct mulle_floatarray *array)` — shrink to used count.

#### Enumerators (wrapper)

`struct mulle_floatarrayenumerator` and `struct mulle_floatarrayreverseenumerator` mirror the base types exactly (same layout). Functions follow the same pattern:
- `_mulle_floatarray_enumerate()` / `mulle_floatarray_enumerate()`
- `_mulle_floatarrayenumerator_next()` / `mulle_floatarrayenumerator_next()`
- `_mulle_floatarrayenumerator_done()` / `mulle_floatarrayenumerator_done()`
- `_mulle_floatarray_reverseenumerate()` / `mulle_floatarray_reverseenumerate()`
- `_mulle_floatarrayreverseenumerator_next()` / `mulle_floatarrayreverseenumerator_next()`
- `_mulle_floatarrayreverseenumerator_done()` / `mulle_floatarrayreverseenumerator_done()`

Macros: `mulle_floatarray_for(array, item)` and `mulle_floatarray_for_reverse(array, item)`.

### 3.3. `mulle-fparray.h` — Main Header

Version introspection:
- `static inline unsigned int mulle_fparray_get_version_major(void)` — major version.
- `static inline unsigned int mulle_fparray_get_version_minor(void)` — minor version.
- `static inline unsigned int mulle_fparray_get_version_patch(void)` — patch version.
- `uint32_t mulle_fparray_get_version(void)` — packed version `(major << 20) | (minor << 8) | patch`.

### 3.4. Symbol Visibility

`MULLE__FPARRAY_GLOBAL` controls export visibility. Defined as `MULLE_C_GLOBAL`
when building the library, `extern` when consuming. The following are global
(not inline) symbols exported from the .c file:
- `_mulle__floatarray_sizeto_length`
- `_mulle__floatarray_sizeto`
- `_mulle__floatarray_grow`
- `_mulle__floatarray_guarantee`
- `_mulle__floatarray_set_count`
- `_mulle__floatarray_zero_to_count`
- `_mulle__floatarray_add_floatarray`
- `_mulle__floatarray_init_with_floats`

## 4. Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| `get(index)` | O(1) | Direct pointer arithmetic with assert bounds-check. |
| `add()` | O(1) amortized | O(n) when reallocation triggers doubling. |
| `add_guaranteed()` | O(1) | Assert-only; no growth path. |
| `set(index)` | O(1) if in-bounds | O(n) if index >= capacity (zero-grow to index). |
| `reset()` | O(1) | Just resets `_curr`. |
| `add_floatarray()` | O(m) | m = elements copied via memcpy. |
| `guarantee(n)` | O(1) if sufficient capacity | O(n) if reallocation required. |
| Forward/reverse enum | O(n) | Inline pointer increment, no malloc. |
| `_done()` | O(1) | One free call. |
| `size_to_fit()` | O(1) (realloc may shrink) | Realloc to exact used size. |

- **Struct overhead**: 4 pointers (32 bytes on 64-bit) for base, 5 pointers (40 bytes) for wrapper.
- **Wasted capacity**: Up to 2× due to doubling strategy.
- **Not thread-safe**. Read-only access on completed arrays is safe without concurrent writers.

## 5. AI Usage Recommendations & Patterns

**Best Practices:**
- Always initialize with `_init`/`create` before use; always `_done`/`destroy` after.
- Use the `_mulle__floatarray_*` (base) functions when you need explicit allocator control.
- Use `mulle_floatarray_*` (wrapper) for convenience when embedded allocator is acceptable.
- For tight loops, call `_guarantee(n)` once then use `_add_guaranteed()` to avoid repeated growth checks.
- Prefer `_init_with_static_storage()` for small temporary arrays to avoid heap allocs.
- Use `_advance()` or `_reserve()` for bulk writes — they return a pointer you write into directly.
- Check for NAN return values from `get_first()`, `get_last()`, `get()` on empty/uninitialized arrays.

**Common Pitfalls:**
- **Wrapper `mulle_floatarray_add(array, item)` silently drops 0.0f values** because of `if (array && item)`. Use `_mulle_floatarray_add()` when you need to store zeros or NANs.
- Do not free pointers returned by `get_floats()` — they are borrowed.
- Do not use `add_guaranteed()` without prior `guarantee()` — asserts will fire.
- `_advance()` and `_reserve()` advance `_curr`; you must write to the returned pointer.
- Do not mix base (`mulle__floatarray`) and wrapper (`mulle_floatarray`) functions on the same struct instance.
- `set()` at an out-of-bounds index zero-initializes intermediate elements.
- Enumerators have no protection against concurrent array modifications.
- The `_`-prefixed wrapper functions (e.g. `_mulle_floatarray_add`) skip NULL guards for performance — caller must ensure non-NULL.

**Idiomatic Patterns:**
- Use `mulle_floatarray_for(array, value)` macro for clean iteration.
- Combine `_guarantee()` with `_add_guaranteed()` for batch additions.
- Use `_advance()` followed by `memcpy`/loop for bulk insertion.
- Prefer `copy()` or `add_floatarray()` over manual element-by-element copying.
- Pass `NULL` allocator to use the default malloc-based allocator indirectly via `mulle_allocator`.

## 6. Integration Examples

### Example 1: Creating and Populating a Simple Array

```c
#include <mulle-fparray/mulle-fparray.h>
#include <stdio.h>

void   simple_example( void)
{
   struct mulle_floatarray   *array;
   float                     value;
   unsigned int              i;

   array = mulle_floatarray_create( 10, NULL);

   for( i = 0; i < 5; i++)
   {
      value = (float) i * 3.14f;
      _mulle_floatarray_add( array, value);   // use _ variant for 0.0f safety
   }

   printf( "First: %.2f\n", mulle_floatarray_get_first( array));
   printf( "Last: %.2f\n", mulle_floatarray_get_last( array));
   printf( "Count: %u\n", mulle_floatarray_get_count( array));

   mulle_floatarray_destroy( array);
}
```

### Example 2: Static Storage for Temporary Arrays

```c
#include <mulle-fparray/mulle-fparray.h>

float   compute_sum( float *values, unsigned int n)
{
   struct mulle_floatarray   array;
   float                     storage[ 32];
   float                     sum;
   float                     value;
   unsigned int              i;

   mulle_floatarray_init_with_static_storage( &array,
                                               32,
                                               storage,
                                               NULL);
   for( i = 0; i < n; i++)
   {
      value = values[ i] * 2.0f;
      _mulle_floatarray_add( &array, value);
   }

   sum = 0.0f;
   mulle_floatarray_for( &array, value)
   {
      sum += value;
   }

   mulle_floatarray_done( &array);
   return( sum);
}
```

### Example 3: High-Performance Batch Addition with Guarantee

```c
#include <mulle-fparray/mulle-fparray.h>

void   batch_add_optimized( struct mulle_floatarray *array,
                             float *values,
                             unsigned int count)
{
   unsigned int   i;

   _mulle_floatarray_guarantee( array, count);

   for( i = 0; i < count; i++)
      _mulle_floatarray_add_guaranteed( array, values[ i]);
}
```

### Example 4: Forward and Reverse Enumeration

```c
#include <mulle-fparray/mulle-fparray.h>
#include <stdio.h>

void   enumerate_example( struct mulle_floatarray *array)
{
   struct mulle_floatarrayenumerator           rover;
   struct mulle_floatarrayreverseenumerator    rrover;
   float                                       value;

   printf( "Forward: ");
   rover = mulle_floatarray_enumerate( array);
   while( _mulle_floatarrayenumerator_next( &rover, &value))
      printf( "%.2f ", value);
   mulle_floatarrayenumerator_done( &rover);
   printf( "\n");

   printf( "Reverse: ");
   rrover = mulle_floatarray_reverseenumerate( array);
   while( _mulle_floatarrayreverseenumerator_next( &rrover, &value))
      printf( "%.2f ", value);
   mulle_floatarrayreverseenumerator_done( &rrover);
   printf( "\n");
}
```

### Example 5: Array Copying and Concatenation

```c
#include <mulle-fparray/mulle-fparray.h>

struct mulle_floatarray   *combine_arrays( struct mulle_floatarray *a,
                                            struct mulle_floatarray *b)
{
   struct mulle_floatarray   *result;
   struct mulle_range        range;

   result = mulle_floatarray_create( 0, NULL);

   mulle_floatarray_copy( result, a);

   range = mulle_range_make( 5, 5);
   mulle_floatarray_add_floatarray( result, b, range);

   return( result);
}
```

### Example 6: Using Base API with Custom Allocator

```c
#include <mulle-fparray/mulle-fparray.h>
#include <stdio.h>

void   base_api_example( struct mulle_allocator *allocator)
{
   struct mulle__floatarray   array;
   float                      *ptr;
   unsigned int               i;

   _mulle__floatarray_init( &array, 100, allocator);

   ptr = _mulle__floatarray_advance( &array, 10, allocator);
   for( i = 0; i < 10; i++)
      ptr[ i] = (float) i * 1.5f;

   printf( "Count: %u\n", _mulle__floatarray_get_count( &array));

   _mulle__floatarray_done( &array, allocator);
}
```

### Example 7: Iteration Using For-Loop Macros

```c
#include <mulle-fparray/mulle-fparray.h>
#include <stdio.h>

void   macro_iteration( struct mulle_floatarray *array)
{
   float   value;

   mulle_floatarray_for( array, value)
   {
      printf( "%.2f\n", value);
   }

   mulle_floatarray_for_reverse( array, value)
   {
      printf( "%.2f\n", value);
   }
}
```

## 7. Dependencies

Direct mulle-sde dependency:
- **mulle-core**: Provides `mulle_allocator`, `mulle_range`, `mulle_data`, `mulle_c11` macros
  (`MULLE_C_NONNULL_FIRST`, `MULLE_C_GLOBAL`, etc.).

Transitive testing dependency:
- **mulle-testallocator**: Used in tests for leak detection.
