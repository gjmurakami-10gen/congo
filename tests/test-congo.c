#include <Counters/Counter.h>

#include "CoreTests.h"
#include "MutatorTests.h"


int
main (int argc,      /* IN */
      char *argv[])  /* IN */
{
   TestSuite suite;
   int ret;

   Counters_Init ();

   TestSuite_Init (&suite, "/", argc, argv);

   CoreTests_Install (&suite);
   MutatorTests_Install (&suite);

   ret = TestSuite_Run (&suite);
   TestSuite_Destroy (&suite);

   return ret;
}
