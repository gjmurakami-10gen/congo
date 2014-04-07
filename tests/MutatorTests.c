#include <limits.h>

#include <Core/Debug.h>
#include <Core/Types.h>
#include <Storage/Mutable.h>

#include "MutatorTests.h"


#pragma pack(push, 1)
typedef struct
{
   int32_t field1;
   char    field2 [PAGE_SIZE - sizeof(int32_t)];
   int32_t field3;
   char    field4; /* throw off alignments */
   int32_t field5;
   struct {
      int32_t a;
      int32_t b;
   } stat;
   int64_t field6;
} TestStruct;
#pragma pack(pop)


static uint8_t gTestBuf [PAGE_SIZE * 3];


static void
Test_Mutable_Basic (void)
{
   Mutable mutable = { 0 };
   Page *pages[3];

   pages [0] = (Page *)&gTestBuf [PAGE_SIZE*0];
   pages [1] = (Page *)&gTestBuf [PAGE_SIZE*1];
   pages [2] = (Page *)&gTestBuf [PAGE_SIZE*2];

   Mutable_Init (&mutable, pages, N_ELEMENTS (pages));

   /*
    * Mutate some fields.
    */
   Mutable_SetInt32 (&mutable, TestStruct, field1, 1234);
   Mutable_SetInt32 (&mutable, TestStruct, field3, 4567);
   Mutable_SetInt32 (&mutable, TestStruct, field5, 4111);
   Mutable_SetInt32 (&mutable, TestStruct, stat.a, 1);
   Mutable_SetInt32 (&mutable, TestStruct, stat.b, 2);
   Mutable_SetInt64 (&mutable, TestStruct, field6, INT64_MAX);

   /*
    * Ensure the mutations worked and can be read back.
    */
   assert (1234 == Mutable_GetInt32 (&mutable, TestStruct, field1));
   assert (4567 == Mutable_GetInt32 (&mutable, TestStruct, field3));
   assert (4111 == Mutable_GetInt32 (&mutable, TestStruct, field5));
   assert (1 == Mutable_GetInt32 (&mutable, TestStruct, stat.a));
   assert (2 == Mutable_GetInt32 (&mutable, TestStruct, stat.b));
   assert (INT64_MAX == Mutable_GetInt64 (&mutable, TestStruct, field6));

   /*
    * Clean up structures.
    */
   Mutable_Destroy (&mutable);
}


void
MutatorTests_Install (TestSuite *suite) /* IN */
{
   TestSuite_Add (suite, "Storage/Mutable/Basic", Test_Mutable_Basic);
}
