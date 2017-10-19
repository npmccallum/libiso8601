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

int main(int argc, const char **argv)
{
    iso8601_time ta, tb;
    time_t a, b;

    setenv("TZ", "EST+5", 1);

    do {
        assert((a = time(NULL)) != (time_t)-1);
        assert(iso8601_current(false, 0, &ta) == 0);
        assert((b = time(NULL)) != (time_t)-1);
    } while (a != b);

    /*
     * Check comparison.
     *
     * NOTE: iso8601_from_time_t() exercises all of the iso8601_from_*()
     *       functions internally.
     */
    iso8601_from_time_t(b, ta.usecond, false, 0, &tb);
    assert(iso8601_compare(&ta, &tb) == 0);
    iso8601_from_time_t(b - 1, ta.usecond, false, 0, &tb);
    assert(iso8601_compare(&ta, &tb) > 0);
    iso8601_from_time_t(b + 1, ta.usecond, false, 0, &tb);
    assert(iso8601_compare(&ta, &tb) < 0);


    /*
     * Check conversion to time_t.
     *
     * NOTE: iso8601_from_time_t() exercises all of the iso8601_from_*()
     *       functions internally.
     */
    iso8601_to_time_t(&ta, &a);
    assert(a == b);

    /* Check comparison against UTC from local time. */
    iso8601_from_time_t(0, 0, true, 0, &ta);
    iso8601_from_time_t(0, 0, false, 0, &tb);
    assert(iso8601_compare(&ta, &tb) == 0);

    /* Check comparison against UTC from offset time. */
    iso8601_from_time_t(0, 0, false, 0, &ta);
    iso8601_from_time_t(0, 0, false, 120, &tb);
    assert(iso8601_compare(&ta, &tb) == 0);

    /* Check comparison against offset time from local time. */
    iso8601_from_time_t(0, 0, true, 0, &ta);
    iso8601_from_time_t(0, 0, false, 120, &tb);
    assert(iso8601_compare(&ta, &tb) == 0);

    /* Check usecond comparison. */
    iso8601_from_time_t(0, 0, true, 0, &ta);
    iso8601_from_time_t(0, 500000, false, 120, &tb);
    assert(iso8601_compare(&ta, &tb) < 0);
    assert(iso8601_compare(&tb, &ta) > 0);

    /* Check negative conversion. */
    a = -157766400;
    iso8601_from_time_t(a, 0, false, 0, &ta);
    iso8601_to_time_t(&ta, &b);
    assert(a == b);

    /* Check comparison of large dates. */
    assert(iso8601_parse("1970-01-01T00:00:00Z", &ta) == 0);
    assert(iso8601_parse("9999-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) < 0);
    assert(iso8601_compare(&tb, &ta) > 0);
    assert(iso8601_parse("+99999-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) < 0);
    assert(iso8601_compare(&tb, &ta) > 0);
    assert(iso8601_parse("-0100-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) > 0);
    assert(iso8601_compare(&tb, &ta) < 0);
    assert(iso8601_parse("9999-01-01T00:00:01Z", &ta) == 0);
    assert(iso8601_parse("9999-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) > 0);
    assert(iso8601_compare(&tb, &ta) < 0);

    /* Check comparison of large dates in different zones. */
    assert(iso8601_parse("1970-01-01T00:00:00", &ta) == 0);
    assert(iso8601_parse("9999-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) < 0);
    assert(iso8601_compare(&tb, &ta) > 0);
    assert(iso8601_parse("+99999-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) < 0);
    assert(iso8601_compare(&tb, &ta) > 0);
    assert(iso8601_parse("-0100-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) > 0);
    assert(iso8601_compare(&tb, &ta) < 0);
#if SIZEOF_TIME_T > 4
    /* Fails when time_t is too small to represent the dates. */
    assert(iso8601_parse("9999-01-01T00:00:01", &ta) == 0);
    assert(iso8601_parse("9999-01-01T00:00:00Z", &tb) == 0);
    assert(iso8601_compare(&ta, &tb) > 0);
    assert(iso8601_compare(&tb, &ta) < 0);
#endif

    return 0;
}
