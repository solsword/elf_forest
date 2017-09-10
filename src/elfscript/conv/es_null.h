#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "NULL",
  .unpacker = elfscript__null,
  .packer = null__elfscript,
  .copier = dont_copy,
  .destructor = dont_cleanup
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_NULL_H
#define INCLUDE_ELFSCRIPT_CONV_NULL_H
// elfscript_null.h
// Creates NULL pointers

#include "elfscript/elfscript.h"

void* elfscript__null(es_scope *sc) {
  return NULL;
}

es_scope *null__elfscript(void *v_t) {
  return create_es_scope();
}

#endif // INCLUDE_ELFSCRIPT_CONV_NULL_H
#endif // ELFSCRIPT_REGISTRATION
