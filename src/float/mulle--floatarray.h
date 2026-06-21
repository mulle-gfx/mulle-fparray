#ifndef mulle__floatarray_h__
#define mulle__floatarray_h__

#include "include.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>

//
// This is a growing array of struct sized structs.
// It has been coded for a fast "reserve" operation.
//
#define MULLE__FLOATARRAY_BASE   \
   float     *_storage;          \
   float     *_curr;             \
   float     *_sentinel;         \
   float     *_initial_storage  


struct mulle__floatarray
{
   MULLE__FLOATARRAY_BASE;
};


#define MULLE__FLOATARRAY_INIT( storage, type, count) \
   ((struct mulle__floatarray)                        \
   {                                                  \
      storage,                                        \
      storage,                                        \
      &((char *) storage)[ count],                    \
      storage                                         \
   })


static inline struct mulle__floatarray  *
   mulle__floatarray_alloc( struct mulle_allocator *allocator)
{
   struct mulle__floatarray   *array;

   array = mulle_allocator_malloc( allocator, sizeof( struct mulle__floatarray));
   return( array);
}


MULLE_C_NONNULL_FIRST
static inline void   _mulle__floatarray_init( struct mulle__floatarray *array,
                                              unsigned int capacity,
                                              struct mulle_allocator *allocator)
{
   MULLE__FPARRAY_GLOBAL
   void   _mulle__floatarray_sizeto( struct mulle__floatarray *array,
                                     unsigned int new_size,
                                     struct mulle_allocator *allocator);

   array->_storage         = NULL;
   array->_curr            = NULL;
   array->_sentinel        = NULL;
   array->_initial_storage = NULL;

   if( capacity)
      _mulle__floatarray_sizeto( array, capacity, allocator);
}


MULLE_C_NONNULL_FIRST
void   _mulle__floatarray_init_with_floats( struct mulle__floatarray *array,
                                            float *values,
                                            unsigned int n,
                                            struct mulle_allocator *allocator);


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarray_init_with_static_storage( struct mulle__floatarray *array,
                                                unsigned int count,
                                                float  *storage,
                                                struct mulle_allocator *allocator)
{
   array->_storage         = storage;
   array->_curr            = storage;
   array->_sentinel        = &array->_storage[ count];
   array->_initial_storage = storage;
}



static inline struct mulle__floatarray *
   mulle__floatarray_create( unsigned int capacity,
                             struct mulle_allocator *allocator)
{
   struct mulle__floatarray  *array;

   array = mulle__floatarray_alloc( allocator);
   _mulle__floatarray_init( array, capacity, allocator);
   return( array);
}


MULLE_C_NONNULL_FIRST
static inline void  _mulle__floatarray_done( struct mulle__floatarray *array,
                                             struct mulle_allocator *allocator)
{
   if( ! array->_initial_storage)
      mulle_allocator_free( allocator, array->_storage);
#ifdef DEBUG
   memset( array, 0xFD, sizeof( struct mulle__floatarray));
#endif
}



static inline void  mulle__floatarray_destroy( struct mulle__floatarray *array,
                                               struct mulle_allocator *allocator)
{
   if( array)
   {
      _mulle__floatarray_done( array, allocator);
      mulle_allocator_free( allocator, array);
   }
}


MULLE_C_NONNULL_FIRST
static inline void   _mulle__floatarray_reset( struct mulle__floatarray *array)
{
   array->_curr = array->_storage;
}


# pragma mark - petty accessors

// if you crash here, you forgot to initialize the array
MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle__floatarray_get_count( struct mulle__floatarray *array)
{
   return( (unsigned int) (array->_curr - array->_storage));
}


static inline unsigned int
   mulle__floatarray_get_count( struct mulle__floatarray *array)
{
   if( ! array)
      return( 0);
   return( _mulle__floatarray_get_count( array));
}


MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle__floatarray_get_used( struct mulle__floatarray *array)
{
   return( (unsigned int) (array->_curr - array->_storage));
}


MULLE_C_NONNULL_FIRST
static inline size_t
   _mulle__floatarray_get_used_as_length( struct mulle__floatarray *array)
{
   return( (array->_curr - array->_storage) * sizeof( float));
}



MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle__floatarray_get_size( struct mulle__floatarray *array)
{
   return( (unsigned int) (array->_sentinel - array->_storage));
}


MULLE_C_NONNULL_FIRST
static inline size_t
   _mulle__floatarray_get_size_as_length( struct mulle__floatarray *array)
{
   return( (array->_sentinel - array->_storage) * sizeof( float));
}


MULLE_C_NONNULL_FIRST
static inline float *
   _mulle__floatarray_get_floats( struct mulle__floatarray *array)
{
   return( array->_storage);
}


MULLE_C_NONNULL_FIRST
static inline float
   _mulle__floatarray_get_first( struct mulle__floatarray *array)
{
   return( array->_curr > array->_storage ? *array->_storage : NAN);
}


// use in conjunction with guarantee only
MULLE_C_NONNULL_FIRST
static inline float
   _mulle__floatarray_get_current( struct mulle__floatarray *array)
{
   return( *array->_curr);
}


MULLE_C_NONNULL_FIRST
static inline float
   _mulle__floatarray_get( struct mulle__floatarray *array, unsigned int i)
{
   float   *address;

   assert( array);
   address = &array->_storage[ i];
   assert( address < array->_sentinel);

   return( *address);
}


MULLE_C_NONNULL_FIRST
static inline float
   _mulle__floatarray_get_last( struct mulle__floatarray *array)
{
   if( array->_curr == array->_storage)
      return( NAN);

   return( array->_curr[ -1]);
}


static inline void
   _mulle__floatarray_remove_last( struct mulle__floatarray *array)
{
   if( array->_curr > array->_storage)
      --array->_curr;
}


static inline float
   mulle__floatarray_get_last( struct mulle__floatarray *array)
{
   if( ! array)
      return( NAN);
   return( _mulle__floatarray_get_last( array));
}


# pragma mark - array operations

MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarray_add( struct mulle__floatarray *array,
                           float item,
                           struct mulle_allocator *allocator)
{
   MULLE__FPARRAY_GLOBAL
   void   _mulle__floatarray_grow( struct mulle__floatarray *array,
                                   struct mulle_allocator *allocator);

   if( array->_curr == array->_sentinel)
      _mulle__floatarray_grow( array, allocator);

   *array->_curr++ = item;
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarray_add_guaranteed( struct mulle__floatarray *array,
                                      float item)
{
   assert( array->_curr < array->_sentinel);
   *array->_curr++ = item;
}


MULLE__FPARRAY_GLOBAL
MULLE_C_NONNULL_FIRST
float   *_mulle__floatarray_guarantee( struct mulle__floatarray *array,
                                       unsigned int count,
                                       struct mulle_allocator *allocator);


MULLE_C_NONNULL_FIRST
static inline float *
   _mulle__floatarray_advance( struct mulle__floatarray *array,
                                unsigned int count,
                                struct mulle_allocator *allocator)
{
   float   *reserved;

   reserved = _mulle__floatarray_guarantee( array, count, allocator);
   if( reserved)
      array->_curr = &array->_curr[ count];
   return( reserved);
}


MULLE_C_NONNULL_FIRST
MULLE__FPARRAY_GLOBAL
void
   _mulle__floatarray_add_floatarray( struct mulle__floatarray *array,
                                      struct mulle__floatarray *other,
                                      struct mulle_range range,
                                      struct mulle_allocator *allocator);


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarray_copy( struct mulle__floatarray *array,
                            struct mulle__floatarray *other,
                            struct mulle_allocator *allocator)
{
   _mulle__floatarray_reset( array);
   _mulle__floatarray_add_floatarray( array, other, mulle_range_make_all(), allocator);
}


MULLE_C_NONNULL_FIRST
static inline float *
   _mulle__floatarray_reserve( struct mulle__floatarray *array,
                               struct mulle_allocator *allocator)
{
   float   *reserved;

   reserved = _mulle__floatarray_guarantee( array, 1, allocator);
   if( reserved)
      ++array->_curr;
   return( reserved);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarray_size_to_fit( struct mulle__floatarray *array,
                                    struct mulle_allocator *allocator)
{
   MULLE__FPARRAY_GLOBAL
   void   _mulle__floatarray_sizeto( struct mulle__floatarray *array,
                                      unsigned int new_size,
                                      struct mulle_allocator *allocator);

   _mulle__floatarray_sizeto( array, _mulle__floatarray_get_used( array), allocator);
}


MULLE__FPARRAY_GLOBAL
MULLE_C_NONNULL_FIRST
unsigned int   _mulle__floatarray_set_count( struct mulle__floatarray *array,
                                             unsigned int count,
                                             struct mulle_allocator *allocator);

MULLE__FPARRAY_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__floatarray_zero_to_count( struct mulle__floatarray *array,
                                          unsigned int count,
                                          struct mulle_allocator *allocator);



MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarray_set( struct mulle__floatarray *array,
                           unsigned int index,
                           float item,
                           struct mulle_allocator *allocator)
{
   if( &array->_storage[ index] >= array->_sentinel)
      _mulle__floatarray_zero_to_count( array, index + 1, allocator);

   array->_storage[ index] = item;
}



#pragma mark - extract


MULLE_C_NONNULL_FIRST
static inline struct mulle_data
   _mulle__floatarray_extract_data( struct mulle__floatarray *buffer,
                                     struct mulle_allocator *allocator)
{
   struct mulle_data   data;

   data.bytes  = buffer->_storage;
   data.length = _mulle__floatarray_get_used_as_length( buffer);

   buffer->_storage          =
   buffer->_curr             =
   buffer->_sentinel         = NULL;

   return( data);
}



#pragma mark - floatarray enumerator

#define MULLE__FLOATARRAYENUMERATOR_BASE    \
   float     *_curr;                        \
   float     *_sentinel


struct mulle__floatarrayenumerator
{
   MULLE__FLOATARRAYENUMERATOR_BASE;
};

#define mulle__floatarrayenumerator_empty  \
   ((struct mulle__floatarrayenumerator) { 0 })


MULLE_C_NONNULL_FIRST
static inline struct mulle__floatarrayenumerator
   _mulle__floatarray_enumerate( struct mulle__floatarray *array)
{
   struct mulle__floatarrayenumerator   rover;

   rover._curr          = array->_storage;
   rover._sentinel      = array->_curr;

   assert( rover._sentinel >= rover._curr);

   return( rover);
}


static inline struct mulle__floatarrayenumerator
   mulle__floatarray_enumerate( struct mulle__floatarray *array)
{
   if( ! array)
      return( mulle__floatarrayenumerator_empty);
   return( _mulle__floatarray_enumerate( array));
}


MULLE_C_NONNULL_FIRST_SECOND
static inline int
   _mulle__floatarrayenumerator_next( struct mulle__floatarrayenumerator *rover,
                                      float *item)
{
   if( rover->_curr < rover->_sentinel)
   {
      *item        = *rover->_curr;
      rover->_curr++;
      return( 1);
   }
   *item = NAN;
   return( 0);
}

static inline int
   mulle__floatarrayenumerator_next( struct mulle__floatarrayenumerator *rover,
                                     float *item)
{
   if( ! rover || rover->_curr >= rover->_sentinel)
   {
      if( item)
         *item = NAN;
      return( 0);
   }

   if( item)
      *item = *rover->_curr;
   rover->_curr++;
   return( 1);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarrayenumerator_done( struct mulle__floatarrayenumerator *rover)
{
}


static inline void
   mulle__floatarrayenumerator_done( struct mulle__floatarrayenumerator *rover)
{
}


#pragma mark - reverse enumerator

#define MULLE__FLOATARRAYREVERSEENUMERATOR_BASE MULLE__FLOATARRAYENUMERATOR_BASE

struct mulle__floatarrayreverseenumerator
{
   MULLE__FLOATARRAYREVERSEENUMERATOR_BASE;
};


#define mulle__floatarrayreverseenumerator_empty  \
   ((struct mulle__floatarrayreverseenumerator) { 0 })


static inline struct mulle__floatarrayreverseenumerator
   _mulle__floatarray_reverseenumerate( struct mulle__floatarray *array)
{
   struct mulle__floatarrayreverseenumerator   rover;

   rover._curr          = array->_curr;
   rover._sentinel      = array->_storage;

   assert( rover._curr >= rover._sentinel);

   return( rover);
}


static inline struct mulle__floatarrayreverseenumerator
   mulle__floatarray_reverseenumerate( struct mulle__floatarray *array)
{
   if( ! array)
      return( mulle__floatarrayreverseenumerator_empty);
   return( _mulle__floatarray_reverseenumerate( array));
}


MULLE_C_NONNULL_FIRST_SECOND
static inline int
   _mulle__floatarrayreverseenumerator_next( struct mulle__floatarrayreverseenumerator *rover,
                                             float *item)
{
   if( rover->_curr > rover->_sentinel)
   {
      rover->_curr--;
      *item = *rover->_curr;
      return( 1);
   }

   *item = NAN;
   return( 0);
}


static inline int
   mulle__floatarrayreverseenumerator_next( struct mulle__floatarrayreverseenumerator *rover,
                                            float *item)
{
   if( ! rover || rover->_curr <= rover->_sentinel)
   {
      if( item)
         *item = NAN;      
      return( 0);
   }

   rover->_curr--;
   if( item)
      *item = *rover->_curr;
   return( 1);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__floatarrayreverseenumerator_done( struct mulle__floatarrayreverseenumerator *rover)
{
}


static inline void
   mulle__floatarrayreverseenumerator_done( struct mulle__floatarrayreverseenumerator *rover)
{
}


#define mulle__floatarray_for( array, item)                                                       \
   for( struct mulle__floatarrayenumerator rover__ ## item = mulle__floatarray_enumerate( array); \
        _mulle__floatarrayenumerator_next( &rover__ ## item, &item);)

#define mulle__floatarray_for_reverse( array, item)                                                              \
   for( struct mulle_floatarrayreverseenumerator rover__ ## item = mulle__floatarray_reverseenumerate( array); \
        _mulle__floatarrayreverseenumerator_next( &rover__ ## item, &item);)

#endif

