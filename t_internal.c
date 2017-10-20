/* vim: set tabstop=8 shiftwidth=4 softtabstop=4 expandtab smarttab colorcolumn=80: */
/**
 * Copyright: 2013 Red Hat, Inc.
 * Author: Nathaniel McCallum <npmccallum@redhat.com>
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

#include "internal.h"
#include <stdio.h>
#include <assert.h>

#define ARRAY_LENGTH(arr) (sizeof((arr)) / sizeof(*(arr)))

struct {
    int32_t year;
    struct {
        uint8_t month;
        uint8_t day;
        bool fail;
    } date;
    struct {
        uint16_t ordinal;
        bool fail;
    } ordinal;
} ordinals[] = {
    {2014, { 1,  1},       {1}},
    {2014, { 3,  1},       {60}},
    {2014, {12, 31},       {365}},
    {2014, {12, 32, true}, {366, true}},
    {2014, { 1,  0, true}, {0, true}},
    {2014, { 2, 29, true}, {0, true}},
    {2014, {12, 32, true}, {0, true}},
    {2014, { 0,  1, true}, {0, true}},
    {2014, {13,  1, true}, {0, true}},

    {2012, { 1,  1},       {1}},
    {2012, { 2, 29},       {60}},
    {2012, { 3,  1},       {61}},
    {2012, {12, 30},       {365}},
    {2012, {12, 31},       {366}},
    {2012, {12, 32, true}, {367, true}},
};

struct {
    struct {
        int32_t year;
        uint8_t month;
        uint8_t day;
        bool fail;
    } date;
    struct {
        int32_t year;
        uint8_t week;
        uint8_t day;
        bool fail;
    } week;
} weekdates[] = {
    {{2007,  1,  1},       {2007,  1, 1}},
    {{2007, 12, 30},       {2007, 52, 7}},
    {{2007, 12, 31},       {2008,  1, 1}},
    {{2008,  1,  1},       {2008,  1, 2}},
    {{2009, 12, 31},       {2009, 53, 4}},
    {{2010,  1,  1},       {2009, 53, 5}},
    {{2010,  1,  4},       {2010,  1, 1}},
    {{2011,  1,  1},       {2010, 52, 6}},
    {{1984, 12, 31},       {1985,  1, 1}},
    {{2000, 13,  1, true}, {2000, 53, 1, true}},
    {{2000,  0,  1, true}, {2000,  0, 1, true}},
    {{2000, 12,  0, true}, {2000, 52, 0, true}},
    {{2000, 12, 32, true}, {2000, 52, 8, true}},
};

int main(int argc, const char **argv)
{
    uint16_t ordinal;
    int32_t year;
    uint8_t month;
    uint8_t week;
    uint8_t day;

    /* Test length_*(). */
    assert(length_year_days(2011) == 365);
    assert(length_year_days(2012) == 366);
    assert(length_month_days(2011, 2) == 28);
    assert(length_month_days(2012, 2) == 29);
    assert(length_month_days(2012, 12) == 31);
    assert(length_month_days(2012, 0) == 0);
    assert(length_month_days(2012, 13) == 0);
    assert(length_year_weeks(2008) == 52);
    assert(length_year_weeks(2009) == 53);

    /* Test ordinal_*_date(). */
    for (size_t i = 0; i < ARRAY_LENGTH(ordinals); i++) {
        /* Test ordinal_to_date(). */
        assert(ordinal_to_date(ordinals[i].year, ordinals[i].ordinal.ordinal,
                               &month, &day)
               != ordinals[i].ordinal.fail);
        if (!ordinals[i].ordinal.fail) {
            assert(ordinals[i].date.month == month);
            assert(ordinals[i].date.day == day);
        }

        /* Test ordinal_from_date(). */
        assert(ordinal_from_date(ordinals[i].year, ordinals[i].date.month,
                                 ordinals[i].date.day, &ordinal)
               != ordinals[i].date.fail);
        if (!ordinals[i].date.fail)
            assert(ordinals[i].ordinal.ordinal == ordinal);
    }

    /* Test weekdate_*_date(). */
    for (size_t i = 0; i < ARRAY_LENGTH(weekdates); i++) {
        /* Test weekdate_to_date(). */
        assert(weekdate_to_date(weekdates[i].week.year,
                                weekdates[i].week.week,
                                weekdates[i].week.day,
                                &year, &month, &day)
               != weekdates[i].week.fail);
        if (!weekdates[i].week.fail) {
            assert(weekdates[i].date.year == year);
            assert(weekdates[i].date.month == month);
            assert(weekdates[i].date.day == day);
        }

        /* Test weekdate_from_date(). */
        assert(weekdate_from_date(weekdates[i].date.year,
                                  weekdates[i].date.month,
                                  weekdates[i].date.day,
                                  &year, &week, &day)
               != weekdates[i].date.fail);
        if (!weekdates[i].date.fail) {
            assert(weekdates[i].week.year == year);
            assert(weekdates[i].week.week == week);
            assert(weekdates[i].week.day == day);
        }
    }
}
