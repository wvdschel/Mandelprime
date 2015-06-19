#include "stdlib.h"
#include "string.h"

#include "refcount.h"
#include "log.h"

typedef struct refcount_ptr
{
  size_t references;
  size_t size;
  void*  ptr;
} refcount_ptr_t;

static int sanity_check(void *ptr)
{
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);
  int retval = 0;

  if(ptr != refcount->ptr)
  {
    retval += 1;
    dlog("Warning: reference counted pointer %p has a corrupted marker.", ptr);
  }

  return retval;
}

void* refcount_allocate(size_t bytes)
{
  refcount_ptr_t *refcount = malloc(bytes + sizeof(refcount_ptr_t));
  refcount->references = 1;
  refcount->size = bytes;
  refcount->ptr  = (void*)(refcount) + sizeof(refcount_ptr_t);

  sanity_check(refcount);

  dlog("Created reference counted pointer at %p.", refcount->ptr);
  
  return refcount->ptr;
}

void  refcount_free(void* ptr)
{
  sanity_check(ptr);
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);
  
  if(refcount->references > 1)
    dlog("Warning: freeing pointer %p which still has %zu references.", ptr, refcount->references);
  dlog("Freeing reference counted pointer at %p.", ptr);
  
  free(refcount);
}

void* refcount_copy(void* ptr)
{
  sanity_check(ptr);
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);

  void* new_ptr = refcount_allocate(refcount->size);
  memcpy(new_ptr, ptr, refcount->size);

  return new_ptr;
}

void* refcount_resize(void* ptr, size_t new_size)
{
  sanity_check(ptr);
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);

  void* new_ptr = refcount_allocate(new_size);
  memcpy(new_ptr, ptr, refcount->size > new_size ? new_size : refcount->size);

  refcount_decrement(ptr);
  return new_ptr;
}

size_t refcount_size(void* ptr)
{
  sanity_check(ptr);
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);

  return refcount->size;
}

void  refcount_increment(void* ptr)
{
  sanity_check(ptr);
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);
  refcount->references++;
}

void  refcount_decrement(void* ptr)
{
  sanity_check(ptr);
  refcount_ptr_t *refcount = ptr - sizeof(refcount_ptr_t);

  if(refcount->references) refcount->references--;

  if(refcount->references == 0) refcount_free(ptr);
}
