# clib

A high-performance C data structure library featuring an Arena allocator, dynamic vectors, chaining and open addressing hash maps, and binary heaps. Built as part of a [self-directed C curriculum](https://github.com/WillEdgington/c-curriculum).

---

## Build

Requires `gcc` and `make`.

```bash
make         # Builds the static library libclib.a
make test    # Compiles and runs unit tests
make metric  # Compiles and runs performance benchmarks
make debug   # Builds with debug symbols and sanitisers
```

To remove compiled objects, dependency files and binaries:
```bash
make clean
```

---

## Project Structure

```
include/clib/
├── arena.h            Slab-based chained memory management
├── vector.h           Dynamic contiguous array implementation
├── hashmap.h          Chaining-based hash map
├── ohashmap.h         Open-addressing hash map
├── heap.h             Binary heap (Min/Max)
├── iter.h             Generic iterator
├── test_framework.h   Macros for unit testing
└── metric_framework.h Nanosecond timing and RSS memory tracking
src/                   Core implementation (.c files)
tests/                 Unit tests (functional verification)
metrics/               Performance benchmarks for key operations
```

---

## Core Architecture & API Reference

### Arena Allocator (`arena.c/h`)

The `Arena` struct implements a **slab-based chained memory management** system. By allocating memory in large, contiguous blocks (slabs) and carving out space for smaller allocations, it significantly reduces the overhead associated with frequent `malloc` and `free` calls. This approach also improves cache locality by keeping related data physically close in memory.

Because the `Arena` chains `Region` structs together, it can grow dynamically as needed. This makes it an ideal backbone for the [`HashMap`](#chaining-hashmap-hashmapch) and [`OHashMap`](#open-addressing-hashmap-ohashmapch) implementations, allowing them to manage their internal storage with minimum fragmentation. A key advantage of this architecture is bulk deallocation: instead of tracking and freeing thousands of individual nodes, a single call to `arena_free` reclaims all memory associated with the workload.

It is recommended to choose a large slab size to avoid pockets of wasted memory at the end of each arena.

|Function|Description|
|---|---|
|`int arena_init(Arena *a, size_t slab_size)`|Initialises an `Arena` with a fixed `slab_size` for its initial and subsequent slabs.|
|`void *arena_alloc(Arena *a, size_t size)`|Allocates size bytes from the current slab. If the current slab is full, a new slab is automatically chained.|
|int arena_reset(Arena *a)|Rewinds the offsets in all allocated slabs to zero, allowing the memory to be reused without re-allocation.|
|void arena_free(Arena *a)|Frees all slabs and regions associated with the Arena back to the system.|

### Iterator (`iter.h`)

The `Iter` struct provides a unified interface for traversing any collection in the library. This allows you to write generic functions that process data regardless of whether it is stored in a Vector, Heap, HashMap or elsewhere. Once an `Iter` struct is initialised properly it can be used like such:

```C
/* it.next(Iter *it) returns 0 (success), 1 (end), -1 (error) */
while (it.next(&it) == 0) {
  /* access it.current.value */
}
```

You can see in the individual data structure sections how `Iter` is utilised further but the general rule of thumb is that you need to define a **constructor method** (e.g. `Iter vector_iter(Vector *v)`) and a **next method** (e.g. `int vector_iter_next(Iter *it)`).

### Vector (`vector.c/h`)

The `Vector` struct is a **dynamic, contiguous array** that supports amortised $O(1)$ insertions. It serves as the primary storage backend for the [Binary Heap](#binary-heap-heapch).

|Function|Description|
|---|---|
|`int vector_init(Vector *v, size_t item_size)`|Initialises an empty `Vector`.|
|`int vector_push(Vector *v, const void *item)`|Appends `item` to `Vector`, resizing if necessary.|
|`void *vector_get(const Vector *v, size_t index)`|Returns a pointer to the item at `index`.|
|`int vector_pop(Vector *v, void *ptr)`|Removes the last item and assigns the pointer to that item to `*ptr`.|
|`void vector_free(Vector *v)`|Frees the allocated memory for `Vector` struct.|
|`Iter vector_iter(Vector *vector)`|Returns an `Iter` struct for iteration of items in `Vector`.|
|`int vector_iter_next(Iter *iter)`|Moves `Iter` struct to the next item in the `Vector` (no need for explicit use of this method).|

### Chaining HashMap (`hashmap.c/h`)

The `HashMap` struct is a **separate chaining** hash map implementation. Each bucket is the head of a linked list. This implementation is particularly robust against collision rates. 

`HashMap` uses an arena allocator to store its key-value pairs. It is sensitive to low slab sizes for the arena allocator, but given a sensible slab size `HashMap` becomes the preferred implementation (see [Performance Benchmarking](#hashmap-chaining-vs-open-addressing-vs_hashmapc) for more details).

`HashMap` has customisable compare and hash functions. The default hash function is an implementation of **FNV-1a**, which treats the key as a raw sequence of bytes and is robust for most scenarios. The default compare function uses **`memcmp`** to compare the two blocks of memory for each key.

|Function|Description|
|---|---|
|`int hashmap_init(HashMap *m, size_t key_size, size_t val_size, Arena *a)`|Initialises `HashMap` struct with `Arena` backed storage.|
|`void hashmap_set_functions(HashMap *m, HashFn h, CompareFn c)`|Sets functions for hash and compare functions (NULL for h or c, falls back to default).|
|`int hashmap_put(HashMap *m, const void *k, const void *v)`|Inserts or updates a key-value pair.|
|`void *hashmap_get(const HashMap *m, const void *k)`|Retrieves a pointer to the value associated with a key.|
|`int hashmap_remove(HashMap *m, const void *k)`|Removes key-value pair from `HashMap` if it exists.|
|`size_t hashmap_count(const HashMap *m)`|Returns the amount of key-value pairs in `HashMap` struct.|
|`void hashmap_free(HashMap *m)`|Frees the allocated memory for `HashMap` struct (this does not free memory allocated in `Arena`).|
|`Iter hashmap_iter(HashMap *m)`|Returns an `Iter` struct for iteration of key-value pairs in `HashMap` struct|
|`int hashmap_iter_next(Iter *it)`|Moves `Iter` struct to the next key-value pair in `HashMap` (no need for explicit use of this method).|

### Open-addressing HashMap (`ohashmap.c/h`)

The `OHashMap` struct is a **linear probing** hash map implementation. By storing all entries in a single contiguous array, it minimises pointer chasing and maximises CPU cache hits. It is ideal for workloads where memory locality is the primary performance bottleneck.

`OHashMap` uses an arena allocator to store its key-value pairs. It is robust to low slab sizes for the arena allocator, but this also means it sees minimal performance benefits from the arena allocator (see [Performance Benchmarking](#hashmap-chaining-vs-open-addressing-vs_hashmapc) for more details).

Like `HashMap`, `OHashMap` has customisable compare and hash functions. The default hash function is an implementation of **FNV-1a**, which treats the key as a raw sequence of bytes and is robust for most scenarios. The default compare function uses **`memcmp`** to compare the two blocks of memory for each key.

|Function|Description|
|---|---|
|`int ohashmap_init(OHashMap *m, size_t key_size, size_t val_size, Arena *a)`|Initialises `OHashMap` struct with `Arena` backed storage.|
|`void ohashmap_set_functions(OHashMap *m, HashFn h, CompareFn c)`|Sets functions for hash and compare functions (`NULL` for `h` or `c`, falls back to default).|
|`int ohashmap_put(OHashMap *m, const void *k, const void *v)`|Inserts or updates a key-value pair.|
|`void *ohashmap_get(const OHashMap *m, const void *k)`|Retrieves a pointer to the value associated with a key.|
|`int ohashmap_remove(OHashMap *m, const void *k)`|Removes key-value pair from `OHashMap` if it exists.|
|`size_t ohashmap_count(const OHashMap *m)`|Returns the amount of key-value pairs in `OHashMap` struct.|
|`void ohashmap_free(OHashMap *m)`|Frees the allocated memory for `OHashMap` struct (this does not free memory allocated in `Arena`).|
|`Iter ohashmap_iter(OHashMap *m)`|Returns an `Iter` struct for iteration of key-value pairs in `OHashMap` struct|
|`int ohashmap_iter_next(Iter *it)`|Moves `Iter` struct to the next key-value pair in `OHashMap` (no need for explicit use of this method).|

### Binary Heap (`heap.c/h`)

The `Heap` struct is a **priority queue** implementation that is built on top of a [`Vector`](#vector-vectorch) struct for item storage and organisation.

`Heap` contains a customisable compare function, allowing a custom hierarchy to be established. By default, this compare function provides the utility for an `int` based min-heap.

|Function|Description|
|---|---|
|`int heap_init(Heap *h, size_t item_size, HeapCompareFn c)`|Initialises a `Heap` with a custom `HeapCompareFn` (`NULL` for `c`, falls back to deafult).|
|`void heap_heapify(Heap *h, Vector *d)`|Bulk-loads a `Vector` into a `Heap` in $O(n)$ time.|
|`int heap_push(Heap *h, const void *value)`|Standard $O(logn)$ insertion onto heap|
|`int heap_pop(Heap *h, void *ptr)`|Removes the top element, restores order ($O(\log n)$) and stores a pointer to the element in `*ptr`.|
|`void *heap_peek(Heap *h)`|Returns a pointer to the top element without removal.|
|`void heap_free(Heap *h)`|Frees the allocated memory for `Heap` struct.|
|`Iter heap_iter(Heap *heap)`|Returns an `Iter` struct for level-order traversal of items in the binary heap tree.|
|`int heap_iter_next(Iter *iter)`|Moves `Iter` struct to the next item in `Heap` (no need for explicit use of this method).|

---

## Performance Benchmarking

The library includes a custom benchmarking suite in `metrics/` that measures:
1. **Throughput:** Average time per operation in nanoseconds ($ns/op$).
2. **Memory Footprint:** Tracks **Resident Set Size (RSS)** changes via `/proc/self/status` to measure physical memory impact in kilobytes (KB).

> **Note:** All metrics were captured on an Intel i5-1135G7 @ 2.40GHz (16GB RAM).

### Vector Benchmarks (`unit_vector.c`)

These are the results from running the compiled binaries for the `metrics/unit_vector.c` script. These tests are for an `int` based `Vector`.

|Operation (N)|Speed|Memory|
|---|---|---|
|**Push** (1M)|5.71 ns/op|+4224 KB|
|**Pop** (1M)|3.61 ns/op|+0 KB|
|**Get** (1M)|1.52 ns/op|+0 KB|
|**Iteration** (1M)|1.27 ns/op|+0 KB|

**Analysis:** Random access and iteration are essentially instantaneous, costing roughly 1-2 nanoseconds. Even the `push` operation, which includes capacity checks and occasional resizing, is highly optimised.

### Heap Benchmarks (`unit_heap.c`)

These are the results from running the compiled binaries for the `metrics/unit_heap.c` script. These tests are for an `int` based Min-Heap.

|Operation (N)|Speed|Memory|
|---|---|---|
|**Push (Ascending Vals)** (1M)|7.50 ns/op|+4224 KB|
|**Push (Descending Vals)** (1M)|193.83 ns/op|+3712 KB|
|**Heapify (Descending Vals)** (1M)|0.0155 s (whole Vector)|+0 KB|
|**Pop** (1M)|239.82 ns/op|+0 KB|
|**Iteration** (1M)|1.43 ns/op|+0 KB|

**Analysis:** In a Min-Heap, pushing values that are already in order is nearly as fast as a `Vector` `push` ($O(1)$). However, the worst-case `pop` is significantly more expensive because it requires a two-way comparison at every level of the tree.

**The Heapify Optimisation:** Building a Min-Heap of 1 million items using repeated `push` calls would take **~192.5ms** for descending values (and **~7.6ms** for ascending values). By using `heap_heapify`, the same task (Descending Values) is completed in **~15.5ms** (**~12.5x speedup**).

### HashMap: Chaining vs. Open-Addressing (`vs_hashmap.c`)

These are the results from running the compiled binaries for the `metrics/vs_hashmap.c` script. These tests are for a `HashMap` (chaining), `OHashMap` (open-addressing) with load factor of 0.70.

**Arena Slab Size - 4096 bytes:**

|Operation (N)|Speed|Memory|
|---|---|---|
|**Put (Chaining)** (1M)|177.18 ns/op|+56136 KB|
|**Get (Chaining)** (1M)|27.83 ns/op|+0 KB|
|**Put (Open-Addressing)** (1M)|209.53 ns/op|+73540 KB|
|**Get (Open-Addressing)** (1M)|42.75 ns/op|+0 KB|

**Arena Slab Size - 64 bytes:**

|Operation (N)|Speed|Memory|
|---|---|---|
|**Put (Chaining)** (1M)| 289.08 ns/op|+89336 KB|
|**Get (Chaining)** (1M)| 34.06 ns/op|+0 KB|
|**Put (Open-Addressing)** (1M)| 220.43 ns/op|+49152 KB|
|**Get (Open-Addressing)** (1M)| 42.88 ns/op|+0 KB|

**Analysis:** The results reveal a significant performance shift depending on the **Arena Slab Size**.
- **Slab Size Impact (4096 bytes):** With a larger slab size, **Chaining** is the clear winner. It is **~20%** faster on `put` operations and **~50%** faster on `get` operations. Because the Arena packs linked nodes into large, contiguous blocks, Chaining achieves high cache locality, effectively mitigating the "pointer chasing" typically associated with linked structures.
- **Slab Size Impact (64 bytes):** When the slab size is reduced to 64 bytes, Chaining suffers from **Slab Thrashing**. Memory consumption jumps by **~60%** (+56MB to +89MB) and `put` speed increases by **~60%**. This likely happens because each small allocation for a node is likely triggering the allocation of a new Arena region, leading to significant memory fragmentation and overhead.
- **Open-Addressing Resilience:** `OHashMap` is much more resilient to slab size changes. Because it maintains one large contiguous array, its performance remains stable regardless of whether the Arena provides 4KB or 64B slabs. Interestingly, at the 64B slab size, it becomes the faster implementation for `put` operations, as it avoids the allocation overhead that cripples Chaining in small-memory environments.

### HashMap Collision Resilience: Chaining vs. Open-Addressing (`vs_hashmap_collisions.c`)

These are the results from running the compiled binaries for the `metrics/vs_hashmap_collisions.c` script. These tests are for a `HashMap` (chaining), `OHashMap` (open-addressing) with load factor of 0.70.

For this test, the hash maps use a custom `bad_hash` hash function that always returns 0. This means we have a 100% collision rate.

**Arena Slab Size - 4096 bytes:**

|Operation (N)|Speed|Memory|
|---|---|---|
|**Put (Chaining)** (10K)| 14463.53 ns/op|+512 KB|
|**Get (Chaining)** (10K)| 13157.05 ns/op|+0 KB|
|**Put (Open-Addressing)** (10K)| 38884.60 ns/op|+256 KB|
|**Get (Open-Addressing)** (10K)| 27635.73 ns/op|+0 KB|

**Arena Slab Size - 64 bytes:**

|Operation (N)|Speed|Memory|
|---|---|---|
|**Put (Chaining)** (10K)| 17691.04 ns/op|+768 KB|
|**Get (Chaining)** (10K)| 18805.31 ns/op|+0 KB|
|**Put (Open-Addressing)** (10K)| 38350.02 ns/op|+256 KB|
|**Get (Open-Addressing)** (10K)| 28490.30 ns/op|+0 KB|

**Analysis:** Forcing a 100% collision rate provides a "stress test" for the internal search and insertion logic. While both implementations degrade from $O(1)$ to $O(N)$, **Chaining** proves to be significantly more resilient.
- **Chaining Resilience:** Even with 10,000 items in one bucket, Chaining is **~2x** faster than Open-Addressing. Sequential `Entry` allocations within a large Arena slab ensure the linked list nodes remain physically adjacent, minimizing cache misses during the linear search.
- **The Open-Addressing "Primary Clustering" penalty:** Every `put` and `get` must scan the entire contiguous block of occupied slots starting from index 0. This linear scan is significantly slower than the localised list traversal in Chaining, leading to a massive increase in $ns/op$.

---

## Author

Created by [**WillEdgington**](https://github.com/WillEdgington)

📧 [**willedge037@gmail.com**](mailto:willedge037@gmail.com) &nbsp;|&nbsp; 🔗 [**LinkedIn**](https://www.linkedin.com/in/williamedgington/)