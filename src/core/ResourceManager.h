#pragma once

// TODO: test this! everything! even the resource copy/assignment shenanigans

template <typename T, typename creator>
class HAPI ResourceManager : public creator
{
    // max refcount is 2^15 and max different resources are 2^15
    struct Resource
    {
        char        data[sizeof(T)];
        int16       refcount  = -1; // -1 means the current slot is free
        int16       next_free = -1; // -1 means no next in free list
        std::string name;

        void destroy() {
            hassert(refcount == 0 || refcount == -1);
            if(refcount != -1) {
                ResourceManager::get().destroy(data);
                refcount = -1;
            }
        }

        Resource() = default;
        Resource(const Resource& other)
                : refcount(other.refcount)
                , next_free(other.next_free)
                , name(other.name) {
            HA_SUPPRESS_WARNINGS
            if(refcount != -1)
                new(data) T(*reinterpret_cast<const T*>(other.data)); // call copy ctor
            HA_SUPPRESS_WARNINGS_END
        }
        Resource& operator=(const Resource& other) {
            if(this != &other) {
                refcount  = other.refcount;
                next_free = other.next_free;
                name      = other.name;
                HA_SUPPRESS_WARNINGS
                if(refcount != -1)
                    reinterpret_cast<T&>(*data) =
                            reinterpret_cast<const T&>(*other.data); // call assignment operator
                HA_SUPPRESS_WARNINGS_END
            }
            return *this;
        }

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
                ++ResourceManager::get().m_resources[m_idx].refcount;
        }

        void release() const {
            if(m_idx != -1) {
                hassert(ResourceManager::get().m_resources[m_idx].refcount > 0);
                --ResourceManager::get().m_resources[m_idx].refcount;
            }
        }

    public:
        Handle()
                : m_idx(-1) {}

        // I wish this could be private but the serialization routines need it to construct a handle from an integer
        Handle(int16 idx)
                : m_idx(idx) {
            lease();
        }

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

        bool operator==(const Handle& other) const { return m_idx == other.m_idx; }
        bool operator<(const Handle& other) const { return m_idx < other.m_idx; }

        ~Handle() { release(); }

        // TODO: make these explicit? or leave them implicit? a bit dangerous... should stick to get()
        // a handle to a Mesh* already got passed to a function taking bool because it was the best overload... o_O
        //operator T&() { return get(); }
        //operator const T&() const { return get(); }

        T& get() {
            hassert(m_idx != -1);
            HA_SUPPRESS_WARNINGS
            return *reinterpret_cast<T*>(ResourceManager::get().m_resources[m_idx].data);
            HA_SUPPRESS_WARNINGS_END
        }
        const T& get() const { return const_cast<Handle*>(this)->get(); }
    };

    template <typename... Args>
    Handle get(const std::string& name, Args&&... args) {
        auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const Resource& res) {
            return res.refcount >= 0 && res.name == name;
        });
        if(it != m_resources.end())
            return int16(it - m_resources.begin());

        if(m_next_free == -1) {
            m_resources.emplace_back();
            creator::create(m_resources.back().data, name, std::forward<Args>(args)...);
            m_resources.back().refcount = 0;
            m_resources.back().name     = name;
            return int16(m_resources.size() - 1);
        } else {
            auto  curr_idx = m_next_free;
            auto& curr     = m_resources[curr_idx];
            hassert(curr.refcount == -1);
            creator::create(curr.data, name, std::forward<Args>(args)...);
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
