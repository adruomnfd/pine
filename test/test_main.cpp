#include <util/fileio.h>
#include <util/log.h>
#include <util/reflect.h>

using namespace pine;

#define ExpectEQ(a, b) CHECK_LT(Length(a - b), 0.001f)

struct MyStruct {
    int value;
    std::vector<float> values;
    std::shared_ptr<float> ptr0;
    std::unique_ptr<float> ptr1;
};

int main() {
    MyStruct x{5, {1, 2, 3, 4, 6}, std::make_shared<float>(7.0f), std::make_unique<float>(8.0f)};
    Serialize("data.bin", x);

    MyStruct y = Deserialize<MyStruct>("data.bin");
    print(y);
}