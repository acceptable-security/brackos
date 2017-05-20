# SLEB Allocator
It's like the SLUB allocator... without some of the funny business.

# Structure

The SLEB allocator has a linked list of caches, like the normal SLUB allocator. Each cache has two lists. One is the
used list, which contains slabs that are completely used (no free spots left). The other is the unused list. The unused
list utilizes an intrusive linked list within the free items of the unused list that spans between all unused slabs.
The way that looks is...

     |----v|---v|-- (cont. below)
    +-------------+
    |F|U|U|F|U|F|U|
    +-------------+
    -v|----v|--v
    +-------------+
    |F|U|U|F|U|F|U|
    +-------------+

Each free item contains a pointer to the next free item. The last free item in one slab points to the next free item in
the next slab. If there are no free items, then the pointer in the cache to the first free item is NULL. This way when
someone tries to allocate from it, the allocator immediately sees the NULL, and allocates a new page for it to use.

# Initialization
An initial cache_cache is created in the data section. It's initial lists are completely empty. When a cache is created,
it comes from the cache_cache and is added to the end of the linked list.

# Allocating
The cache is located in the linked list of caches. Once the cache is located the free list pointer is checked. If it is
NULL then a new page is allocated, every object within it is initialized to the next object's pointer (the free list)
except for the last object which is initialized to NULL, and the free list pointer is set to that. Then the free list's
pointer is taken and the next pointer is read (by dereferencing the first one). Then it is determined if they are on the
same page or not (`~(A & B) > 0x1000`). If they aren't then the slab has been completely used up and is added to the
used list. The allocated object is then returned to the caller.

# Deallocating
