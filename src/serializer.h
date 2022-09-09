#pragma once
#include <stdint.h>
#include <string.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <map>
#include <assert.h>
#include <memory.h>
#include <memory>
#include <functional>


namespace ser
{
    /*
    packet:
        magic           4b
        packet size     4b
        [variables]     xx

    numeric variable:
        type            1b
        hash of name    4b
        value           xx

    string variable:
        type            1n
        hash of name    4b
        length          4b
        data            xx
    */


    /*
     * API
     */

    template <typename ... Vars>
    inline int get_packed_size(Vars const&... vars);

    template <typename ... Vars>
    inline int pack(char* buf, int buf_len, Vars const&... vars);

    template <typename ... Vars>
    inline int pack(std::string& result, Vars const&... vars);

    template <typename ... Vars>
    inline int unpack(char const* buf, int buf_len, Vars&... result);

    inline int get_buf_pack_size(char const* buf, int buf_len);

    class Packet;
    class PacketReader;
    typedef std::shared_ptr<PacketReader> PacketReaderPtr;

    typedef std::function<int(char*,int)> PacketReaderCb;
    typedef std::shared_ptr<PacketReaderCb> PacketReaderCbPtr;

    template <typename Fun>
    inline PacketReaderPtr make_pack_reader(Fun&& f);


    /*
     * hash
     */

    static const uint32_t OFFSET_BASIS = 2166136261u;
    static const uint32_t FNV_PRIME = 16777619u;

    inline uint32_t fnv_hash(const char* s)
    {
       uint32_t hash = OFFSET_BASIS;
       while (*s != '\0')
       {
           hash ^= *s;
           hash *= FNV_PRIME;
           ++s;
       }
       return hash;
    }

    struct const_str_t {
        char const* s;
        int len;
    };

    enum VarID {
        ID_int8 = 1,
        ID_uint8 = 2,
        ID_int32 = 3,
        ID_uint32 = 4,
        ID_int64 = 5,
        ID_uint64 = 6,
        ID_float = 7,
        ID_double = 8,
        ID_string = 9,
    };

    static const char* magic = "MGic";
    static const int pack_header_size = (int)strlen(magic) + sizeof(uint32_t);

    inline std::tuple<int8_t, uint8_t, int>   get_numeric_type(int8_t x) { return std::make_tuple(x, ID_int8, (int)sizeof(x)); }
    inline std::tuple<uint8_t, uint8_t, int>  get_numeric_type(uint8_t x) { return std::make_tuple(x, ID_uint8, (int)sizeof(x)); }
    inline std::tuple<int32_t, uint8_t, int>  get_numeric_type(int32_t x) { return std::make_tuple(x, ID_int32, (int)sizeof(x)); }
    inline std::tuple<uint32_t, uint8_t, int> get_numeric_type(uint32_t x) { return std::make_tuple(x, ID_uint32, (int)sizeof(x)); }
    inline std::tuple<int64_t, uint8_t, int>  get_numeric_type(int64_t x) { return std::make_tuple(x, ID_int64, (int)sizeof(x)); }
    inline std::tuple<uint64_t, uint8_t, int> get_numeric_type(uint64_t x) { return std::make_tuple(x, ID_uint64, (int)sizeof(x)); }
    inline std::tuple<float, uint8_t, int>    get_numeric_type(float x) { return std::make_tuple(x, ID_float, (int)sizeof(x)); }
    inline std::tuple<double, uint8_t, int>   get_numeric_type(double x) { return std::make_tuple(x, ID_double, (int)sizeof(x)); }

    inline int get_numeric_type_sz(uint8_t type_id)
    {
        switch (type_id)
        {
        case ID_int8: return sizeof(int8_t);
        case ID_uint8: return sizeof(uint8_t);
        case ID_int32: return sizeof(int32_t);
        case ID_uint32: return sizeof(uint32_t);
        case ID_int64: return sizeof(int64_t);
        case ID_uint64: return sizeof(uint64_t);
        case ID_float: return sizeof(float);
        case ID_double: return sizeof(double);
        default: return -1;
        }
    }


    /*
     * serialization
     */

    struct ostream_t
    {
        char* s;
        int rest;

        ostream_t(char* s, int len) : s(s), rest(len) {}

        inline intptr_t sub(ostream_t const& s2)
        {
            return (intptr_t)s - (intptr_t)s2.s;
        }

        template <typename T>
        inline bool put(T const& var)
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "not implemented");

            if (rest < sizeof(T))
                return false;

            *reinterpret_cast<T*>(s) = var;
            s += sizeof(T);
            rest -= sizeof(T);
            return true;
        }

        inline bool put(char const* str, int len)
        {
            if (rest < len)
                return false;
            memcpy(s, str, len);
            s += len;
            rest -= len;
            return true;
        }

        inline bool put(char const* str)
        {
            return put(str, (int)strlen(str));
        }

        template <typename T>
        inline bool append_var(char const* name, T const& _value)
        {
            auto const& value = get_numeric_type(_value);
            if (!put(std::get<1>(value)))
                return false;
            if (!put(fnv_hash(name)))
                return false;
            if (!put(std::get<0>(value)))
                return false;
            return true;
        }

        inline bool append_var(char const* name, char const* str)
        {
            if (!put<uint8_t>(ID_string))
                return false;
            if (!put(fnv_hash(name)))
                return false;
            if (!put((uint32_t)strlen(str)))
                return false;
            if (!put(str))
                return false;
            return true;
        }

        inline bool append_var(char const* name, std::string const& str)
        {
            if (!put<uint8_t>(ID_string))
                return false;
            if (!put(fnv_hash(name)))
                return false;
            int sz = (int)str.size();
            if (!put(uint32_t(sz)))
                return false;
            if (!put(str.data(), sz))
                return false;
            return true;
        }

        inline bool append_var(char const* name, const_str_t const& str)
        {
            if (!put<uint8_t>(ID_string))
                return false;
            if (!put(fnv_hash(name)))
                return false;
            if (!put(uint32_t(str.len)))
                return false;
            if (!put(str.s, str.len))
                return false;
            return true;
        }

        template <typename T, typename ... Etc>
        inline bool append_var_list(char const* name, T const& val, Etc const&... etc)
        {
            if (!append_var(name, val))
                return false;
            if (!append_var_list(etc...))
                return false;
            return true;
        }

        template <typename T>
        inline bool append_var_list(char const* name, T const& val)
        {
            return append_var(name, val);
        }
    };

    template <typename T>
    inline int get_var_size(char const* name, T const& value)
    {
        auto t = get_numeric_type(value);
        return sizeof(uint8_t) + sizeof(uint32_t) + std::get<2>(t);
    }

    inline int get_var_size(char const* name, char const* value)
    {
        return int(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + strlen(value));
    }

    inline int get_var_size(char const* name, std::string const& s)
    {
        return int(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + s.size());
    }

    inline int get_var_size(char const* name, const_str_t const& s)
    {
        return int(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + s.len);
    }

    inline int get_var_list_size()
    {
        return 0;
    }

    template <typename T, typename ... Etc>
    inline int get_var_list_size(char const* name, T const& value, Etc const&... etc)
    {
        return get_var_size(name, value) + get_var_list_size(etc...);
    }

    template <typename ... Vars>
    inline int get_packed_size(Vars const&... vars)
    {
        return (int)strlen(magic) +
            (int)sizeof(uint32_t) +
            get_var_list_size(vars...);
    }

    template <typename ... Vars>
    inline int pack(char* buf, int buf_len, Vars const&... vars)
    {
        ostream_t s(buf, buf_len);

        // write magic
        ostream_t s_bgn = s;
        if (!s.put(magic))
            return -1;

        // write size
        ostream_t s_sz = s;
        if (!s.put<uint32_t>(0))
            return -1;

        // append vars
        if (!s.append_var_list(vars...))
            return -1;

        // fix size
        ostream_t s_end = s;
        uint32_t sz = (uint32_t)s_end.sub(s_bgn);
        s = s_sz;
        if (!s.put(sz))
            return -1;

        return sz;
    }

    template <typename ... Vars>
    inline int pack(std::string& result, Vars const&... vars)
    {
        int sz = get_packed_size(vars...);
        if (sz < 0)
            return sz;
        result.resize(sz);
        return pack(const_cast<char*>(result.data()), (int)result.size(), vars...);
    }

    template <typename Writer, typename ... Vars>
    inline int send_pack(Writer& writer, Vars const&... vars)
    {
        std::string buf;
        int sz = pack(buf, vars...);
        if (sz <= 0)
            return sz;
        return writer(buf.data(), sz);
    }


    /*
     * deserialization
     */

    struct istream_t
    {
        char const* s;
        int rest;

        istream_t() : s(nullptr), rest(0) {}

        istream_t(char const* s, int len) : s(s), rest(len) {}

        inline intptr_t sub(istream_t const& s2)
        {
            return (intptr_t)s - (intptr_t)s2.s;
        }

        template <typename T>
        int fetch(T& result)
        {
            if (rest < sizeof(T))
                return -1;

            result = *reinterpret_cast<T const*>(s);
            s += sizeof(T);
            rest -= sizeof(T);
            return 1;
        }

        template <typename T>
        int peek(T& result)
        {
            if (rest < sizeof(T))
                return -1;

            result = *reinterpret_cast<T const*>(s);
            return 1;
        }

        int fetch_and_compare(char const* c, int len)
        {
            if (rest < len)
                return -1;

            int result = memcmp(c, s, len);
            s += len;
            rest -= len;
            return result == 0 ? 1 : 0;
        }

        int fetch_and_compare(char const* c)
        {
            return fetch_and_compare(c, int(strlen(c)));
        }

        inline int fetch_var(uint32_t& name_hash, const_str_t& str)
        {
            int status;
            uint8_t type_id;

            status = fetch(type_id);
            if (status <= 0)
                return status;

            if (type_id != ID_string)
                return -1;

            status = fetch(name_hash);
            if (status <= 0)
                return status;

            uint32_t sz;
            status = fetch(sz);
            if (status <= 0)
                return status;

            if ((int)sz < rest)
                return -1;

            str.s = s;
            str.len = sz;
            return 1;
        }

        inline int fetch_var(uint32_t& name_hash, std::string& str)
        {
            const_str_t _str;
            int status = fetch_var(name_hash, _str);
            if (status <= 0)
                return status;
            str.clear();
            str.append(_str.s, _str.len);
            return 1;
        }

        template <typename T>
        inline int fetch_var(uint32_t& name_hash, T& var)
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "not implemented");

            int status;
            uint8_t type_id;

            status = fetch(type_id);
            if (status <= 0)
                return status;

            if (std::get<1>(get_numeric_type(var)) != type_id)
                return -1;

            status = fetch(name_hash);
            if (status <= 0)
                return status;

            status = fetch(var);
            if (status <= 0)
                return status;

            return 1;
        }

        int get_var_size(uint8_t type_id)
        {
            if (type_id == ID_string)
            {
                uint32_t len;
                peek(len);
                return sizeof(len) + len;
            }
            else
            {
                return get_numeric_type_sz(type_id);
            }
        }

        inline int skip_var(uint32_t& name_hash)
        {
            uint8_t type_id;
            int status = fetch(type_id);
            if (status <= 0)
                return status;

            status = fetch(name_hash);
            if (status <= 0)
                return status;

            int sz = get_var_size(type_id);
            if (sz <= 0)
                return sz;

            if (sz > rest)
                return -1;

            s += sz;
            rest -= sz;
            return 1;
        }

        inline int skip_var(uint32_t& name_hash, istream_t& var_loc)
        {
            var_loc.s = s;
            int status = skip_var(name_hash);
            if (status <= 0)
                return status;
            var_loc.rest = int(intptr_t(s) - intptr_t(var_loc.s));
            return 1;
        }
    };

    inline int extract_values(std::map<uint32_t, istream_t> const& entries)
    {
        return 1;
    }

    template <typename T>
    inline int extract_values(std::map<uint32_t, istream_t> const& entries, char const* var_name, T& var_value)
    {
        uint32_t name_hash = fnv_hash(var_name);
        auto it = entries.find(name_hash);
        if (it == entries.end())
            return -1;

        istream_t s = it->second;
        return s.fetch_var(name_hash, var_value);
    }

    template <typename T, typename ... Etc>
    inline int extract_values(std::map<uint32_t, istream_t> const& entries, char const* var_name, T& var_value, Etc&... etc)
    {
        uint32_t name_hash = fnv_hash(var_name);
        auto it = entries.find(name_hash);
        if (it == entries.end())
            return -1;

        istream_t s = it->second;
        int status = s.fetch_var(name_hash, var_value);
        if (status <= 0)
            return status;
        return extract_values(entries, etc...);
    }

    inline int get_buf_pack_size(char const* buf, int buf_len)
    {
        istream_t s(buf, buf_len);

        // read magic
        if (s.fetch_and_compare(magic) <= 0)
            return -1;

        uint32_t sz;
        if (s.fetch(sz) <= 0)
            return -1;

        return int(sz);
    }

    template <typename ... Vars>
    inline int unpack(char const* buf, int buf_len, Vars&... result)
    {
        int status;
        istream_t s(buf, buf_len);
        istream_t s_bgn = s;

        // read magic
        if (s.fetch_and_compare(magic) <= 0)
            return -1;

        // read size
        uint32_t sz;
        if (s.fetch(sz) <= 0)
            return -1;

        if ((int)sz > buf_len)
            return -1;

        // extract names
        std::map<uint32_t, istream_t> entries;

        while ((int)s.sub(s_bgn) < (int)sz)
        {
            uint32_t var_name_hash;
            istream_t var_location;
            status = s.skip_var(var_name_hash, var_location);
            if (status <= 0)
                return status;
            entries[var_name_hash] = var_location;
        }

        if ((int)s.sub(s_bgn) > (int)sz)
            return -1;

        // extract values
        status = extract_values(entries, result...);
        if (status <= 0)
            return status;

        return sz;
    }

    template <typename ... Vars>
    inline int unpack(std::string const& buf, Vars&... result)
    {
        return unpack(buf.data(), (int)buf.size(), result...);
    }

    class Packet
    {
    private:
        std::map<uint32_t, istream_t> m_entries;
        std::string m_buf;

    public:
        int parse(std::string& buffer)
        {
        	m_buf = std::move(buffer);
            istream_t s(m_buf.data(), int(m_buf.size()));
            istream_t s_bgn = s;

            // read magic
            if (s.fetch_and_compare(magic) <= 0)
                return -1;

            // read size
            uint32_t sz;
            if (s.fetch(sz) <= 0)
                return -1;

            if ((int)sz != m_buf.size())
                return -1;

            while ((int)s.sub(s_bgn) < (int)sz)
            {
                uint32_t var_name_hash;
                istream_t var_location;
                int status = s.skip_var(var_name_hash, var_location);
                if (status <= 0)
                {
                    return status;
                }
                m_entries[var_name_hash] = var_location;
            }

            if ((int)s.sub(s_bgn) > (int)sz)
                return -1;

            return 1;
        }

        template <typename Value, typename ... Vars>
        inline int get(char const* name, Value& val, Vars&... result)
        {
            return extract_values(m_entries, name, val, result...);
        }

        template <typename T>
        inline T get(char const* name)
        {
        	T result;
        	int status = extract_values<T>(m_entries, name, result);
        	if (status < 0)
        		return T();
        	return result;
        }

        inline int nentries() const
        {
            return m_entries.size();
        }
    };


    class PacketReader
    {
    private:
        static const int    m_buf_total = 8192;
        char                m_buf[m_buf_total];
        int                 m_buf_filled;
        PacketReaderCbPtr   m_reader;

        void rotate_buf(int offset)
        {
            assert(offset <= m_buf_filled);
            memmove(m_buf, m_buf + offset, m_buf_filled - offset);
            m_buf_filled = m_buf_filled - offset;
        }

        int fill_buffer()
        {
        	if (m_buf_filled == m_buf_total)
        		return 0;

            int len = (*m_reader)(m_buf + m_buf_filled, m_buf_total - m_buf_filled);
            if (len <= 0)
                return len;

            m_buf_filled += len;
            return len;
        }


    public:
        PacketReader(PacketReaderCbPtr reader) : m_buf_filled(0), m_reader(reader) {}

        PacketReader(PacketReader const& src) = delete;

        PacketReader(PacketReader&& src)
        {
            memcpy(this->m_buf, src.m_buf, src.m_buf_filled);
            this->m_buf_filled = src.m_buf_filled;
            this->m_reader = src.m_reader;

            src.m_reader = nullptr;
        }

        int fetch_next(Packet& result)
        {
            int status;

            if (m_buf_filled < pack_header_size)
            {
                status = fill_buffer();
                if (status <= 0)
                    return status;

                return fetch_next(result);
            }

            int pack_sz = get_buf_pack_size(m_buf, m_buf_filled);
            if (pack_sz <= 0)
                return pack_sz;

            if (pack_sz > m_buf_filled)
            {
                if (pack_sz > m_buf_total)
                {
                	// TODO not implemented
                	return -1;
                }

                status = fill_buffer();
                if (status <= 0)
                    return status;

                if (pack_sz > m_buf_filled)
                    return 0;
            }

            std::string packet_buf(m_buf, pack_sz);
            int extracted = result.parse(packet_buf);
            if (extracted <= 0)
                return extracted;

            rotate_buf(pack_sz);
            return 1;
        }
    };

    template <typename Fun>
    inline PacketReaderPtr make_pack_reader(Fun&& f)
    {
        PacketReaderCbPtr cb = std::make_shared<PacketReaderCb>(f);
        return std::make_shared<PacketReader>(cb);
    }
}
