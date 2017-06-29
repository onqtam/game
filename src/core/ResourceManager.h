#pragma once

template <typename T, typename creator>
class HAPI ResourceManager : public creator
{
    typedef ResourceManager<T, creator> this_RM;

    // max refcount is 2^15 and max different resources are 2^15
    struct Resource
    {
        char        data[sizeof(T)];
        int16       refcount  = -1; // -1 means the current slot is free
        int16       next_free = -1; // -1 means no next in free list
        std::string name;

        void destroy() {
            PPK_ASSERT(refcount <= 0);
            if(refcount == 0) {
                HA_SUPPRESS_WARNINGS
                reinterpret_cast<T*>(data)->~T();
                HA_SUPPRESS_WARNINGS_END
                refcount = -1;
            }
        }

        Resource()                = default;
        Resource(const Resource&) = default;
        Resource& operator=(const Resource&) = default;

        ~Resource() { destroy(); }
    };

    std::vector<Resource> m_resources;
    int16                 m_next_free = -1;

    HA_SCOPED_SINGLETON(ResourceManager, class Application);

public:
    class Handle
    {
        template <typename, typename>
        friend class ResourceManager;

        int16 m_idx;

        void lease() const {
            if(m_idx != -1)
                ++this_RM::get().m_resources[m_idx].refcount;
        }

        void release() const {
            if(m_idx != -1) {
                PPK_ASSERT(this_RM::get().m_resources[m_idx].refcount > 0);
                --this_RM::get().m_resources[m_idx].refcount;
            }
        }

        Handle(int16 idx = -1)
                : m_idx(idx) {
            lease();
        }

    public:
        Handle(Handle&& other) noexcept
                : m_idx(other.m_idx) {
            other.m_idx = -1;
        }

        Handle(const Handle& other)
                : m_idx(other.m_idx) {
            lease();
        }

        Handle& operator=(Handle&& other) noexcept {
            release();
            m_idx       = other.m_idx;
            other.m_idx = -1;
            return *this;
        }

        Handle& operator=(const Handle& other) {
            if(this != &other) {
                release();
                m_idx = other.m_idx;
                lease();
            }
            return *this;
        }

        ~Handle() { release(); }

        T& get() {
            PPK_ASSERT(m_idx != -1);
            HA_SUPPRESS_WARNINGS
            return *reinterpret_cast<T*>(this_RM::get().m_resources[m_idx].data);
            HA_SUPPRESS_WARNINGS_END
        }
        const T& get() const { return const_cast<Handle*>(this)->get(); }
    };

    Handle get(const std::string& name) {
        auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const Resource& res) {
            return res.refcount >= 0 && res.name == name;
        });
        if(it != m_resources.end())
            return int16(it - m_resources.begin());

        if(m_next_free == -1) {
            m_resources.emplace_back();
            creator::create(m_resources.back().data, name);
            m_resources.back().refcount = 0;
            m_resources.back().name     = name;
            return int16(m_resources.size() - 1);
        } else {
            auto  curr_idx = m_next_free;
            auto& curr     = m_resources[curr_idx];
            PPK_ASSERT(curr.refcount == -1);
            creator::create(curr.data, name);
            curr.refcount = 0;
            m_next_free   = curr.next_free;
            return curr_idx;
        }
    }

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
};

template <typename T, typename creator>
ResourceManager<T, creator>* ResourceManager<T, creator>::s_instance = nullptr;

#define HA_RESOURCE_MANAGER(type, creator)                                                         \
    template class ResourceManager<type, creator>;                                                 \
    typedef ResourceManager<type, creator>         HA_CAT_1(type, Man);                            \
    typedef ResourceManager<type, creator>::Handle HA_CAT_1(type, Handle)

struct intCreator
{
    int create(char* storage, const std::string&) {
        new(storage) int();
        return 0;
    }
};

HA_RESOURCE_MANAGER(int, intCreator); // produces intMan and intHandle
