#include <limits.h>
#include <string.h>

#include <Array.h>
#include <Atomic.h>
#include <BlockingQueue.h>
#include <CString.h>
#include <Counter.h>
#include <Debug.h>
#include <Endian.h>
#include <File.h>
#include <Heap.h>
#include <Path.h>
#include <Sched.h>
#include <Task.h>
#include <TestSuite.h>
#include <TimeSpec.h>
#include <Tunable.h>
#include <Value.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"

COUNTER (MyCounter01, "General", "MyCounter01", "A test counter")
COUNTER (MyCounter02, "General", "MyCounter02", "A test counter")
COUNTER (MyCounter03, "General", "MyCounter03", "A test counter")
COUNTER (MyCounter04, "General", "MyCounter04", "A test counter")
COUNTER (MyCounter05, "General", "MyCounter05", "A test counter")
COUNTER (MyCounter06, "General", "MyCounter06", "A test counter")
COUNTER (MyCounter07, "General", "MyCounter07", "A test counter")
COUNTER (MyCounter08, "General", "MyCounter08", "A test counter")
COUNTER (MyCounter09, "General", "MyCounter09", "A test counter")
COUNTER (MyCounter010, "General", "MyCounter010", "A test counter")

static void
Test_Core_Counters_Basic (void)
{
   int i;

   Counters_Init ();

   assert (Platform_GetCpuCount () > 0);

   assert (__MyCounter01.values);
   assert (__MyCounter02.values);
   assert (__MyCounter03.values);
   assert (__MyCounter04.values);
   assert (__MyCounter05.values);
   assert (__MyCounter06.values);
   assert (__MyCounter07.values);
   assert (__MyCounter08.values);
   assert (__MyCounter09.values);
   assert (__MyCounter010.values);

   assert (((uint8_t*)__MyCounter01.values + 8) == (void *)__MyCounter02.values);
   assert (((uint8_t*)__MyCounter02.values + 8) == (void *)__MyCounter03.values);
   assert (((uint8_t*)__MyCounter03.values + 8) == (void *)__MyCounter04.values);
   assert (((uint8_t*)__MyCounter04.values + 8) == (void *)__MyCounter05.values);
   assert (((uint8_t*)__MyCounter05.values + 8) == (void *)__MyCounter06.values);
   assert (((uint8_t*)__MyCounter06.values + 8) == (void *)__MyCounter07.values);
   assert (((uint8_t*)__MyCounter07.values + 8) == (void *)__MyCounter08.values);
   assert (((uint8_t*)__MyCounter01.values + 64) == (void *)__MyCounter09.values);

   assert (0 == MyCounter01_Get ());
   MyCounter01_Increment ();
   assert (1 == MyCounter01_Get ());
   MyCounter01_Increment ();
   assert (2 == MyCounter01_Get ());

   MyCounter01_Decrement ();
   assert (1 == MyCounter01_Get ());
   MyCounter01_Decrement ();
   assert (0 == MyCounter01_Get ());

   for (i = 1; i <= 1000; i++) {
      MyCounter01_Increment ();
      assert (i == MyCounter01_Get ());
   }

   for (i = 999; i >= 0; i--) {
      MyCounter01_Decrement ();
      assert (i == MyCounter01_Get ());
   }
}

static void
Test_Core_Endian_Basic (void)
{
   char a[] = { 0, 0, 0, 12 };
   int i;

   memcpy (&i, a, sizeof i);

   i = UINT32_FROM_BE (i);

   ASSERT (i == 12);
}

static void
Test_Core_CString_Basic (void)
{
   assert (CString_HasPrefix ("abcd", "a"));
   assert (CString_HasPrefix ("abcd", "ab"));
   assert (CString_HasPrefix ("abcd", "abc"));
   assert (CString_HasPrefix ("abcd", "abcd"));
   assert (!CString_HasPrefix ("abcd", "d"));

   assert (CString_HasSuffix ("abcd", "d"));
   assert (CString_HasSuffix ("abcd", "cd"));
   assert (CString_HasSuffix ("abcd", "bcd"));
   assert (CString_HasSuffix ("abcd", "abcd"));
   assert (!CString_HasSuffix ("abcd", "a"));
}

static void
Test_Core_Path_Basic (void)
{
   Path path;
   Path tmp;
   int i;

   Path_Build (&path, "/", "usr", "local", "share", "man", NULL);
   assert (0 == strcmp (path.str, "/usr/local/share/man"));

   for (i = 0; i < 4; i++) {
      assert (true == Path_Parent (&path, &tmp));
      Path_Copy (&tmp, &path);
   }

   assert (false == Path_Parent (&path, &tmp));
}

static void
Test_Core_Platform_Basic (void)
{
   const char *tmpdir;
   int i;

   tmpdir = Platform_GetTempDir ();
   assert (tmpdir);

   i = Platform_GetCpuCount ();
   assert (i > 0);

   i = Platform_GetPageSize ();
   assert ((i % 4096) == 0);
}

static int
Int_Compare (const void *a,
             const void *b)
{
   return *(const int *)a - *(const int *)b;
}

static void
Test_Core_Array_Basic (void)
{
   const int *ptr;
   Array ar;
   int key;
   int i;

   Array_Init (&ar, sizeof(int), false);

   for (i = 0; i < 10000; i++)
      Array_Append (&ar, i);
   assert (ar.len == 10000);

   key = 123;
   ptr = Array_Search (&ar, &key, Int_Compare);
   assert (ptr);
   assert (*ptr == key);

   for (i = 0; i < 100; i++)
      Array_Remove (&ar, 123);

   for (i = 123; i < 223; i++) {
      ptr = Array_Search (&ar, &i, Int_Compare);
      assert (!ptr);
   }

   i = 224;
   ptr = Array_Search (&ar, &i, Int_Compare);
   assert (ptr);
   assert (*ptr == 224);

   Array_Destroy (&ar);
}


static void
Test_Core_Atomic_Basic (void)
{
   int64_t i64 = 0;
   int i;

   AtomicInt_Set (&i, 0);
   assert (0 == AtomicInt_Get (&i));

   AtomicInt_Set (&i, 10);
   assert (10 == AtomicInt_Get (&i));

   AtomicInt_Increment (&i);
   assert (11 == AtomicInt_Get (&i));

   AtomicInt64_Increment (&i64);
   assert (1 == AtomicInt64_Get (&i64));

   AtomicInt64_Decrement (&i64);
   assert (0 == AtomicInt64_Get (&i64));
}


static void
Test_Core_BlockingQueue_Basic (void)
{
   BlockingQueue q;
   int i;

   BlockingQueue_Init (&q, 32);

   for (i = 0; i < 32; i++) {
      BlockingQueue_Push (&q, (void *)(size_t)i);
   }

   assert (q.head == 0);
   assert (q.count == 32);

   for (i = 0; i < 32; i++) {
      assert (i == (int)(size_t)BlockingQueue_Pop (&q));
   }

   assert (q.head == 0);
   assert (q.count == 0);

   BlockingQueue_Destroy (&q);
}


static void
Test_Core_File_Zero_Task (void *data)
{
   const size_t size = (1024 * 1024);
   struct stat st;
   char command [256];
   File fd;
   Path path;
   bool ret;
   Error error;

   Path_Build (&path, "testdata", "Test_IO_File_Zero", NULL);
   if (Path_IsDir (&path)) {
      snprintf (command, sizeof command, "rm -rf \"%s\"", path.str);
      assert (0 == system (command));
   }
   assert (Path_MkdirWithParents (&path, 0750));

   /*
    * Test file filled with zeroes.
    */
   Path_Build (&path, "testdata", "Test_IO_File_Zero", "test-filled.dat", NULL);
   if (!Path_IsCopyOnWrite (&path)) {
      fd = Path_Open (&path, O_RDWR | O_CREAT, 0640);
      assert (fd != FILE_INVALID);
      ret = File_ZeroAllocate (fd, size, false, 0);
      ASSERT_TRUE_WITH_ERROR (ret, &error);
      assert (Path_Stat (&path, &st));
      assert (st.st_size == size);
      assert (st.st_blocks == (size / 512));
      assert (File_Close (fd));
   }

   /*
    * Test sparse file of zeroes.
    */
   Path_Build (&path, "testdata", "Test_IO_File_Zero", "test-sparse.dat", NULL);
   fd = Path_Open (&path, O_RDWR | O_CREAT, 0640);
   assert (fd != FILE_INVALID);
   ret = File_ZeroAllocate (fd, size, true, 0);
   ASSERT_TRUE_WITH_ERROR (ret, &error);
   assert (Path_Stat (&path, &st));
   assert (st.st_size == size);
#if !defined(PLATFORM_APPLE)
   assert ((st.st_blocks == 0) || (st.st_blocks == 1));
#endif
   assert (File_Close (fd));
}


static void
Test_Core_File_Zero (void)
{
   Task task;

   Task_Create (&task, Test_Core_File_Zero_Task, NULL);
   Sched_Run ();
}


static void
Test_Core_Value_Basic (void)
{
   Value value;

   Value_InitInt16 (&value, 1234);
   assert (Value_GetInt16 (&value) == 1234);

   Value_InitInt32 (&value, 1234);
   assert (Value_GetInt32 (&value) == 1234);

   Value_InitInt64 (&value, 1234);
   assert (Value_GetInt64 (&value) == 1234);

   Value_InitDouble (&value, 123.45);
   assert (Value_GetDouble (&value) == 123.45);
}


static void
Test_Core_Tunable_Basic (void)
{
   Tunable t1;
   Tunable t2;
   Tunable t3;
   Tunable t4;
   Value dummy;

   Value_InitSize (&dummy, 10);

   t1 = Tunable_Register ("a.b.c.d", &dummy);
   t2 = Tunable_Register ("a.b.c.e", &dummy);
   t3 = Tunable_Register ("a.f.c.e", &dummy);
   t4 = Tunable_Register ("z.f.c.e", &dummy);

   assert (t3 == Tunable_Find ("a.f.c.e"));
   Tunable_Get (t3, &dummy);
   assert (10 == Value_GetSize (&dummy));

   (void)t1;
   (void)t2;
   (void)t3;
   (void)t4;
}


typedef struct
{
   char c;
   int a;
   void *f1;
   void *f2;
   void *f3;
   void *f4;
} TestAlignof;


static void
Test_Core_alignof (void)
{
   ASSERT (ALIGNOF(TestAlignof) == sizeof(void*));
}


typedef struct
{
   int a;
   void *b;
} HeapTest;


static __inline__ int
HeapTest_Compare (const HeapTest *a, const HeapTest *b)
{
   return b->a - a->a;
}


HEAP_DEFINE (MyHeap, HeapTest, HeapTest_Compare)


static void
Test_Core_Heap (void)
{
   HeapTest ht;
   MyHeap heap;

   MyHeap_Init (&heap);

   ht.a = 123;
   ht.b = NULL;
   MyHeap_Insert (&heap, &ht);
   assert (MyHeap_Size (&heap) == 1);

   ht.a = 124;
   ht.b = NULL;
   MyHeap_Insert (&heap, &ht);
   assert (MyHeap_Size (&heap) == 2);

   MyHeap_Extract (&heap, &ht);
   assert (ht.a == 123);
   assert (MyHeap_Size (&heap) == 1);

   MyHeap_Extract (&heap, &ht);
   assert (ht.a == 124);
   assert (MyHeap_Size (&heap) == 0);

   MyHeap_Destroy (&heap);
}


#pragma GCC diagnostic pop


void
CoreTests_Install (TestSuite *suite) /* IN */
{
   TestSuite_Add (suite, "Core/Array/Basic", Test_Core_Array_Basic);
   TestSuite_Add (suite, "Core/Atomic/Basic", Test_Core_Atomic_Basic);
   TestSuite_Add (suite, "Core/BlockingQueue/Basic", Test_Core_BlockingQueue_Basic);
   TestSuite_Add (suite, "Core/Counters/Basic", Test_Core_Counters_Basic);
   TestSuite_Add (suite, "Core/CString/Basic", Test_Core_CString_Basic);
   TestSuite_Add (suite, "Core/Endian/Basic", Test_Core_Endian_Basic);
   TestSuite_Add (suite, "Core/Path/Basic", Test_Core_Path_Basic);
   TestSuite_Add (suite, "Core/Platform/Basic", Test_Core_Platform_Basic);
   TestSuite_Add (suite, "Core/File/Zero", Test_Core_File_Zero);
   TestSuite_Add (suite, "Core/Tunable/Basic", Test_Core_Tunable_Basic);
   TestSuite_Add (suite, "Core/Value/Basic", Test_Core_Value_Basic);
   TestSuite_Add (suite, "Core/alignof", Test_Core_alignof);
   TestSuite_Add (suite, "Core/Heap", Test_Core_Heap);
}
