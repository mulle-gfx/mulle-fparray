//
//  main.c
//  test-array
//
//  Created by Nat! on 04.11.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
//

#include <mulle-fparray/mulle-fparray.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>



#define LOOPS   1000

static void  null( void)
{
   float                                     a;
   float                                     *v;
   struct mulle_floatarray                   *array;
   struct mulle_floatarrayenumerator         rover;
   struct mulle_floatarrayreverseenumerator  rrover;
   unsigned int                              i;

   array = NULL;

   assert( isnan( mulle_floatarray_get_last( array)));

   v = mulle_floatarray_reserve( array);
   assert( v == NULL);
   mulle_floatarray_add( array, a);
   assert( isnan( mulle_floatarray_get_last( array)));

   rover = mulle_floatarray_enumerate( array);
   if( _mulle_floatarrayenumerator_next( &rover, &a))
      abort();
   mulle_floatarrayenumerator_done( &rover);

   rrover = mulle_floatarray_reverseenumerate( array);
   if( _mulle_floatarrayreverseenumerator_next( &rrover, &a))
      abort();
   mulle_floatarrayreverseenumerator_done( &rrover);

   mulle_floatarray_destroy( array);
}



static void  simple( void)
{
   float                                     *a;
   float                                     v;
   struct mulle_floatarray                   *array;
   struct mulle_floatarrayenumerator         rover;
   struct mulle_floatarrayreverseenumerator  rrover;
   unsigned int                               i;
   void                                       *item;

   array = mulle_floatarray_create( 10, NULL);

   assert( isnan( mulle_floatarray_get_last( array)));

   for( i = 0; i < LOOPS; i++)
   {
      fprintf( stderr, "%u\n", i);
      if( i < LOOPS / 2)
         a = _mulle_floatarray_reserve( array);
      else
         a = &v;

      *a = (float) i + 1848;
      if( i >= LOOPS / 2)
         _mulle_floatarray_add( array, *a);
   }

   assert( ! isnan( mulle_floatarray_get_last( array)));

   for( i = 0; i < LOOPS; i++)
   {
      v = _mulle_floatarray_get( array, i);

      assert( v == (float) i + 1848);
   }

   i     = 0;
   rover = mulle_floatarray_enumerate( array);
   while( _mulle_floatarrayenumerator_next( &rover, &v))
   {
      assert( v == (float) i + 1848);
      ++i;
   }
   assert( i == LOOPS);
   mulle_floatarrayenumerator_done( &rover);

   // more or less just a syntax check
   i = 0;
   mulle_floatarray_for( array, v)
   {
      assert( ! isnan( v));
      ++i;
   }
   assert( i == LOOPS);

   rrover = mulle_floatarray_reverseenumerate( array);
   while( _mulle_floatarrayreverseenumerator_next( &rrover, &v))
   {
      --i;
      assert( v == (float) i + 1848);
   }
   assert( i == 0);
   mulle_floatarrayreverseenumerator_done( &rrover);

   // more or less just a syntax check
   i = LOOPS;
   mulle_floatarray_for_reverse( array, v)
   {
      assert( ! isnan( v));
      --i;
   }
   assert( i == 0);

   mulle_floatarray_destroy( array);
}


// the mulle_testallocator detects and aborts on leaks
static void  run_test( void (*f)( void), char *name)
{
   mulle_testallocator_reset();
  // mulle_printf( "%s\n", name);
   (f)();
   mulle_testallocator_reset();
}


int main(int argc, const char * argv[])
{
   run_test( null,   "null");
   run_test( simple, "simple");
   return( 0);
}
