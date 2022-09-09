#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <cstring>
#include <assert.h>

namespace Servo
{
    static const char _magic[] = "servopkt";
    static const char _cmd_start[] = "start";
    static const char _cmd_stop[] = "stop";
    static const char _cmd_torque[] = "torque";
    static const int _magic_len = 8;
    static const int _cmd_len = 8;

    enum Cmd
    {
        CmdInvalid = -1,
        CmdStart = 1,
        CmdStop = 2,
        CmdTorque = 3,
    };

    struct alignas(8) InfoPack
    {
        char magic[_magic_len];
        int64_t t;
        double theta;
        double dtheta;
    };

    struct alignas(8) CmdPack
    {
        char magic[_magic_len];
        char cmd[_cmd_len];
        int64_t t;
        double torque;
    };

    static_assert(sizeof(_magic) - 1 <= sizeof(InfoPack::magic), "incorrect _magic");
    static_assert(sizeof(_cmd_torque) - 1 <= sizeof(CmdPack::cmd), "incorrect _cmd_torque");
    static_assert(sizeof(_cmd_stop) - 1 <= sizeof(CmdPack::cmd), "incorrect _cmd_stop");
    static_assert(sizeof(_cmd_start) - 1 <= sizeof(CmdPack::cmd), "incorrect _cmd_start");

    inline void 
    set_cmd(CmdPack& pack, Cmd cmd_type)
    {
        switch (cmd_type)
        {
        case CmdStart: std::memcpy(pack.cmd, _cmd_start, sizeof(_cmd_start) - 1); break;
        case CmdStop: std::memcpy(pack.cmd, _cmd_stop, sizeof(_cmd_stop) - 1); break;
        case CmdTorque: std::memcpy(pack.cmd, _cmd_torque, sizeof(_cmd_torque) - 1); break;
        default: assert(false);
        };
    }

    inline Cmd 
    get_cmd(CmdPack const& pack)
    {
        if (std::memcmp(pack.cmd, _cmd_start, sizeof(_cmd_start) - 1) == 0)
            return CmdStart;
        if (std::memcmp(pack.cmd, _cmd_stop, sizeof(_cmd_stop) - 1) == 0)
            return CmdStop;
        if (std::memcmp(pack.cmd, _cmd_torque, sizeof(_cmd_torque) - 1) == 0)
            return CmdTorque;
        return CmdInvalid;
    }

    inline void 
    set_magic(CmdPack& pack)
    {
        static_assert(sizeof(_magic) - 1 <= sizeof(CmdPack::magic), "incorrect Servo::_magic");
        memcpy(pack.magic, _magic, sizeof(_magic) - 1);
    }

    inline void 
    set_magic(InfoPack& pack)
    {
        static_assert(sizeof(_magic) - 1 <= sizeof(InfoPack::magic), "incorrect Servo::_magic");
        memcpy(pack.magic, _magic, sizeof(_magic) - 1);
    }

    inline bool
    cmp_magic(CmdPack const& pack)
    {
        static_assert(sizeof(_magic) - 1 <= sizeof(CmdPack::magic), "incorrect _magic");
        return memcmp(pack.magic, _magic, sizeof(_magic) - 1) == 0 ? true : false;
    }

    inline bool
    cmp_magic(InfoPack const& pack)
    {
        static_assert(sizeof(_magic) - 1 <= sizeof(InfoPack::magic), "incorrect _magic");
        return memcmp(pack.magic, _magic, sizeof(_magic) - 1) == 0 ? true : false;
    }

    inline bool 
    verify_pack(InfoPack const& pack)
    {
        return cmp_magic(pack);
    }

    inline bool 
    verify_pack(CmdPack const& pack)
    {
        return cmp_magic(pack);
    }

    inline void
    init_info_pack(
        int64_t const& t,
        double const& theta, 
        double const& dtheta, 
        InfoPack& pack
        )
    {
        set_magic(pack);
        pack.t = t;
        pack.theta = theta;
        pack.dtheta = dtheta;
    }

    inline bool
    serialize_info(
        int64_t const& t,
        double const& theta, 
        double const& dtheta, 
        char* buf,
        int bufsz
        )
    {
        if (bufsz < (int)sizeof(InfoPack))
            return false;

        InfoPack* p = reinterpret_cast<InfoPack*>(buf);
        init_info_pack(t, theta, dtheta, *p);
        return true;
    }

    inline void
    serialize_info(
        int64_t const& t,
        double const& theta, 
        double const& dtheta, 
        std::vector<uint8_t>& buf
        )
    {
        buf.resize(sizeof(InfoPack));
        InfoPack* p = reinterpret_cast<InfoPack*>(&buf[0]);
        init_info_pack(t, theta, dtheta, *p);
    }

    inline bool
    deserialize_info(char const* buf, int len, InfoPack& result)
    {
        if (len < (int)sizeof(InfoPack))
            return false;

        InfoPack const* p = reinterpret_cast<InfoPack const*>(buf);
        if (!cmp_magic(*p))
            return false;

        result = *p;
        return true;
    }

    inline bool
    deserialize_info(std::vector<uint8_t> const& buf, InfoPack& result)
    {
        return deserialize_info(reinterpret_cast<char const*>(&buf[0]), buf.size(), result);
    }

    inline void
    serialize_cmd_start(
        int64_t const& t,
        std::vector<uint8_t>& buf
        )
    {
        buf.resize(sizeof(CmdPack));
        CmdPack* p = reinterpret_cast<CmdPack*>(&buf[0]);
        set_magic(*p);
        set_cmd(*p, CmdStart);
        p->t = t;
        p->torque = 0.0;
    }

    inline void
    init_cmd_start(
        int64_t const& t,
        CmdPack& pack
        )
    {
        set_magic(pack);
        set_cmd(pack, CmdStart);
        pack.t = t;
        pack.torque = 0.0;
    }

    inline void
    serialize_cmd_stop(
        int64_t const& t,
        std::vector<uint8_t>& buf
        )
    {
        buf.resize(sizeof(CmdPack));
        CmdPack* p = reinterpret_cast<CmdPack*>(&buf[0]);
        set_magic(*p);
        set_cmd(*p, CmdStop);
        p->t = t;
        p->torque = 0.0;
    }

    inline void
    serialize_cmd_torque(
        int64_t const& t,
        double const& torque,
        std::vector<uint8_t>& buf
        )
    {
        buf.resize(sizeof(CmdPack));
        CmdPack* p = reinterpret_cast<CmdPack*>(&buf[0]);
        set_magic(*p);
        set_cmd(*p, CmdTorque);
        p->t = t;
        p->torque = torque;
    }

    inline void
    init_cmd_torque(
        int64_t const& t,
        double const& torque,
        CmdPack& pack
        )
    {
        set_magic(pack);
        set_cmd(pack, CmdTorque);
        pack.t = t;
        pack.torque = torque;
    }

    inline bool
    deserialize_cmd(
        char const* buf, 
        int len, 
        CmdPack& result
        )
    {
        if (len < (int)sizeof(CmdPack))
            return false;

        CmdPack const* p = reinterpret_cast<CmdPack const*>(buf);
        if (!cmp_magic(*p))
            return false;

        result = *p;
        return true;
    }

    inline bool
    deserialize_cmd(
        std::vector<uint8_t> const& buf,
        CmdPack& result
        )
    {
        return deserialize_cmd(reinterpret_cast<char const*>(&buf[0]), buf.size(), result);
    }

}
