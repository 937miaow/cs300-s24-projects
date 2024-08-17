#define DMALLOC_DISABLE 1
#include "dmalloc.hh"

#include <cassert>
#include <cstring>
#include <unordered_map>

// 初始化全局统计信息
struct dmalloc_stats global_stats = {0, 0, 0, 0, 0, 0, (uintptr_t)-1, 0};
// 哈希表，存储已分配的内存块
static std::unordered_map<void*, BlockInfo> allocated_blocks;

// 跟踪内存分配，记录信息
void track_allocation(void* ptr, size_t size, const char* file, long line) {
    allocated_blocks[ptr] = {size, true, file, line};
}

// 获取内存块大小
size_t get_allocation_size(void* ptr) { return allocated_blocks[ptr].size; }

// 获取内存块是否在使用
bool is_in_use(void* ptr) { return allocated_blocks[ptr].in_use; }

// 设置内存块未使用
void set_free(void* ptr) { allocated_blocks[ptr].in_use = false; }

// 获取内存块的文件名
char* get_file(void* ptr) { return (char*)allocated_blocks[ptr].file; }

// 获取内存块的行号
long get_line(void* ptr) { return allocated_blocks[ptr].line; }

// 查找内存块是否已分配
bool find_allocation(void* ptr) {
    return allocated_blocks.find(ptr) != allocated_blocks.end();
}

// 查找包含指定指针的内存块
void* find_allocation_containing(void* ptr) {
    for (const auto& entry : allocated_blocks) {
        void* alloc_ptr = entry.first;
        size_t alloc_size = entry.second.size;

        // 检查指针是否在这个内存块内
        if (ptr >= alloc_ptr && ptr < (void*)((char*)alloc_ptr + alloc_size)) {
            return alloc_ptr;  // 返回内存块的起始地址
        }
    }
    return nullptr;  // 没有找到指针位于任何已分配的内存块内
}

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
    if (sz > SIZE_MAX - sizeof(int)) {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }
    // 计算总大小
    size_t total_size = sz + sizeof(int);

    void* ptr = base_malloc(total_size);
    if (ptr) {
        track_allocation(ptr, sz, file, line);
        global_stats.nactive++;
        global_stats.active_size += sz;
        global_stats.ntotal++;
        global_stats.total_size += sz;

        uintptr_t addr = (uintptr_t)ptr;
        if (addr < global_stats.heap_min) {
            global_stats.heap_min = addr;
        }
        if (addr + total_size > global_stats.heap_max) {
            global_stats.heap_max = addr + total_size;
        }
    } else {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }

    int pattern = 114514;  // 设定一个已知的模式值
    // 用memcpy强制写入，防止未对齐访问
    memcpy((char*)ptr + sz, &pattern, sizeof(pattern));
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
    if (!ptr) return;

    // not in heap
    if (ptr < (void*)global_stats.heap_min ||
        ptr >= (void*)global_stats.heap_max) {
        fprintf(stderr, "MEMORY BUG: invalid free of pointer %p, not in heap",
                ptr);
        abort();
    }

    // not allocated
    // 大的if不能省略，主要是性能优化
    if (!find_allocation(ptr)) {
        // 查找包含该指针的块
        void* containing_ptr = find_allocation_containing(ptr);
        if (!containing_ptr) {
            fprintf(
                stderr,
                "MEMORY BUG: %s:%ld: invalid free of pointer %p, not allocated",
                file, line, ptr);
            abort();
        }
        // 检查指针是否是块的起始地址
        if (containing_ptr != ptr) {
            fprintf(stderr,
                    "MEMORY BUG: %s:%ld: invalid free of pointer %p, not "
                    "allocated\n",
                    file, line, ptr);
            fprintf(
                stderr,
                "%s:%ld: %p is %ld bytes inside a %zu byte region allocated "
                "here\n ",
                file, get_line(containing_ptr), ptr,
                (char*)ptr - (char*)containing_ptr,
                get_allocation_size(containing_ptr));
            abort();
        }
    }

    // double free
    if (!is_in_use(ptr)) {
        fprintf(stderr, "MEMORY BUG: invalid free of pointer %p, double free",
                ptr);
        abort();
    }

    // boundary error
    size_t sz = get_allocation_size(ptr);
    int pattern = 114514;
    if (memcmp((char*)ptr + sz, &pattern, sizeof(pattern)) != 0) {
        fprintf(stderr,
                "MEMORY BUG: detected wild write during free of pointer %p",
                ptr);
        abort();
    }

    global_stats.nactive--;
    global_stats.active_size -= get_allocation_size(ptr);
    set_free(ptr);  // 设置内存块未使用
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
    // 注意溢出，所以把nmemb放在后面
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
    // 遍历哈希表，输出所有未释放的内存块
    for (auto& it : allocated_blocks) {
        if (!it.second.in_use) continue;
        void* ptr = it.first;
        size_t sz = it.second.size;
        char* file = (char*)it.second.file;
        long line = it.second.line;
        fprintf(stderr,
                "LEAK CHECK: %s:%ld: allocated object %p with size %lu\n", file,
                line, ptr, sz);
    }
}
