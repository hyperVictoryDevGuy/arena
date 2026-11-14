#include <print>
#include <cstdlib>

namespace ekp {

struct arena {
    size_t size_bytes = 0;
    std::byte* head = nullptr;
    std::byte* position = nullptr;

    [[nodiscard]]
    static arena init(const size_t size) {
        arena ret;
        ret.size_bytes = size;
        ret.head = static_cast<std::byte*>(std::malloc(size));
        ret.position = ret.head;
        return ret;
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

}

/*
 * Arena Allocator mechanics:
 *  - allocates entire program's memory (or at least a substantial portion) at once
 *  - all allocations happen sequentially
 *  - allocations tracked via an advancing pointer
 *  - nontrivial destructors typically ignored and entire arena freed at once
 */

int main() {
    constexpr size_t block_size = sizeof(char) * 1024;
    ekp::arena arena = ekp::arena::init(block_size);

    std::print("arena size: {}\n", arena.size_bytes);
    return 0;
}