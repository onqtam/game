
#ifndef HA_RESOURCE_MANAGER
#error "Define HA_RESOURCE_MANAGER before including ResourceManager.inl !!!"
#endif
#ifndef HA_RESOURCE_CREATOR
#error "Define HA_RESOURCE_CREATOR before including ResourceManager.inl !!!"
#endif
#ifndef HA_RESOURCE_TYPE
#error "Define HA_RESOURCE_TYPE before including ResourceManager.inl !!!"
#endif

#include "utils/utils.h"

class HAPI HA_RESOURCE_MANAGER : protected HA_RESOURCE_CREATOR,
                                 public Singleton<HA_RESOURCE_MANAGER>
{
    // max refcount is 2^15 and max different resources are 2^15
    struct Resource
    {
        char        data[sizeof(HA_RESOURCE_TYPE)];
        int16       refcount  = -1; // -1 means the current slot is free
        int16       next_free = -1; // -1 means no next in free list
        std::size_t hash      = 0;
        std::string name;

        void destroy() {
            hassert(refcount == 0 || refcount == -1);
            if(refcount != -1) {
                HA_RESOURCE_MANAGER::get().destroy(data);
                refcount = -1;
            }
        }

        Resource() = default;

        Resource(Resource&& other)
                : refcount(other.refcount)
                , next_free(other.next_free)
                , hash(other.hash)
                , name(std::move(other.name)) {
            HA_SUPPRESS_WARNINGS
            if(refcount != -1) {
                new(data) HA_RESOURCE_TYPE(std::move(
                        *reinterpret_cast<HA_RESOURCE_TYPE*>(other.data))); // call move/copy ctor
                reinterpret_cast<HA_RESOURCE_TYPE*>(other.data)
                        ->~HA_RESOURCE_TYPE(); // destroy other instance using it's dtor - not the inherited destroy method
                other.refcount = -1;
            }
            HA_SUPPRESS_WARNINGS_END
        }

        ~Resource() { destroy(); }

        Resource(const Resource& other) = delete;
        Resource& operator=(const Resource& other) = delete;
    };

    std::vector<Resource> m_resources;
    int16                 m_next_free = -1;

    HA_SINGLETON(HA_RESOURCE_MANAGER);

public:
    HA_RESOURCE_MANAGER()
            : Singleton(this) {}

    class Handle
    {
        friend class HA_RESOURCE_MANAGER;

        int16 m_idx = -1;

        void incref() const {
            if(isValid())
                ++HA_RESOURCE_MANAGER::get().m_resources[m_idx].refcount;
        }

        void decref() const {
            if(isValid()) {
                hassert(HA_RESOURCE_MANAGER::get().m_resources[m_idx].refcount > 0);
                --HA_RESOURCE_MANAGER::get().m_resources[m_idx].refcount;
            }
        }

        Handle(int16 idx)
                : m_idx(idx) {
            incref();
        }

    public:
        Handle() = default;

        Handle(Handle&& other) noexcept
                : m_idx(other.m_idx) {
            other.m_idx = -1;
        }

        Handle(const Handle& other)
                : m_idx(other.m_idx) {
            incref();
        }

        Handle& operator=(Handle&& other) noexcept {
            decref();
            m_idx       = other.m_idx;
            other.m_idx = -1;
            return *this;
        }

        Handle& operator=(const Handle& other) {
            if(this != &other) {
                decref();
                m_idx = other.m_idx;
                incref();
            }
            return *this;
        }

        bool operator==(const Handle& other) const { return m_idx == other.m_idx; }
        bool operator<(const Handle& other) const { return m_idx < other.m_idx; }

        ~Handle() { decref(); }

        explicit operator HA_RESOURCE_TYPE&() { return get(); }
        explicit operator const HA_RESOURCE_TYPE&() const { return get(); }

        HA_RESOURCE_TYPE& get() {
            hassert(isValid());
            HA_SUPPRESS_WARNINGS
            return *reinterpret_cast<HA_RESOURCE_TYPE*>(
                    HA_RESOURCE_MANAGER::get().m_resources[m_idx].data);
            HA_SUPPRESS_WARNINGS_END
        }
        const HA_RESOURCE_TYPE& get() const { return const_cast<Handle*>(this)->get(); }

        bool isValid() const { return m_idx != -1; }

        int16 refcount() const {
            hassert(isValid());
            return HA_RESOURCE_MANAGER::get().m_resources[m_idx].refcount;
        }
        void release() {
            decref();
            m_idx = -1;
        }
    };

    template <typename... Args>
    Handle get(const std::string& name, Args&&... args) {
        std::size_t hash = 0;
        Utils::hash_combine(hash, name, args...);

        auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const Resource& res) {
            return res.refcount >= 0 && res.hash == hash;
        });
        if(it != m_resources.end())
            return Handle(int16(it - m_resources.begin()));

        if(m_next_free == -1) {
            m_resources.emplace_back();
            auto& curr = m_resources.back();
            HA_RESOURCE_CREATOR::create(curr.data, name, std::forward<Args>(args)...);
            curr.refcount = 0;
            curr.hash     = hash;
            curr.name     = name;
            return Handle(int16(m_resources.size() - 1));
        } else {
            auto  curr_idx = m_next_free;
            auto& curr     = m_resources[curr_idx];
            hassert(curr.refcount == -1);
            HA_RESOURCE_CREATOR::create(curr.data, name, std::forward<Args>(args)...);
            curr.refcount = 0;
            curr.hash     = hash;
            m_next_free   = curr.next_free;
            return Handle(curr_idx);
        }
    }

    Handle getHandleFromIndex_UNSAFE(int16 idx) { return Handle(idx); }

    void free() {
        auto size = m_resources.size();
        for(size_t i = 0; i < size; ++i) {
            if(m_resources[i].refcount == 0) {
                m_resources[i].destroy();
                m_resources[i].next_free = m_next_free;
                m_next_free              = int16(i);
            }
        }
    }

    uint16 numFreeSlots() const {
        auto   curr = m_next_free;
        uint16 res  = 0;
        while(curr != -1) {
            ++res;
            curr = m_resources[curr].next_free;
        }
        return res;
    }

    uint16 numSlots() const { return uint16(m_resources.size()); }

    uint16 numCanFree() const {
        return uint16(count_if(m_resources.begin(), m_resources.end(),
                               [](const Resource& res) { return res.refcount == 0; }));
    }
};

#undef HA_RESOURCE_MANAGER
#undef HA_RESOURCE_CREATOR
#undef HA_RESOURCE_TYPE
