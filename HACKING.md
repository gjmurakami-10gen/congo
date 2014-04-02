# Congo

## Prerequisites

If you want to hack on congo you will need the following:

 - GNU autoconf
 - GNU automake
 - GNU libtool
 - C compiler (GCC/Clang/etc)


## Compiling

Once you have checked out the source from GIT, you will need to run autogen.sh
to bootstrap the automake system. This is done by:

```sh
$ ./autogen.sh
```

Running `autogen.sh` will automatically call `configure` with the arguments
supplied, but if you choose, you can call `./configure` manually. To see the
options available, run `./configure --help`.

If you have an x86_64 core i7 or newer system, you might consider using the
`--enable-rdtscp` instruction for faster counters.


## Coding Style

The coding style mimics the VMware coding style with a few readability
improvements.

 - 3 space tabs (vim: set ts=3 sw=3 et).

 - 1 space between function/macro name and open parenthesis.
   "Function ()"

 - 1 space between variable and array index.
   "array [i]"

 - Keep the class libraries CLEAN! If you can't figure out a clean way to
   do something, ask Christian or see how GLib does it. That is one of the
   cleaner C libraries out there.

 - Structs should be Capitalized.
   "MyFoo".

 - Function return type is on it's own line (with specifiers).

 - Function parameters are one per line, and aligned by name with one space
   between type and variable name or asterisk.

 - All function parameters specifiy IN, OUT, or INOUT.

 - All functions should be documented. Internal or Public. Include the return
   type and required free functions as well as any parameter side-effects.

 - Functions are prefixed with their structure name.
   "MyFoo" struct would be "MyFoo_Bar".

An example function is below.


```c
/*
 *--------------------------------------------------------------------------
 *
 * MyFoo_Bar --
 *
 *       Bar the MyFoo instance.
 *
 *       @buf will be baz'd before returning.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @buf will be baz'd.
 *
 *--------------------------------------------------------------------------
 */

static void
MyFoo_Bar (MyFoo *self, /* IN */
           void *buf,   /* OUT */
           size_t size) /* IN */
{
   ASSERT (self);
   ASSERT (buf);
   ASSERT (size);

   MyFoo_Baz (self, buf, size);
}
```
