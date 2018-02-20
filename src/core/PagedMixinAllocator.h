#pragma once

template <typename Mixin, int NUM_IN_PAGE = 1024>
class PagedMixinAllocator : public dynamix::mixin_allocator
{
    struct Chunk
    {
        Chunk* next;
        size_t pageIdx;
    };

    const size_t       mixin_buf_size;    // the size of a single mixin instance buffer
    const size_t       page_size;         // size in bytes of a page
    size_t             m_num_allocations; // how many allocations are there currently
    Chunk*             m_free;            // the next free chunk in the free list (in LIFO style)
    std::vector<bool>  m_allocated_flags; // if a mixin is created at the given index
    std::vector<char*> m_pages; // pages of data where each page can store NUM_IN_PAGE instances

    void new_memory_page() {
        const size_t new_page_idx = m_pages.size();
        // alloc page
        char* page = new char[page_size];
        // initialize the linked free list in the page memory
        for(char* curr = page; curr < page + page_size; curr += mixin_buf_size) {
            reinterpret_cast<Chunk*>(curr)->next = reinterpret_cast<Chunk*>(curr + mixin_buf_size);
            reinterpret_cast<Chunk*>(curr)->pageIdx = new_page_idx; // cache the page index
        }
        // set the last to point to nothing - end of the linked list
        reinterpret_cast<Chunk*>(page + page_size - mixin_buf_size)->next = nullptr;
        // redirect the head to point to the begining of the newly created page
        m_free = reinterpret_cast<Chunk*>(page);
        // add page
        m_pages.push_back(page);
        // extend bitfield
        m_allocated_flags.resize(m_allocated_flags.size() + NUM_IN_PAGE, false);
    }

    static PagedMixinAllocator* instance;

public:
    static PagedMixinAllocator& get() {
        hassert(instance);
        return *instance;
    }

    static PagedMixinAllocator& constructGlobalInstance() {
        static PagedMixinAllocator var;
        instance = &var;
        return var;
    }

    PagedMixinAllocator()
            : mixin_buf_size(mem_size_for_mixin(sizeof(Mixin), std::alignment_of<Mixin>::value))
            , page_size(mixin_buf_size * NUM_IN_PAGE)
            , m_num_allocations(0)
            , m_free(nullptr) {}

    ~PagedMixinAllocator() { freePages(); }

    std::pair<char*, size_t> alloc_mixin(const dynamix::basic_mixin_type_info&,
                                         const dynamix::object* obj) override {
        // allocate new page if free list is empty
        if(m_free == nullptr)
            new_memory_page();
        // give the chunk
        auto out_buffer = reinterpret_cast<char*>(m_free);
        ++m_num_allocations;
        // again calculate the offset using this static member function
        size_t offset = mixin_offset(out_buffer, std::alignment_of<Mixin>::value);
        // move forward in the free list
        m_free = m_free->next;

        // update bitfield
        const size_t idx =
                NUM_IN_PAGE * reinterpret_cast<Chunk*>(out_buffer)->pageIdx +
                size_t(out_buffer - m_pages[reinterpret_cast<Chunk*>(out_buffer)->pageIdx]) /
                        mixin_buf_size;
        m_allocated_flags[idx] = true;

        return {out_buffer, offset};
    }

    void dealloc_mixin(char* buf, size_t, const dynamix::basic_mixin_type_info&,
                       const dynamix::object* obj) override {
        // prepend the newly freed chunk to the free list
        Chunk* temp  = m_free;
        m_free       = reinterpret_cast<Chunk*>(buf);
        m_free->next = temp;

        // update num allocs
        --m_num_allocations;

        // search for the page index of the newly freed chunk - this happens only on deallocation
        const size_t num_pages  = m_pages.size();
        bool         found_page = false;
        for(size_t i = 0; i < num_pages; ++i) {
            if(m_pages[i] <= reinterpret_cast<char*>(m_free) &&
               reinterpret_cast<char*>(m_free) < m_pages[i] + page_size) {
                m_free->pageIdx = i;
                found_page      = true;
                break;
            }
        }
        hassert(found_page);

        // update bitfield
        const size_t idx =
                NUM_IN_PAGE * m_free->pageIdx +
                size_t(reinterpret_cast<char*>(m_free) - m_pages[m_free->pageIdx]) / mixin_buf_size;
        m_allocated_flags[idx] = false;
    }

    void freePages() {
        hassert(m_num_allocations == 0);

        // free pages
        for(size_t i = 0; i < m_pages.size(); ++i)
            delete[] m_pages[i];

        // reset fields
        m_free = nullptr;
        m_pages.clear();
        m_allocated_flags.clear();
    }

    size_t getNumAllocations() const { return m_num_allocations; }

    const std::vector<bool>& getAllocatedFlags() const { return m_allocated_flags; }

    Mixin& operator[](size_t idx) const {
        hassert(idx < m_allocated_flags.size());
        hassert(m_allocated_flags[idx]);

        char*        chunk_ptr = m_pages[idx / NUM_IN_PAGE] + (idx % NUM_IN_PAGE) * mixin_buf_size;
        const size_t offset    = mixin_offset(chunk_ptr, std::alignment_of<Mixin>::value);
        return *(reinterpret_cast<Mixin*>(chunk_ptr + offset));
    }
};
