#include <vector>
#include <stdint.h>
#include <assert.h>
#include <algorithm>
#include <cppmisc/traces.h>


struct TimedSignal
{
    int64_t ts;
    double value;
};


class SigHistoryIteratorConst : 
    public std::iterator<std::forward_iterator_tag,TimedSignal const*>
{
private:
    int _i, _size;
    int _newest;

    std::vector<TimedSignal> const& _history;

public:
    SigHistoryIteratorConst() = delete;

    SigHistoryIteratorConst(int i0, int newest, int size, std::vector<TimedSignal> const& history) : 
        _i(i0),
        _size(size),
        _newest(newest),
        _history(history)
    {
    }

    inline SigHistoryIteratorConst& operator++()
    {
        ++ _i;
        assert(_i <= _size);
        return *this;
    }

    inline SigHistoryIteratorConst operator++(int)
    {
        SigHistoryIteratorConst copy = *this;
        operator++();
        return copy;
    }

    inline bool equal(SigHistoryIteratorConst const& other) const
    {
        return this->_i == other._i;
    }

    inline bool operator == (SigHistoryIteratorConst const& other) const
    {
        return equal(other);
    }

    inline bool operator != (SigHistoryIteratorConst const& other) const
    {
        return !equal(other);
    }

    inline void add(int n)
    {
        _i += n;
        assert(_i <= _size && _i >= 0);
    }

    inline SigHistoryIteratorConst operator + (int n) const
    {
        SigHistoryIteratorConst result = *this;
        result.add(n);
        return result;
    }

    inline SigHistoryIteratorConst operator - (int n) const
    {
        SigHistoryIteratorConst result = *this;
        result.add(-n);
        return result;
    }

    TimedSignal const& operator*() const
    {
        int k = (_newest - _i + _history.size()) % _history.size();
        return _history[k];
    }

    TimedSignal const* operator->() const
    {
        int k = (_newest - _i + _history.size()) % _history.size();
        return &_history[k];
    }
};

class SigHistoryIteratorBackwardConst : 
    public std::iterator<std::forward_iterator_tag,TimedSignal const*>
{
private:
    int _i, _size;
    int _oldest;

    std::vector<TimedSignal> const& _history;

public:
    SigHistoryIteratorBackwardConst() = delete;

    SigHistoryIteratorBackwardConst(int i0, int oldest, int size, std::vector<TimedSignal> const& history) : 
        _i(i0),
        _size(size),
        _oldest(oldest),
        _history(history)
    {
    }

    inline SigHistoryIteratorBackwardConst& operator++()
    {
        ++ _i;
        assert(_i <= _size);
        return *this;
    }

    inline SigHistoryIteratorBackwardConst operator++(int)
    {
        SigHistoryIteratorBackwardConst copy = *this;
        operator++();
        return copy;
    }

    inline bool equal(SigHistoryIteratorBackwardConst const& other) const
    {
        return this->_i == other._i;
    }

    inline bool operator == (SigHistoryIteratorBackwardConst const& other) const
    {
        return equal(other);
    }

    inline bool operator != (SigHistoryIteratorBackwardConst const& other) const
    {
        return !equal(other);
    }

    TimedSignal const& operator*() const
    {
        int k = (_oldest + _i) % _history.size();
        return _history[k];
    }

    TimedSignal const* operator->() const
    {
        int k = (_oldest + _i) % _history.size();
        return &_history[k];
    }
};

class SigHistory
{
private:
    std::vector<TimedSignal> _history;
    int _newest, _size;


    inline int oldest_idx() const
    {
        if (_size == 0)
            return _newest;
        return (_newest - _size + 1 + _history.size()) % _history.size();
    }

public:
    SigHistory(int wndsz)
    {
        assert(wndsz > 0);
        _history.resize(wndsz);
        _newest = 0, _size = 0;
    }

    void push(TimedSignal const& sig)
    {
        if (_size > 0)
            assert(sig.ts >= _history[_newest].ts);

        _newest = (_newest + 1) % _history.size();
        _history[_newest] = sig;
        _size = std::min(_size + 1, int(_history.size()));
    }

    void clear_old(int64_t ts)
    {
        for (int i = 0; i < _size; ++ i)
        {
            int k = (_newest - i + _history.size()) % _history.size();
            auto const& sig = _history[k];
            if (sig.ts <= ts)
            {
                _size = i;
                break;
            }
        }
    }

    inline int size() const
    {
        return _size;
    }

    inline TimedSignal const* newest() const
    {
        if (_size == 0)
            return nullptr;
        return &_history[_newest];
    }

    SigHistoryIteratorConst begin() const
    {
        return {0, _newest, _size, _history};
    }

    SigHistoryIteratorConst end() const
    {
        return {_size, _newest, _size, _history};
    }

    SigHistoryIteratorBackwardConst rbegin() const
    {
        int oldest = oldest_idx();
        return {0, oldest, _size, _history};
    }

    SigHistoryIteratorBackwardConst rend() const
    {
        int oldest = oldest_idx();
        return {_size, oldest, _size, _history};
    }
};


class MovingAverage
{
private:
    SigHistory _history;
    int64_t _newest;
    int64_t _period;
    double _sum;

public:
    MovingAverage(double step, double period) : 
        _history(2 * period / step),
        _newest(-1),
        _period(sec_to_usec(period)),
        _sum(0.)
    {
        assert(step > 0);
        assert(_period > step);
        dbg_msg("wnd size ", 2 * period / step);
    }

    void update(int64_t ts, double value)
    {
        assert(ts > _newest);
        _history.push({ts, value});
        _newest = ts;
        int64_t oldest = _newest - _period;
        _sum = 0.;

        for (auto i = ++_history.begin(); i != _history.end(); ++ i)
        {
            if (i->ts < oldest)
                break;
            auto j = i - 1;
            _sum += (j->value + i->value) * usec_to_sec(j->ts - i->ts) / 2;
        }
    }

    inline double value() const
    {
        return _sum / usec_to_sec(_period);
    }
};
