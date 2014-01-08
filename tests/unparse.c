/**
 * Copyright: 2013 Red Hat, Inc.
 * Author: Nathaniel McCallum <npmccallum@redhat.com>
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iso8601.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define ARRAY_LENGTH(arr) (sizeof((arr)) / sizeof(*(arr)))

struct {
    iso8601_time time;
    const char *str;
} tests[] = {
    {{1970, 6, 1, 10, 10, 10, 123456}, "1970-01-01T00:00:00Z"},
};

int main(int argc, const char **argv)
{
    char buf[1000];
    int ret;

    ret = iso8601_unparse(&tests[0].time, ISO8601_FLAG_BASIC, 6,
                          ISO8601_FORMAT_NORMAL, ISO8601_TRUNCATE_NONE,
                          sizeof(buf), buf);
    if (ret != 0) {
        fprintf(stderr, "%s\n", strerror(ret));
        assert(ret == 0);
    }
    fprintf(stderr, "%s\n", buf);

    return 0;
}
