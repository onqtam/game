#include <vector>
#include <cstdlib>
#include <algorithm>
 
template <class Key, class Value, template <class...> class Storage = std::vector>
struct flat_map {
        struct kv {
                Key k;
                Value v;
                template <class K, class V>
                kv(K&& kin, V&& vin)
                    : k(std::forward<K>(kin))
                    , v(std::forward<V>(vin)) {}
        };
        using storage_t = Storage<kv>;
        storage_t storage;
 
        // TODO: adl upgrade
        using iterator = decltype(std::begin(std::declval<storage_t&>()));
        using const_iterator = decltype(std::begin(std::declval<const storage_t&>()));
        // boilerplate:
        iterator begin() {
                using std::begin;
                return begin(storage);
        }
        const_iterator begin() const {
                using std::begin;
                return begin(storage);
        }
        const_iterator cbegin() const {
                using std::begin;
                return begin(storage);
        }
        iterator end() {
                using std::end;
                return end(storage);
        }
        const_iterator end() const {
                using std::end;
                return end(storage);
        }
        const_iterator cend() const {
                using std::end;
                return end(storage);
        }
        size_t size() const { return storage.size(); }
        bool empty() const { return storage.empty(); }
        // these only have to be valid if called:
        void reserve(size_t n) { storage.reserve(n); }
        size_t capacity() const { return storage.capacity(); }
        // map-like interface:
        // TODO: SFINAE check for type of key
        template <class K>
        Value& operator[](K&& k) {
                auto it = find(k);
                if(it != end())
                        return it->v;
                storage.emplace_back(std::forward<K>(k), Value{});
                return storage.back().v;
        }
 
private: // C++14, but you can just inject the lambda at point of use in 11:
        template <class K>
        auto key_match(K& k) {
                return [&k](kv const& kv) { return kv.k == k; };
        }
 
public:
        template <class K>
        iterator find(K&& k) {
                return std::find_if(begin(), end(), key_match(k));
        }
        template <class K>
        const_iterator find(K&& k) const {
                return const_cast<flat_map*>(this)->find(k);
        }
   
        // iterator-less query functions:
        template <class K>
        Value* get(K&& k) {
                auto it = find(std::forward<K>(k));
                if(it == end())
                        return nullptr;
                return std::addressof(it->v);
        }
        template <class K>
        Value const* get(K&& k) const {
                return const_cast<flat_map*>(this)->get(std::forward<K>(k));
        }
   
        // key-based erase: (SFINAE should be is_comparible, but that doesn't exist)
        template <class K, class = std::enable_if_t<std::is_convertible<K, Key>{}> >
        bool erase(K&& k) {
                auto it = std::remove(storage.begin(), storage.end(), key_match(std::forward<K>(k)));
                if(it == storage.end())
                        return false;
                storage.erase(it, storage.end());
                return true;
        }
        // classic erase, for iterating:
        iterator erase(const_iterator it) { return storage.erase(it); }
   
        template <class K2, class V2, class = std::enable_if_t<std::is_convertible<K2, Key>{} && std::is_convertible<V2, Value>{}> >
        void set(K2&& kin, V2&& vin) {
                auto it = find(kin);
                if(it != end()) {
                        it->second = std::forward<V2>(vin);
                        return;
                } else {
                        storage.emplace_back(std::forward<K2>(kin), std::forward<V2>(vin));
                }
        }
};
 
// TODO:
// - other map members
// - maybe try to separate keys and values in 2 different containers
// - add comparator for keys (for equality! not less than!) as template argument
// - add sfinae enable_if checks for everything using template <class K>
// - Removing element from unsorted vector is O(1) - just swap it with .back() and then .pop_back()
 
#include <iostream>
#include <vector>
#include <map>
 
#include "boost/container/flat_map.hpp"
#include "boost/container/small_vector.hpp"
#include "boost/container/static_vector.hpp"
 
using namespace std;
 
using namespace boost;
 
template <class T>
using mySmallVec = boost::container::small_vector<T, 1>;
 
int main() {
    flat_map<int, int, mySmallVec> a;
   
    a[5] = 1;
    a[8] = 1;
    a[2] = 1;
    a[2] = 1;
    a[2] = 1;
   
    //cout << a.size() << endl;
   
    cout << sizeof(a) << endl;
    //cout << sizeof(flat_map<int, int>::storage) << endl;
    cout << sizeof(flat_map<int, int, mySmallVec>::storage) << endl;
    cout << sizeof(container::small_vector<flat_map<int, int>::kv, 1>) << endl;
    cout << sizeof(container::small_vector<int, 3>) << endl;
    cout << sizeof(container::static_vector<int, 3>) << endl;
    //cout << sizeof(container::small_vector<int, 2>) << endl;
    //cout << sizeof(container::small_vector<int, 3>) << endl;
   
    //cout << sizeof(std::vector<int>) << endl;
    //cout << sizeof(std::map<int, int>) << endl << endl;
    //cout << sizeof(container::flat_map<int, int>) << endl;
    //cout << sizeof(container::static_vector<int, 3>) << endl;
   
    return 0;
}