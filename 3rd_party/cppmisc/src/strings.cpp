#include <cppmisc/strings.h>


using namespace std;

bool __match_mask(string const& name, string const& mask, int iname, int imask)
{
    while (true)
    {
        if (iname == (int)name.size())
            break;

        if (imask == (int)mask.size())
            return false;

        char c = mask[imask];

        switch (c)
        {
        case '*':
            for (int iname1 = iname; iname1 <= (int)name.size(); ++ iname1)
            {
                if (__match_mask(name, mask, iname1, imask + 1))
                    return true;
            }
            return false;
        case '?':
            ++ iname;
            ++ imask;
            break;
        default:
            if (c != name[iname])
                return false;
            ++ iname;
            ++ imask;
            break;
        }
    }

    if (imask == (int)mask.size())
        return true;

    return false;
}

bool match_mask(string const& filename, string const& mask)
{
    return __match_mask(filename, mask, 0, 0);
}

string get_digits_substr(string const& s)
{
    int lim1 = 0, lim2 = 0;

    for (int i = 0; i < (int)s.size(); ++i)
    {
        if (isdigit(s[i]))
        {
            int start = i;
            while (i < (int)s.size() && isdigit(s[i]))
                ++i;

            int end = i;

            if (lim2 - lim1 < end - start)
            {
                lim1 = start;
                lim2 = end;
            }
        }
    }

    return s.substr(lim1, lim2);
}
