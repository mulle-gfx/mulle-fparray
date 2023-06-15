#ifndef mulle_floatarray_h__
#define mulle_floatarray_h__

#include "mulle--floatarray.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>



#pragma mark - item array

//
// This is a growing array of struct sized structs.
// It has been coded for a fast "reserve" operation.
//
struct mulle_floatarray
{
   MULLE__FLOATARRAY_BASE;
   struct mulle_allocator   *allocator;
};


#define MULLE_FLOATARRAY_INIT( storage, count, allocator) \
   ((struct mulle_floatarray)                             \
   {                                                      \
      storage,                                            \
      storage,                                            \
      &((char *) storage)[ count],                        \
      storage,                                            \
      allocator                                           \
   })


static inline struct mulle_floatarray  *
   mulle_floatarray_alloc( struct mulle_allocator *allocator)
{
   struct mulle_floatarray   *array;

   array = mulle_allocator_malloc( allocator, sizeof( struct mulle_floatarray));
   return( array);
}


//
// it's assumed that "malloc" always returns memory that is properly aligned
// for any given float
//
MULLE_C_NONNULL_FIRST
static inline void   _mulle_floatarray_init( struct mulle_floatarray *array,
                                              unsigned int capacity,
                                              struct mulle_allocator *allocator)
{
   _mulle__floatarray_init( (struct mulle__floatarray *) array,
                             capacity,
                             allocator);

   array->allocator = allocator;
}

// TODO: simplify to..
// #define mulle_floatarray_init( array, capacity, allocator) \
// do                                                                \
// {                                                                 \
//    if( array)                                                     \
//       _mulle_floatarray_init( array,                             \
//                                capacity,                          \
//                                allocator);                        \
// }                                                                 \
// while( 0)

static inline void   mulle_floatarray_init( struct mulle_floatarray *array,
                                             unsigned int capacity,
                                             struct mulle_allocator *allocator)
{
   if( array)
      _mulle_floatarray_init( array,
                               capacity,
                               allocator);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarray_init_with_static_storage( struct mulle_floatarray *array,
                                                unsigned int count,
                                                void  *storage,
                                                struct mulle_allocator *allocator)
{
   _mulle__floatarray_init_with_static_storage( (struct mulle__floatarray *) array,
                                                  count,
                                                  storage,
                                                  allocator);

   array->allocator = allocator;
}


static inline void
   mulle_floatarray_init_with_static_storage( struct mulle_floatarray *array,
                                               unsigned int count,
                                               void  *storage,
                                               struct mulle_allocator *allocator)
{
   if(  array)
   {
      _mulle__floatarray_init_with_static_storage( (struct mulle__floatarray *) array,
                                                    count,
                                                    storage, //  NULL storage is OK, so useless
                                                    allocator);
      array->allocator = allocator;
   }
}


static inline struct mulle_floatarray *
   mulle_floatarray_create( unsigned int capacity,
                            struct mulle_allocator *allocator)
{
   struct mulle_floatarray  *array;

   array = mulle_floatarray_alloc( allocator);
   _mulle_floatarray_init( array, capacity, allocator);
   return( array);
}


MULLE_C_NONNULL_FIRST
static inline void  _mulle_floatarray_done( struct mulle_floatarray *array)
{
   _mulle__floatarray_done( (struct mulle__floatarray *) array, array->allocator);
}


static inline void  mulle_floatarray_done( struct mulle_floatarray *array)
{
   if( array)
      _mulle__floatarray_done( (struct mulle__floatarray *) array, array->allocator);
}


static inline void  mulle_floatarray_destroy( struct mulle_floatarray *array)
{
   struct mulle_allocator   *allocator;

   if( array)
   {
      allocator = array->allocator;
      _mulle_floatarray_done( array);
      mulle_allocator_free( allocator, array);
   }
}


MULLE_C_NONNULL_FIRST
static inline void   _mulle_floatarray_reset( struct mulle_floatarray *array)
{
   _mulle__floatarray_reset( (struct mulle__floatarray *) array);
}


static inline void   mulle_floatarray_reset( struct mulle_floatarray *array)
{
   if( array)
      _mulle__floatarray_reset( (struct mulle__floatarray *) array);
}


# pragma mark - petty accessors

MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle_floatarray_get_count( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_count( (struct mulle__floatarray *) array));
}


static inline unsigned int
   mulle_floatarray_get_count( struct mulle_floatarray *array)
{
   if( ! array)
      return( 0);
   return( _mulle_floatarray_get_count( array));
}


// cheaper as we don't divide here
MULLE_C_NONNULL_FIRST
static inline size_t
   _mulle_floatarray_get_used_as_length( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_used_as_length( (struct mulle__floatarray *) array));
}


MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle_floatarray_get_size( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_size( (struct mulle__floatarray *) array));
}


static inline unsigned int
   mulle_floatarray_get_size( struct mulle_floatarray *array)
{
   if( ! array)
      return( 0);

   return( _mulle__floatarray_get_size( (struct mulle__floatarray *) array));
}


MULLE_C_NONNULL_FIRST
static inline size_t
   _mulle_floatarray_get_size_as_length( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_size_as_length( (struct mulle__floatarray *) array));
}


static inline size_t
   mulle_floatarray_get_size_as_length( struct mulle_floatarray *array)
{
   if( ! array)
      return( 0);

   return( _mulle__floatarray_get_size_as_length( (struct mulle__floatarray *) array));
}



# pragma mark - array operations

MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarray_add( struct mulle_floatarray *array, float item)
{
   _mulle__floatarray_add( (struct mulle__floatarray *) array,
                           item,
                           array->allocator);
}


static inline void
   mulle_floatarray_add( struct mulle_floatarray *array, float item)
{
   if( array && item)
      _mulle_floatarray_add( array, item);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarray_add_guaranteed( struct mulle_floatarray *array,
                                      float item)
{
   _mulle__floatarray_add_guaranteed((struct mulle__floatarray *) array, item);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarray_set( struct mulle_floatarray *array,
                          unsigned int index,
                          float item)
{
   _mulle__floatarray_set( (struct mulle__floatarray *) array, 
                           index,
                           item,
                           array->allocator);
}


static inline void
   mulle_floatarray_set( struct mulle_floatarray *array,
                          unsigned int index,
                          float item)
{
   if( array)
      _mulle_floatarray_set( array, index, item);
}


// guarantee ensures that you can add up to length items without a realloc
MULLE_C_NONNULL_FIRST
static inline float   *_mulle_floatarray_guarantee( struct mulle_floatarray *array,
                                                    unsigned int count)
{
   return( _mulle__floatarray_guarantee( (struct mulle__floatarray *) array,
                                          count,
                                          array->allocator));
}


// reserve, returns storage for multiple items, but it will be already counted
// as used. copy data in and your good
MULLE_C_NONNULL_FIRST
static inline float *
   _mulle_floatarray_advance( struct mulle_floatarray *array,
                              unsigned int count)
{
   return( _mulle__floatarray_advance( (struct mulle__floatarray *) array,
                                       count,
                                       array->allocator));
}


static inline void
   _mulle_floatarray_add_array( struct mulle_floatarray *array,
                                struct mulle_floatarray *other,
                                struct mulle_range range)
{
   _mulle__floatarray_add_array( (struct mulle__floatarray *) array,
                                 (struct mulle__floatarray *) other,
                                 range,
                                 array->allocator);
}


static inline void
   mulle_floatarray_add_array( struct mulle_floatarray *array,
                               struct mulle_floatarray *other,
                               struct mulle_range range)
{
   if( array)
      _mulle__floatarray_add_array( (struct mulle__floatarray *) array,
                                    (struct mulle__floatarray *) other,
                                    range,
                                    array->allocator);
}


static inline void
   mulle_floatarray_copy( struct mulle_floatarray *array,
                          struct mulle_floatarray *other)
{
   if( ! array)
      return;

   _mulle_floatarray_reset( array);
   _mulle_floatarray_add_array( array,
                                other,
                                mulle_range_make_all());
}

// _mulle_floatarray_reserve is like _mulle_floatarray_advance but just for
// one item
MULLE_C_NONNULL_FIRST
static inline float *
   _mulle_floatarray_reserve( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_reserve( (struct mulle__floatarray *) array,
                                        array->allocator));
}


static inline float *
   mulle_floatarray_reserve( struct mulle_floatarray *array)
{
   if( ! array)
      return( NULL);
   return( _mulle__floatarray_reserve( (struct mulle__floatarray *) array,
                                        array->allocator));
}


MULLE_C_NONNULL_FIRST
static inline void   _mulle_floatarray_set_count( struct mulle_floatarray *array,
                                                  size_t count)
{
   _mulle__floatarray_set_count( (struct mulle__floatarray *) array,
                                  count,
                                  array->allocator);
}


MULLE_C_NONNULL_FIRST
static inline void   _mulle_floatarray_zero_to_count( struct mulle_floatarray *array,
                                                      size_t count)
{
   _mulle__floatarray_zero_to_count( (struct mulle__floatarray *) array,
                                      count,
                                      array->allocator);
}



#pragma mark - petty accessors

MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator  *
   _mulle_floatarray_get_allocator( struct mulle_floatarray *array)
{
   return( array->allocator);
}


static inline struct mulle_allocator  *
   mulle_floatarray_get_allocator( struct mulle_floatarray *array)
{
   return( array ? array->allocator : NULL);
}


// use in conjunction with guarantee only
MULLE_C_NONNULL_FIRST
static inline float
   _mulle_floatarray_get_current( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_current( (struct mulle__floatarray *) array));
}


static inline float
   mulle_floatarray_get_current( struct mulle_floatarray *array)
{
   return( array ? _mulle__floatarray_get_current( (struct mulle__floatarray *) array) : NAN);
}


MULLE_C_NONNULL_FIRST
static inline float *
   _mulle_floatarray_get_floats( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_floats( (struct mulle__floatarray *) array));
}



MULLE_C_NONNULL_FIRST
static inline float
   _mulle_floatarray_get_first( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_first( (struct mulle__floatarray *) array));
}


static inline float
   mulle_floatarray_get_first( struct mulle_floatarray *array)
{
   return( array  ? _mulle__floatarray_get_first( (struct mulle__floatarray *) array) : NAN);
}



// use get to set as well
MULLE_C_NONNULL_FIRST
static inline float
   _mulle_floatarray_get( struct mulle_floatarray *array, unsigned int i)
{
   return( _mulle__floatarray_get( (struct mulle__floatarray *) array, i));
}


static inline float
   mulle_floatarray_get( struct mulle_floatarray *array, unsigned int i)
{
   if( array)
      return( _mulle_floatarray_get( array, i));
   return( NAN);
}


MULLE_C_NONNULL_FIRST
static inline float
   _mulle_floatarray_get_last( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_get_last( (struct mulle__floatarray *) array));
}


static inline float
   mulle_floatarray_get_last( struct mulle_floatarray *array)
{
   if( array)
      return( _mulle_floatarray_get_last( array));
    return( NAN);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarray_remove_last( struct mulle_floatarray *array)
{
   _mulle__floatarray_remove_last( (struct mulle__floatarray *) array);
}


static inline void
   mulle_floatarray_remove_last( struct mulle_floatarray *array)
{
   if( array)
      _mulle_floatarray_remove_last( array);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarray_size_to_fit( struct mulle_floatarray *array)
{
   return( _mulle__floatarray_size_to_fit( (struct mulle__floatarray *) array,
                                            array->allocator));
}


#pragma mark - floatarray enumerator

#define MULLE_FLOATARRAYENUMERATOR_BASE   MULLE__FLOATARRAYENUMERATOR_BASE


struct mulle_floatarrayenumerator
{
   MULLE_FLOATARRAYENUMERATOR_BASE;
};


MULLE_C_NONNULL_FIRST
static inline struct mulle_floatarrayenumerator
   _mulle_floatarray_enumerate( struct mulle_floatarray *array)
{
   struct mulle_floatarrayenumerator    rover;
   struct mulle__floatarrayenumerator   tmp;

   tmp = _mulle__floatarray_enumerate( (struct mulle__floatarray *) array);
   memcpy( &rover, &tmp, sizeof( struct mulle__floatarrayenumerator));
   return( rover);
}


static inline struct mulle_floatarrayenumerator
   mulle_floatarray_enumerate( struct mulle_floatarray *array)
{
   struct mulle_floatarrayenumerator    rover;
   struct mulle__floatarrayenumerator   tmp;

   tmp = mulle__floatarray_enumerate( (struct mulle__floatarray *) array);
   memcpy( &rover, &tmp, sizeof( struct mulle__floatarrayenumerator));
   return( rover);
}


MULLE_C_NONNULL_FIRST
static inline int
   _mulle_floatarrayenumerator_next( struct mulle_floatarrayenumerator *rover,
                                     float *item)
{
   return( _mulle__floatarrayenumerator_next( (struct mulle__floatarrayenumerator *) rover,
                                               item));
}


static inline int
   mulle_floatarrayenumerator_next( struct mulle_floatarrayenumerator *rover,
                                    float *item)
{
   return( mulle__floatarrayenumerator_next( (struct mulle__floatarrayenumerator *) rover,
                                               item));
}



MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarrayenumerator_done( struct mulle_floatarrayenumerator *rover)
{
}


static inline void
   mulle_floatarrayenumerator_done( struct mulle_floatarrayenumerator *rover)
{
}



#pragma mark - floatarray enumerator

#define MULLE_FLOATARRAYREVERSEENUMERATOR_BASE   MULLE__FLOATARRAYREVERSEENUMERATOR_BASE

struct mulle_floatarrayreverseenumerator
{
   MULLE_FLOATARRAYENUMERATOR_BASE;
};


MULLE_C_NONNULL_FIRST
static inline struct mulle_floatarrayreverseenumerator
   _mulle_floatarray_reverseenumerate( struct mulle_floatarray *array)
{
   struct mulle_floatarrayreverseenumerator    rover;
   struct mulle__floatarrayreverseenumerator   tmp;

   tmp = _mulle__floatarray_reverseenumerate( (struct mulle__floatarray *) array);
   memcpy( &rover, &tmp, sizeof( struct mulle__floatarrayreverseenumerator));
   return( rover);
}


static inline struct mulle_floatarrayreverseenumerator
   mulle_floatarray_reverseenumerate( struct mulle_floatarray *array)
{
   struct mulle_floatarrayreverseenumerator    rover;
   struct mulle__floatarrayreverseenumerator   tmp;

   tmp = mulle__floatarray_reverseenumerate( (struct mulle__floatarray *) array);
   memcpy( &rover, &tmp, sizeof( struct mulle__floatarrayreverseenumerator));
   return( rover);
}

MULLE_C_NONNULL_FIRST_SECOND
static inline int
   _mulle_floatarrayreverseenumerator_next( struct mulle_floatarrayreverseenumerator *rover,
                                            float *item)
{
   return( _mulle__floatarrayreverseenumerator_next( (struct mulle__floatarrayreverseenumerator *) rover,
                                                      item));
}


static inline int
   mulle_floatarrayreverseenumerator_next( struct mulle_floatarrayreverseenumerator *rover,
                                           float *item)
{
   return( mulle__floatarrayreverseenumerator_next( (struct mulle__floatarrayreverseenumerator *) rover,
                                                     item));
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_floatarrayreverseenumerator_done( struct mulle_floatarrayreverseenumerator *rover)
{
}


static inline void
   mulle_floatarrayreverseenumerator_done( struct mulle_floatarrayreverseenumerator *rover)
{
}


#define mulle_floatarray_for( array, item)                                                      \
   for( struct mulle_floatarrayenumerator rover__ ## item = mulle_floatarray_enumerate( array); \
        _mulle_floatarrayenumerator_next( &rover__ ## item, &item);)

#define mulle_floatarray_for_reverse( array, item)                                                            \
   for( struct mulle_floatarrayreverseenumerator rover__ ## item = mulle_floatarray_reverseenumerate( array); \
        _mulle_floatarrayreverseenumerator_next( &rover__ ## item, &item);)


#endif

