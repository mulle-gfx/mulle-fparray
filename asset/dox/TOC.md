# mulle-fparray Library Documentation for AI
<!-- Keywords: floating-point, dynamic-array, generic, allocation, enumeration -->

## 1. Introduction & Purpose

mulle-fparray is a specialized C library that provides dynamic, resizable arrays for floating-point numbers and other primitive types. The library is designed for high-performance graphics and numerical computing applications where efficient memory management and fast array operations are critical. It offers both allocator-aware (`mulle_floatarray`) and base-level (`mulle__floatarray`) implementations, allowing flexibility between convenience and performance. The library uses a generic code generation approach to support multiple numeric types from a single template implementation. mulle-fparray is part of the mulle-gfx ecosystem and depends on mulle-core for foundational data structures and memory management utilities.

## 2. Key Concepts & Design Philosophy

mulle-fparray is built around several core design principles:

- **Two-Tier Architecture**: The library provides both a base implementation (`mulle__floatarray`) and a wrapper implementation (`mulle_floatarray`). The base version requires an explicit allocator parameter for every operation, while the wrapper stores the allocator in the structure for convenience.

- **Generic Code Generation**: Rather than using C preprocessor macros or manual duplication, the library uses a script-based code generation approach. The `make-type-array` script transforms the float implementation into versions for other types (e.g., double, int), maintaining consistency across type variants.

- **Allocator Awareness**: All memory operations go through `mulle_allocator`, enabling custom memory management strategies, debugging, and leak detection. This is critical for graphics applications with specific memory requirements.

- **Growth Strategy**: Arrays grow dynamically using a doubling strategy when capacity is exceeded. The library distinguishes between capacity (allocated size) and count (used elements), optimizing for scenarios with frequent additions.

- **Static Storage Optimization**: Arrays can be initialized with static storage buffers, avoiding initial allocations for small, temporary arrays. When the static buffer is exhausted, the array seamlessly transitions to dynamic allocation.

- **Sentinel-Based Implementation**: Internally, arrays use pointer arithmetic with `_storage` (start), `_curr` (current end), and `_sentinel` (capacity end) pointers, enabling fast boundary checks and efficient iteration.

- **Enumeration Pattern**: The library provides forward and reverse enumerators with a consistent iteration interface, using structs rather than callbacks for zero-overhead enumeration.

## 3. Core API & Data Structures

### 3.1. `mulle--floatarray.h` (Base Implementation)

#### `struct mulle__floatarray`
- **Purpose:** Low-level dynamic float array without embedded allocator.
- **Key Fields:**
  - `float *_storage`: Pointer to the start of allocated memory.
  - `float *_curr`: Pointer to the next available slot (current end of used elements).
  - `float *_sentinel`: Pointer to the end of allocated capacity.
  - `float *_initial_storage`: Pointer to static storage if initialized with one, NULL otherwise.
- **Lifecycle Functions:**
  - `_mulle__floatarray_init()`: Initialize with specified capacity.
  - `_mulle__floatarray_init_with_static_storage()`: Initialize using pre-allocated buffer.
  - `_mulle__floatarray_init_with_floats()`: Initialize with a copy of existing float values.
  - `_mulle__floatarray_done()`: Cleanup and free memory (requires allocator parameter).
  - `_mulle__floatarray_destroy()`: Cleanup and free the array struct itself.
- **Core Operations:**
  - `_mulle__floatarray_add()`: Add a float, growing array if necessary.
  - `_mulle__floatarray_add_guaranteed()`: Add a float assuming capacity exists (no checks).
  - `_mulle__floatarray_set()`: Set value at specific index, growing if needed.
  - `_mulle__floatarray_get()`: Get value at specific index.
  - `_mulle__floatarray_guarantee()`: Ensure capacity for N additional items.
  - `_mulle__floatarray_advance()`: Reserve space for N items and advance current pointer.
  - `_mulle__floatarray_reserve()`: Reserve space for one item and advance.
  - `_mulle__floatarray_add_floatarray()`: Append a range from another array.
  - `_mulle__floatarray_reset()`: Reset count to zero without deallocating.
  - `_mulle__floatarray_set_count()`: Set the count, growing or shrinking as needed.
  - `_mulle__floatarray_zero_to_count()`: Set count and zero-initialize new elements.
- **Inspection Functions:**
  - `_mulle__floatarray_get_count()`: Get number of used elements.
  - `_mulle__floatarray_get_size()`: Get total capacity.
  - `_mulle__floatarray_get_used_as_length()`: Get used size in bytes.
  - `_mulle__floatarray_get_size_as_length()`: Get capacity in bytes.
  - `_mulle__floatarray_get_floats()`: Get pointer to internal storage.
  - `_mulle__floatarray_get_first()`: Get first element (NAN if empty).
  - `_mulle__floatarray_get_last()`: Get last element (NAN if empty).
  - `_mulle__floatarray_get_current()`: Get current element pointer value.
- **Advanced Operations:**
  - `_mulle__floatarray_size_to_fit()`: Shrink capacity to match count.
  - `_mulle__floatarray_sizeto()`: Set capacity to specific size.
  - `_mulle__floatarray_grow()`: Double the array capacity.
  - `_mulle__floatarray_extract_data()`: Extract internal storage as `mulle_data`, transferring ownership.
  - `_mulle__floatarray_remove_last()`: Remove last element.

#### `struct mulle__floatarrayenumerator`
- **Purpose:** Iterator for forward traversal of float arrays.
- **Key Fields:**
  - `float *_curr`: Current position in iteration.
  - `float *_sentinel`: End boundary of iteration.
- **Operations:**
  - `_mulle__floatarray_enumerate()`: Create enumerator from array.
  - `_mulle__floatarrayenumerator_next()`: Advance and get next item, returns 0 when exhausted.
  - `_mulle__floatarrayenumerator_done()`: Cleanup (no-op for this enumerator type).

#### `struct mulle__floatarrayreverseenumerator`
- **Purpose:** Iterator for reverse traversal of float arrays.
- **Key Fields:** Same as forward enumerator but traverses backwards.
- **Operations:**
  - `_mulle__floatarray_reverseenumerate()`: Create reverse enumerator.
  - `_mulle__floatarrayreverseenumerator_next()`: Get previous item, returns 0 when exhausted.
  - `_mulle__floatarrayreverseenumerator_done()`: Cleanup (no-op).

#### Convenience Macros
- `mulle__floatarray_for()`: Macro for iterating over array elements.
- `mulle__floatarray_for_reverse()`: Macro for reverse iteration.

### 3.2. `mulle-floatarray.h` (Wrapper Implementation)

#### `struct mulle_floatarray`
- **Purpose:** Higher-level float array with embedded allocator for convenience.
- **Key Fields:**
  - Contains all fields from `MULLE__FLOATARRAY_BASE` (storage pointers).
  - `struct mulle_allocator *allocator`: Embedded allocator pointer.
- **Lifecycle Functions:**
  - `mulle_floatarray_alloc()`: Allocate array struct.
  - `mulle_floatarray_init()`: Initialize with capacity.
  - `mulle_floatarray_init_with_static_storage()`: Initialize with static buffer.
  - `mulle_floatarray_create()`: Allocate and initialize in one call.
  - `mulle_floatarray_done()`: Cleanup.
  - `mulle_floatarray_destroy()`: Cleanup and free the struct.
  - `mulle_floatarray_reset()`: Reset to empty state.
- **Core Operations:** All operations from base version, but without requiring allocator parameter:
  - `mulle_floatarray_add()`: Add element (uses embedded allocator).
  - `mulle_floatarray_add_guaranteed()`: Add without capacity check.
  - `mulle_floatarray_set()`: Set at index.
  - `mulle_floatarray_get()`: Get at index.
  - `mulle_floatarray_guarantee()`: Ensure capacity.
  - `mulle_floatarray_advance()`: Reserve multiple elements.
  - `mulle_floatarray_reserve()`: Reserve one element.
  - `mulle_floatarray_add_floatarray()`: Append from another array.
  - `mulle_floatarray_copy()`: Copy entire contents from another array.
- **Inspection Functions:** Mirrors base version:
  - `mulle_floatarray_get_count()`: Number of elements.
  - `mulle_floatarray_get_size()`: Capacity.
  - `mulle_floatarray_get_allocator()`: Get embedded allocator.
  - `mulle_floatarray_get_floats()`: Internal storage pointer.
  - `mulle_floatarray_get_first()`: First element or NAN.
  - `mulle_floatarray_get_last()`: Last element or NAN.
  - `mulle_floatarray_remove_last()`: Remove last element.

#### `struct mulle_floatarrayenumerator`
- **Purpose:** Convenience wrapper for base enumerator.
- **Operations:**
  - `mulle_floatarray_enumerate()`: Create enumerator.
  - `mulle_floatarrayenumerator_next()`: Get next element.
  - `mulle_floatarrayenumerator_done()`: Cleanup.

#### `struct mulle_floatarrayreverseenumerator`
- **Purpose:** Convenience wrapper for reverse enumerator.
- **Operations:**
  - `mulle_floatarray_reverseenumerate()`: Create reverse enumerator.
  - `mulle_floatarrayreverseenumerator_next()`: Get previous element.
  - `mulle_floatarrayreverseenumerator_done()`: Cleanup.

#### Convenience Macros
- `mulle_floatarray_for()`: Iterate forward through array.
- `mulle_floatarray_for_reverse()`: Iterate backward through array.

### 3.3. `mulle-fparray.h` (Main Header)

#### Version Functions
- `mulle_fparray_get_version_major()`: Get major version number.
- `mulle_fparray_get_version_minor()`: Get minor version number.
- `mulle_fparray_get_version_patch()`: Get patch version number.
- `mulle_fparray_get_version()`: Get combined version as uint32_t.

## 4. Performance Characteristics

**Memory Allocation:**
- Initial allocation: O(1) for specified capacity
- Growth strategy: Doubling on overflow, amortized O(1) for additions
- Static storage: Zero allocations until capacity exceeded

**Element Access:**
- `get(index)`: O(1) direct pointer arithmetic
- `add()`: O(1) amortized (O(n) when reallocation needed)
- `add_guaranteed()`: O(1) guaranteed (no reallocation checks)
- `set(index)`: O(1) if within capacity, O(n) if growing needed

**Array Operations:**
- `reset()`: O(1) - just resets pointer
- `add_floatarray()`: O(m) where m is the number of elements copied
- `guarantee()`: O(1) if capacity sufficient, O(n) if reallocation needed

**Enumeration:**
- Forward/reverse enumeration: O(n) with minimal overhead (direct pointer increment)
- No heap allocation for enumerators
- Zero function call overhead for `_next()` inline operations

**Space Complexity:**
- Struct overhead: 4-5 pointers (32-40 bytes on 64-bit systems)
- Storage: Actual array size may be up to 2x used elements due to growth strategy
- Static storage mode: Zero heap overhead until capacity exceeded

**Thread Safety:**
- Not thread-safe: Requires external synchronization for concurrent access
- Read-only operations on fully constructed arrays are safe if no writers exist
- Enumerators are not thread-safe and should not be shared between threads

## 5. AI Usage Recommendations & Patterns

**Best Practices:**
- Always initialize arrays before use with `_init()` or `create()` functions.
- Prefer `_mulle__floatarray_*` functions when performance is critical and you need explicit control.
- Use `mulle_floatarray_*` functions for convenience when the embedded allocator overhead is acceptable.
- Use `_add_guaranteed()` in tight loops after calling `_guarantee()` to avoid repeated capacity checks.
- Prefer static storage initialization for small, temporary arrays (< 16-32 elements).
- Always call `_done()` or `destroy()` to avoid memory leaks.
- Use `_advance()` or `_reserve()` when you need direct pointer access to fill multiple elements efficiently.
- Check for NAN return values when using `get_first()`, `get_last()`, or `get()` on potentially empty arrays.

**Common Pitfalls:**
- Do not access `_` prefixed fields directly; use accessor functions.
- Do not free the pointer returned by `get_floats()` - it's borrowed, not owned.
- Do not use `add_guaranteed()` without first calling `guarantee()` - it will corrupt memory.
- Remember that `_advance()` and `_reserve()` move the current pointer; data must be written to returned pointer.
- Do not mix base (`mulle__floatarray`) and wrapper (`mulle_floatarray`) operations on the same struct.
- Be aware that `set()` with out-of-bounds index will grow and zero-initialize intermediate elements.
- Enumerators do not protect against concurrent modifications - ensure array is stable during enumeration.

**Idiomatic Usage:**
- Use `mulle_floatarray_for()` macro for clean, readable iteration.
- Combine `guarantee()` with `add_guaranteed()` for batch additions in loops.
- Use `advance()` for bulk operations like copying from external sources.
- Prefer `copy()` or `add_floatarray()` over manual element-by-element copying.
- Use NULL allocator parameter to use the default allocator in most cases.
- Leverage static storage for stack-allocated temporary arrays in hot paths.

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

   // Create array with initial capacity of 10
   array = mulle_floatarray_create( 10, NULL);

   // Add some values
   for( i = 0; i < 5; i++)
   {
      value = (float) i * 3.14f;
      mulle_floatarray_add( array, value);
   }

   // Access elements
   printf( "First: %.2f\n", mulle_floatarray_get_first( array));
   printf( "Last: %.2f\n", mulle_floatarray_get_last( array));
   printf( "Count: %u\n", mulle_floatarray_get_count( array));

   // Cleanup
   mulle_floatarray_destroy( array);
}
```

### Example 2: Using Static Storage for Temporary Arrays

```c
#include <mulle-fparray/mulle-fparray.h>

float   compute_sum( float *values, unsigned int n)
{
   struct mulle_floatarray   array;
   float                     storage[ 32];  // Stack buffer
   float                     sum;
   float                     value;
   unsigned int              i;

   // Initialize with static storage - no heap allocation
   mulle_floatarray_init_with_static_storage( &array, 
                                               32, 
                                               storage, 
                                               NULL);

   // Process input values
   for( i = 0; i < n; i++)
   {
      value = values[ i] * 2.0f;
      _mulle_floatarray_add( &array, value);
   }

   // Calculate sum
   sum = 0.0f;
   mulle_floatarray_for( &array, value)
   {
      sum += value;
   }

   // Cleanup (frees dynamic memory if static was exceeded)
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

   // Ensure capacity once for all additions
   _mulle_floatarray_guarantee( array, count);

   // Add all values without capacity checks
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

   // Forward enumeration
   printf( "Forward: ");
   rover = mulle_floatarray_enumerate( array);
   while( _mulle_floatarrayenumerator_next( &rover, &value))
      printf( "%.2f ", value);
   mulle_floatarrayenumerator_done( &rover);
   printf( "\n");

   // Reverse enumeration  
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

   // Copy first array entirely
   mulle_floatarray_copy( result, a);

   // Append subset of second array (elements 5-10)
   range = mulle_range_make( 5, 5);
   mulle_floatarray_add_floatarray( result, b, range);

   return( result);
}
```

### Example 6: Using Base API with Custom Allocator

```c
#include <mulle-fparray/mulle-fparray.h>
#include <mulle-allocator/mulle-allocator.h>

void   base_api_example( struct mulle_allocator *allocator)
{
   struct mulle__floatarray   array;
   float                      *ptr;
   unsigned int               i;

   // Initialize base array
   _mulle__floatarray_init( &array, 100, allocator);

   // Reserve space for 10 elements and fill directly
   ptr = _mulle__floatarray_advance( &array, 10, allocator);
   for( i = 0; i < 10; i++)
      ptr[ i] = (float) i * 1.5f;

   printf( "Count: %u\n", _mulle__floatarray_get_count( &array));

   // Cleanup
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

   // Forward iteration with macro
   mulle_floatarray_for( array, value)
   {
      printf( "%.2f\n", value);
   }

   // Reverse iteration with macro
   mulle_floatarray_for_reverse( array, value)
   {
      printf( "%.2f\n", value);
   }
}
```

## 7. Dependencies

The mulle-fparray library directly depends on:

- **mulle-core**: Provides fundamental data structures (`mulle_range`, `mulle_data`), memory management (`mulle_allocator`), and common utilities required by the array implementation. This is the foundational library for the entire mulle ecosystem.
