#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "SINGLETON",
  .unpacker = NULL,
  .packer = NULL,
  .copier = dont_copy,
  .destructor = dont_cleanup
},
#else
#ifndef INCLUDE_EFD_SINGLETON_H
#define INCLUDE_EFD_SINGLETON_H
// efd_singleton.h
// Singleton objects that are malloc'd and freed outside of EFD's purview.
// Singleton objects can't be packed or unpacked, and "copying" them just
// transfers the pointer.

#endif // INCLUDE_EFD_SINGLETON_H
#endif // EFD_REGISTRATION
