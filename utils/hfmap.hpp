#ifndef HFMAP_HPP
#define HFMAP_HPP
#include <algorithm>
#include <boost/noncopyable.hpp>
#include<boost/assert.hpp>
#include<boost/unordered_map.hpp>
#include<boost/thread.hpp>

template<typename Key,typename Value>
class safe_map:boost::noncopyable
{
    typedef boost::unordered_map<Key,Value> map_type;
    typedef boost::shared_mutex rw_mutex;
    typedef boost::shared_lock<rw_mutex> read_lock;
    typedef boost::unique_lock<rw_mutex> write_lock;
public:
    typedef typename map_type::key_type key_type;
    typedef typename map_type::mapped_type mapped_type;
    typedef typename map_type::value_type value_type;

    typedef typename map_type::pointer pointer;
    typedef typename map_type::const_pointer const_pointer;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::difference_type difference_type;
    typedef typename map_type::reference reference;
    typedef typename map_type::const_reference const_reference;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;
private:
    map_type m_map;
    mutable rw_mutex m_mutex;
public:
    boos empty() const
    {
        read_lock lock(m_mutex);
        return m_map.empty();
    }

    size_type size() const
    {
        read_lock lock(m_mutex);
        return m_map.size();
    }

    size_type max_size() const
    {
        read_lock lock(m_mutex);
        return m_map.max_size();
    }

    iterator begin()
    {
        read_lock lock(m_mutex);
        return m_map.begin();
    }

    const_iterator begin() const
    {
        read_lock lock(m_mutex);
        return m_map.begin();
    }

    const_iterator end() const
    {
        read_lock lock(m_mutex);
        return m_map.end();
    }

    iterator end()
    {
        read_lock lock(m_mutex);
        return m_map.end();
    }

    bool insert(const key_type&k,const mapped_type&v)
    {
        write_lock lock(m_mutex);
        return m_map.insert(value_type(k,v)).second;
    }

    bool find(const key_type& k)
    {
        read_lock lock(m_mutex);
        return m_map.find(k) != m_map.end();
    }

    size_type erase(const key_type&  k)
    {
        write_lock lock(m_mutex);
        return m_map.erase(k);
    }

    void clear()
    {
        write_lock lock(m_mutex);
        m_map.clear();
    }

    typedef boost::optional<mapped_type> optional_mapped_type;
    optional_mapped_type at(const key_type&k)
    {
        read_lock lock(m_mutex);
        if(m_map.find(k) != m_map.end())
        {
            return optional_mapped_type(m_map[k]);
        }
        return optional_mapped_type();
    }

    const map_type& operator[](const key_type& k)
    {
        read_lock lock(m_mutex);
        BOOST_ASSERT(m_map.find(k) != m_map.end());
        return m_map[k];
    }

    void set(const key_type&k,const mapped_type& v)
    {
        write_lock lock(m_mutex);
        m_map[k] = v;
    }

    template<typename Func>
    void for_each(Func func)
    {
        read_lock lock(m_mutex);
        std::for_each(m_map.begin(),m_map.end(),func);
    }
};

#endif // HFMAP_HPP

