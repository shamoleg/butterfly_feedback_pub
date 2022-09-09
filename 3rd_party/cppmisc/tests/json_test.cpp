#include <assert.h>
#include <cppmisc/json.h>


void test_json()
{
    char data[] = R"({
        "x": 1,
        "y": [1,2,3]
    })";
    auto json = json_parse(data);

    int x;
    json_get(json, "x", x);
    assert(x == 1);
    std::vector<int> y;
    json_get(json, "y", y);
    assert(y[0] == 1 && y[1] == 2 && y[2] == 3);
}

int main(int argc, char const* argv[])
{
    return 0;
}
