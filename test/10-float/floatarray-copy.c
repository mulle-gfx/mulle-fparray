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



static void  add_array( void)
{
   float                               a;
   float                               *p;
   struct mulle_floatarray             *array;
   struct mulle_floatarray             *other;
   struct mulle_floatarrayenumerator   rover;
   unsigned int                         i;
   void                                 *item;

   array = mulle_floatarray_create( 10, NULL);
   other = mulle_floatarray_create( 10, NULL);
   assert( isnan( mulle_floatarray_get_last( other)));

   for( i = 0; i < 10; i++)
   {
      a = (float) i + 1848;

      _mulle_floatarray_add( other, a);
   }

   mulle_floatarray_add_floatarray( array, other, mulle_range_make( 1, 8));

   i     = 0;
   rover = mulle_floatarray_enumerate( array);
   while( _mulle_floatarrayenumerator_next( &rover, &a))
   {
      printf( "%d: %.0f\n", i, a);
      ++i;
   }
   mulle_floatarrayenumerator_done( &rover);

   mulle_floatarray_destroy( other);
   mulle_floatarray_destroy( array);
}


// the mulle_testallocator detects and aborts on leaks
static void  run_test( void (*f)( void), char *name)
{
   mulle_testallocator_reset();
  // printf( "%s\n", name);
   (f)();
   mulle_testallocator_reset();
}


int main(int argc, const char * argv[])
{
   run_test( add_array, "add array");
   return( 0);
}
