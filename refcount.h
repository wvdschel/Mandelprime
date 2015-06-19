#ifndef __MANDELPRIME_REFCOUNT_H_
#define __MANDELPRIME_REFCOUNT_H_

/**
 * This file provides a bare bones interface to reference counted pointers.
 **/

/**
 * Allocate a chunck of memory with associated reference counter.
 *
 * After this function returns, the number of references will be
 * set to 1.
 *
 * @param bytes Number of bytes to allocate
 * @return Pointer to the allocated memory.
 **/
void* refcount_allocate(size_t bytes);

/**
 * Free a pointer, regardless of whether it still has references.
 *
 * If ptr was not allocated with refcount_allocate, the behaviour
 * is undefined.
 *
 * @param ptr The pointer to free.
 **/
void  refcount_free(void* ptr);

/**
 * Perform a deep copy of a pointer. This will create a new reference
 * counted pointer, with a reference count of 1, with the same memory
 * contents as the original pointer.
 *
 * If ptr was not allocated with refcount_allocate, the behaviour
 * is undefined.
 *
 * @param ptr The pointer to copy.
 * @return A new pointer, pointing to a copy of the contents found at ptr.
 **/
void* refcount_copy(void* ptr);

/**
 * Resize a pointer.
 *
 * A copy of the original pointer will be made, and the 
 * the original will have its reference count reduced by one.
 *
 * If the size is not changed, nothing will happen.
 *
 * If ptr was not allocated with refcount_allocate, the behaviour
 * is undefined.
 *
 * @param ptr The pointer to resize.
 * @param new_size New size, in bytes, to allocate.
 * @return A pointer pointing to the memory region.
 **/
void* refcount_resize(void* ptr, size_t new_size);

/**
 * Increment the number of references for a pointer.
 * @param ptr Pointer to increment reference counter for.
 **/
void  refcount_increment(void* ptr);

/**
 * Decrement the number of references for a pointer.
 *
 * If the number of references is 0 after decrementing,
 * the pointer and its reference counter will be destroyed.
 *
 * @param ptr Pointer to increment reference counter for.
 **/
void  refcount_decrement(void* ptr);
/**
 * Get the number of bytes allocated at a pointer.
 *
 * If ptr was not allocated with refcount_allocate, the behaviour
 * is undefined.
 *
 * @param ptr The pointer you wish to know the size of.
 * @return The number of bytes allocated at ptr.
 **/
size_t refcount_size(void* ptr);

#endif // __MANDELPRIME_REFCOUNT_H_
