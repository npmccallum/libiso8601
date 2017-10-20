[![Travis](https://img.shields.io/travis/npmccallum/libiso8601.svg)]()
[![Codecov](https://img.shields.io/codecov/c/github/npmccallum/libiso8601.svg)]()

This is a C simple library for parsing and managing ISO 8601 dates and times.
It is extensively tested (check out our test coverage!) and is available under
the Apache 2.0 license. We also believe that it will successfully parse all
variants of the specification as well as many common non-conformant formats.

Here are the two main functions:

```c
#include <iso8601.h>
#include <assert.h>
#include <string.h>

int main() {
    iso8601_time time = {};
    char str[128] = {};

    iso8601_parse("2010-02-14T13:14:23.123456Z", &time);

    assert(time.year == 2010);
    assert(time.month == 2);
    assert(time.day == 14);
    assert(time.hour == 13);
    assert(time.minute == 14);
    assert(time.second == 23);
    assert(time.usecond == 123456);

    iso8601_unparse(&time, ISO8601_FLAG_NONE, 4, ISO8601_FORMAT_WEEKDATE,
                    ISO8601_TRUNCATE_DAY, sizeof(str), str);

    assert(strcmp(str, "2010-W06-7") == 0);
    return 0;
}
```
