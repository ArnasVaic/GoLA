#include <assert.h>
#include <frame.hpp>

using namespace std;

int main()
{
    Transform t;
    Frame<4> frame(0);
    auto normalized_frame = frame.normalized(t);
    assert(normalized_frame.get() != 0);
    assert(t.col_offset == 0);
    assert(t.row_offset == 0);
    assert(!t.flip);
    assert(t.turn_count == 0);
}