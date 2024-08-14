#define DMALLOC_DISABLE 1
#include "dmalloc.hh"

#include <cassert>
#include <cstring>
#include <unordered_map>

struct dmalloc_stats global_stats = {0, 0, 0, 0, 0, 0, (uintptr_t)-1, 0};
std::unordered_map<void*, BlockInfo> allocated_blocks;

void track_allocation(void* ptr, size_t size) {
    allocated_blocks[ptr] = {size, true};
}

bool find_allocation(void* ptr) {
    return allocated_blocks.find(ptr) != allocated_blocks.end();
}

size_t get_allocation_size(void* ptr) { return allocated_blocks[ptr].size; }

bool is_in_use(void* ptr) { return allocated_blocks[ptr].in_use; }

void set_free(void* ptr) { allocated_blocks[ptr].in_use = false; }

/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of
 * memory and return a pointer to it
 *
 * @arg size_t sz : the amount of memory requested
 * @arg const char *file : a string containing the filename from which
 * dmalloc was called
 * @arg long line : the line number from which dmalloc was called
 *
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line) {
    (void)file, (void)line;  // avoid uninitialized variable warnings
    // Your code here.
    // 检查是否溢出
    if (sz > SIZE_MAX) {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }

    void* ptr = base_malloc(sz);
    if (ptr) {
        track_allocation(ptr, sz);
        global_stats.nactive++;
        global_stats.active_size += sz;
        global_stats.ntotal++;
        global_stats.total_size += sz;

        uintptr_t addr = (uintptr_t)ptr;
        if (addr < global_stats.heap_min) {
            global_stats.heap_min = addr;
        }
        if (addr + sz > global_stats.heap_max) {
            global_stats.heap_max = addr + sz;
        }
    } else {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }
    return ptr;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`.
 * This should be a pointer that was previously allocated on the heap. If `ptr`
 * is a nullptr do nothing.
 *
 * @arg void *ptr : a pointer to the heap
 * @arg const char *file : a string containing the filename from which dfree was
 * called
 * @arg long line : the line number from which dfree was called
 */
void dfree(void* ptr, const char* file, long line) {
    (void)file, (void)line;  // avoid uninitialized variable warnings
    // Your code here.
    if (!ptr) {
        return;
    }
    // not in heap
    if (ptr < (void*)global_stats.heap_min ||
        ptr >= (void*)global_stats.heap_max) {
        fprintf(stderr, "MEMORY BUG: invalid free of pointer %p, not in heap",
                ptr);
        abort();
    }
    // not allocated
    if (!find_allocation(ptr)) {
        fprintf(stderr,
                "MEMORY BUG: %s:%ld: invalid free of pointer %p, not allocated",
                file, line, ptr);
        abort();
    }
    // double free
    if (!is_in_use(ptr)) {
        fprintf(stderr, "MEMORY BUG: invalid free of pointer %p, double free",
                ptr);
        abort();
        return;
    }
    global_stats.nactive--;
    global_stats.active_size -= get_allocation_size(ptr);
    set_free(ptr);  // 修改hashtable

    base_free(ptr);
}
/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an
 * array of `nmemb` number of elements with wach element being `sz` bytes.
 * The memory should be initialized to zero
 *
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested
 * for
 * @arg const char *file : a string containing the filename from which
 * dcalloc was called
 * @arg long line : the line number from which dcalloc was called
 *
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).
    if (nmemb && sz > (SIZE_MAX) / nmemb) {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }
    size_t total_size = nmemb * sz;
    void* ptr = dmalloc(total_size, file, line);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics
 *
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we
 * want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(dmalloc_stats));
    // Your code here.
    *stats = global_stats;
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout
 */
void print_statistics() {
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.
}
