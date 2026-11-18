#include <print>
#include <cstdlib>
#include <iostream>

namespace ekp {

struct arena {
    size_t size_bytes = 0;
    std::byte* head = nullptr;
    size_t position_bytes = 0;

    [[nodiscard]]
    static arena init(const size_t size) {
        arena ret;
        ret.size_bytes = size;
        ret.head = static_cast<std::byte*>(std::malloc(size));
        ret.position_bytes = 0;
        return ret;
    }

    // allocates the size specified at a valid address of alignment, returning a pointer.
    // NOTE: when allocating a size 0, the position counter will not advance and that allocation may be overwritten.
    [[nodiscard]]
    void* allocate(const size_t size, const size_t alignment) {
        if (!head) {
            std::print("arena has not been initialized\n");
            return nullptr;
        }

        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            std::print("alignment must be a power of 2\n");
            return nullptr;
        }

        if (position_bytes > SIZE_MAX - (alignment - 1)) {
            std::print("alignment calculation overflowed\n");
            return nullptr;
        }

        if (alignment > alignof(std::max_align_t)) {
            std::print("arena does not support overaligned types.\n");
            return nullptr;
        }

        const size_t aligned_offset = (position_bytes + alignment - 1) & ~ (alignment - 1);

        if (aligned_offset > size_bytes || size > size_bytes - aligned_offset) {
            std::print("arena bounds exceeded\n");
            return nullptr;
        }

        position_bytes = aligned_offset + size;
        return head + aligned_offset;
    }

    // allocates memory and constructs object of type T into the allocated memory
    template<typename T, typename... Args>
    [[nodiscard]]
    T* create(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));

        return std::construct_at(
            static_cast<T*>(ptr),
            std::forward<Args>(args)...);
    }

    arena() = default;
    arena(const arena& other) = delete;
    arena& operator=(const arena& other) = delete;
    arena(arena&& other) = default;
    arena& operator=(arena&& other) = default;

    ~arena() {
        std::free(head);
        head = nullptr;
    }

private:

};

// max size includes null terminator
class text_line {
public:

    text_line(const size_t size, arena& arena)
        : _max_size(size), _arena(arena)
    {

    }

    // delete all constructors and ill define when needed
    text_line() = delete;
    text_line(const text_line& other) = delete;
    text_line& operator=(const text_line& other) = delete;
    text_line(text_line&& other) = delete;
    text_line& operator=(text_line&& other) = delete;

private:
    size_t _max_size = 0;
    size_t _length_offset = 0;
    char* _data = nullptr;
    arena& _arena;
};

}

/*
 * Arena Allocator mechanics:
 *  - allocates entire program's memory (or at least a substantial portion) at once
 *  - all allocations happen sequentially
 *  - allocations tracked via an advancing pointer according to alignment
 *      -- memory address of the allocated object must be a multiple of its alignment
 *  - nontrivial destructors typically ignored and entire arena freed at once
 *
 *  NOTE: does not explicitly support overaligned types
 */

struct printer {
    int a = 0;
    int b = 0;

    void print() {
        std::print("a: {}, b: {} \n", a, b);
    }
};

int main() {
    constexpr size_t block_size = sizeof(char) * 1024;
    ekp::arena arena = ekp::arena::init(block_size);

    const auto prtr = arena.create<printer>();
    prtr->print();
    prtr->a = 10;
    prtr->b = 20;
    prtr->print();

    // what do i expect?
    // - construct a line
    //  -- can construct it with existing text
    // - insert / edit characters at offset with space padding
    //  -- so long as offset is less than length (gotta prevent overrunning null)
    // - call .c_str() to get c string representation
    //
    // * needs to allocate string data on the arena too
    // ekp::text_line* line = arena.create<text_line>(80, "Hello, World!", arena);
    // ekp::test_line* line = arena.create<text_line>(80, arena);

    return 0;
}