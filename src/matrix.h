#pragma once
#include <array>



template <int Ny, int Nx, typename T>
class Mat
{
private:
    static const int N = Nx * Ny;
    std::array<T, Nx * Ny> _elems;

public:
    Mat()
    {
    }

    Mat(std::initializer_list<T> const& l)
    {
        fill(0);
        std::copy(l.begin(), l.end(), _elems.begin());
    }

    template <typename Arg0, typename... Args>
    Mat(Arg0 arg0, Args... args)
    {
        set(arg0, args...);
    }

    Mat(Mat const& a)
    {
        _elems = a._elems;
    }

    ~Mat()
    {
    }

    void fill(T const& val)
    {
        _elems.fill(val);
    }

    template <int idx = 0, typename Arg0, typename... Args>
    inline void set(Arg0 arg0, Args... args)
    {
        _elems[idx] = static_cast<T>(arg0);
        set<idx + 1>(args...);
    }

    template <int>
    inline void set() {}

    inline T& at(int y, int x = 0)
    {
        return _elems[y * Nx + x];
    }

    inline T const& at(int y, int x = 0) const
    {
        return _elems[y * Nx + x];
    }

    inline Mat mul(T const& a) const
    {
        Mat res;

        for (int i = 0; i < N; ++i)
            res._elems[i] = _elems[i] * a;

        return res;
    }

    inline Mat add(T const& a) const
    {
        Mat res;

        for (int i = 0; i < N; ++i)
            res._elems[i] = _elems[i] + a;

        return res;
    }

    inline Mat add(Mat const& m) const
    {
        Mat res;

        for (int i = 0; i < N; ++i)
            res._elems[i] = _elems[i] + m._elems[i];

        return res;
    }

    inline Mat sub(Mat const& m) const
    {
        Mat res;

        for (int i = 0; i < N; ++i)
            res._elems[i] = _elems[i] - m._elems[i];

        return res;
    }

    inline T& operator ()(int y, int x = 0)
    {
        return at(y, x);
    }

    inline T const& operator ()(int y, int x = 0) const
    {
        return at(y, x);
    }

    Mat<Nx, Ny, T> t() const
    {
        Mat<Nx, Ny, T> transposed;

        for (int y = 0; y < Ny; ++y)
            for (int x = 0; x < Nx; ++x)
                transposed(x,y) = at(y,x);

        return transposed;
    }
};


template <int Ny1, int Nx1, int Nx2, typename T>
inline Mat<Ny1, Nx2, T> mul(Mat<Ny1, Nx1, T> const& A, Mat<Nx1, Nx2, T> const& B)
{
    Mat<Ny1, Nx2, T> C;

    C.fill(static_cast<T>(0));

    for (int y = 0; y < Ny1; ++y)
        for (int x = 0; x < Nx2; ++x)
            for (int k = 0; k < Nx1; ++k)
                C.at(y, x) += A.at(y, k) * B.at(k, x);

    return C;
}

template <int Ny, int Nx, typename T>
inline Mat<Ny, Nx, T> mul(Mat<Ny, Nx, T> const& A, T const& k)
{
    Mat<Ny, Nx, T> C;

    C.fill(static_cast<T>(0));

    for (int y = 0; y < Ny; ++y)
        for (int x = 0; x < Nx; ++x)
            C.at(y, x) = A.at(y, k) * k;

    return C;
}

template <int Ny, int Nx, typename T>
inline Mat<Ny, Nx, T> operator + (Mat<Ny, Nx, T> const& A, Mat<Ny, Nx, T> const& B)
{
    return A.add(B);
}

template <int Ny, int Nx, typename T>
inline Mat<Ny, Nx, T> operator - (Mat<Ny, Nx, T> const& A, Mat<Ny, Nx, T> const& B)
{
    return A.sub(B);
}

template <int Ny1, int Nx1, int Nx2, typename T>
inline Mat<Ny1, Nx2, T> operator * (Mat<Ny1, Nx1, T> const& A, Mat<Nx1, Nx2, T> const& B)
{
    return mul(A, B);
}

template <int Ny, int Nx, typename T>
inline Mat<Ny, Nx, T> operator * (Mat<Ny, Nx, T> const& A, T const& k)
{
    return mul(A, k);
}

template <int Ny, int Nx, typename T>
inline Mat<Ny, Nx, T> operator * (T const& k, Mat<Ny, Nx, T> const& A)
{
    return mul(A, k);
}

template <typename T>
inline T det(Mat<2, 2, T> const& m)
{
    return m.at(0, 0) * m.at(1, 1) - m.at(0, 1) * m.at(1, 0);
}

template <typename T>
inline Mat<2, 2, T> inv(Mat<2, 2, T> const& m)
{
    T d = det(m);
    return Mat<2, 2, T>(m.at(1, 1) / d, -m.at(0, 1) / d, -m.at(1, 0) / d, m.at(0, 0) / d);
}

template <int N, typename T>
inline T dot(Mat<N, 1, T> const& v1, Mat<N, 1, T> const& v2)
{
    T res = 0;

    for (int x = 0; x < N; ++x)
        res += v1.at(x) * v2.at(x);

    return res;
}

template <typename T>
inline T cross(Mat<2, 1, T> const& u, Mat<2, 1, T> const& v)
{
    return u.at(0) * v.at(1) - u.at(1) * v.at(0);
};

template <typename T>
inline Mat<3, 1, T> cross(Mat<3, 1, T> const& u, Mat<3, 1, T> const& v)
{
    return Mat<3, 1, T>(
        u.at(1) * v.at(2) - u.at(2) * v.at(1),
        -u.at(0) * v.at(2) + u.at(2) * v.at(0),
        u.at(0) * v.at(1) - u.at(1) * v.at(0)
        );
}

template <int Ny, int Nx, typename T>
std::ostream& operator << (std::ostream& s, Mat<Ny, Nx, T> const& m)
{
    for (int y = 0; y < Ny; ++y)
    {
        for (int x = 0; x < Nx; ++x)
            s << m.at(y, x) << ' ';

        s << std::endl;
    }

    return s;
}


using Mat1x2 = Mat<1, 2, double>;
using Mat2x1 = Mat<2, 1, double>;
using Mat2x2 = Mat<2, 2, double>;
using Mat3x3 = Mat<3, 3, double>;
using Vec2 = Mat<2, 1, double>;
using Vec3 = Mat<3, 1, double>;
