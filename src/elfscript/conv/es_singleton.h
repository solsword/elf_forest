#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "SINGLETON",
  .unpacker = NULL,
  .packer = NULL,
  .copier = dont_copy,
  .destructor = dont_cleanup
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_SINGLETON_H
#define INCLUDE_ELFSCRIPT_CONV_SINGLETON_H
// elfscript_singleton.h
// Singleton objects that are malloc'd and freed outside of ELFSCRIPT's purview.
// Singleton objects can't be packed or unpacked, and "copying" them just
// transfers the pointer.

#endif // INCLUDE_ELFSCRIPT_CONV_SINGLETON_H
#endif // ELFSCRIPT_REGISTRATION
