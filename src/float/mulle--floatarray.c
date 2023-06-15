#include "mulle--floatarray.h"


void   _mulle__floatarray_sizeto_length( struct mulle__floatarray *array,
                                         size_t new_size,
                                         struct mulle_allocator *allocator)
{
   size_t   old_size;

   old_size = _mulle__floatarray_get_used_as_length( array);
   if( new_size < old_size)
      abort();

   // if we have an initial storage
   if( array->_initial_storage)
   {
      // and we would still fit, ignore request
      if( new_size <= _mulle__floatarray_get_size_as_length( array))
         return;

      // otherwise we need to malloc, now
      array->_storage  = mulle_allocator_malloc( allocator, new_size);
      memcpy( array->_storage,
              array->_initial_storage,
              old_size);

      array->_initial_storage = NULL;
   }
   else
      array->_storage  = mulle_allocator_realloc( allocator,
                                                  array->_storage,
                                                  new_size);

   array->_curr     = (float *) &((char *) array->_storage)[ old_size];
   array->_sentinel = (float *) &((char *) array->_storage)[ new_size];

   assert( array->_storage <= array->_curr);
   assert( array->_curr    <= array->_sentinel);
}


// Should also be usable for "_size to fit"
void   _mulle__floatarray_sizeto( struct mulle__floatarray *array,
                                  unsigned int new_count,
                                  struct mulle_allocator *allocator)
{
   _mulle__floatarray_sizeto_length( array, new_count * sizeof( float), allocator);
}


void   _mulle__floatarray_grow( struct mulle__floatarray *array,
                                struct mulle_allocator *allocator)
{
   size_t   new_size;
   size_t   old_size;

   old_size = _mulle__floatarray_get_size_as_length( array);
   new_size = (old_size ? old_size : sizeof( float)) * 2;
   _mulle__floatarray_sizeto_length( array, new_size, allocator);
}


float *  _mulle__floatarray_guarantee( struct mulle__floatarray *array,
                                       unsigned int length,
                                       struct mulle_allocator *allocator)
{
   unsigned int   available;
   unsigned int   _size;
   unsigned int   _used;
   size_t         new_size;
   size_t         needed_size;
   size_t         old_size;

   _size     = _mulle__floatarray_get_size( array);
   _used     = _mulle__floatarray_get_count( array);
   available = _size - _used;
   if( available < length)
   {
      // grow by default, if exceeds growth sizeto
      old_size    = _mulle__floatarray_get_size_as_length( array);
      new_size    = (old_size ? old_size : sizeof( float)) * 2;
      needed_size = (_size + (length - available)) * sizeof( float);
      if( needed_size > new_size)
         new_size = needed_size;
      _mulle__floatarray_sizeto_length( array, new_size, allocator);
   }
   return( array->_curr);
}


unsigned int  _mulle__floatarray_set_count( struct mulle__floatarray *array,
                                            unsigned int count,
                                            struct mulle_allocator *allocator)
{
   long   diff;

   diff = (long) count - (long) _mulle__floatarray_get_count( array);
   if( diff <= 0)
   {
      array->_curr = &array->_curr[ diff];
      _mulle__floatarray_size_to_fit( array, allocator);
      return( 0);
   }

   _mulle__floatarray_advance( array, (unsigned int) diff, allocator);
   return( (unsigned int) diff);
}



void   _mulle__floatarray_zero_to_count( struct mulle__floatarray *array,
                                         unsigned int count,
                                         struct mulle_allocator *allocator)
{
   long   diff;

   diff = _mulle__floatarray_set_count( array, count, allocator);
   memset( &array->_curr[ -diff], 0, (unsigned int) diff * sizeof( float));
}


void
   _mulle__floatarray_add_array( struct mulle__floatarray *array,
                                 struct mulle__floatarray *other,
                                 struct mulle_range range,
                                 struct mulle_allocator *allocator)
{
   unsigned int   count;
   float          *buf;

   count = mulle__floatarray_get_count( other);
   range = mulle_range_validate_against_length( range, count);
   if( ! range.length)
      return;

   buf  = _mulle__floatarray_advance( (struct mulle__floatarray *) array, 
                                       range.length, 
                                       allocator);
   memcpy( buf, 
           &other->_storage[ range.location], 
           range.length * sizeof( float));
}


void   _mulle__floatarray_init_with_floats( struct mulle__floatarray *array,
                                            float *values,
                                            unsigned int n,
                                            struct mulle_allocator *allocator)
{
   assert( ! n || values);

   _mulle__floatarray_init( array, n, allocator);
   memcpy( array->_storage, values, n * sizeof( float));
   array->_curr = &array->_storage[ n];
}
