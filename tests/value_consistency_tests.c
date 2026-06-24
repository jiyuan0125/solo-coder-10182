/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"
#include "../cJSON_Utils.h"

#ifndef NAN
#define NAN (0.0/0.0)
#endif

static int doubles_equal_bit_exact(double a, double b)
{
    return memcmp(&a, &b, sizeof(double)) == 0;
}

static void assert_value_fields_consistent(const cJSON *item)
{
    if (item->type & cJSON_True)
    {
        TEST_ASSERT_EQUAL_INT(1, item->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, item->valuedouble);
    }
    else if (item->type & cJSON_False)
    {
        TEST_ASSERT_EQUAL_INT(0, item->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(0.0, item->valuedouble);
    }
    else if (item->type & cJSON_Number)
    {
        double d = item->valuedouble;
        int expected_int;
        if (d >= INT_MAX)
        {
            expected_int = INT_MAX;
        }
        else if (d <= (double)INT_MIN)
        {
            expected_int = INT_MIN;
        }
        else
        {
            expected_int = (int)d;
        }
        TEST_ASSERT_EQUAL_INT(expected_int, item->valueint);
    }
}

static void create_true_should_have_consistent_fields(void)
{
    cJSON *item = cJSON_CreateTrue();
    TEST_ASSERT_NOT_NULL(item);
    assert_has_type(item, cJSON_True);
    assert_value_fields_consistent(item);
    cJSON_Delete(item);
}

static void create_false_should_have_consistent_fields(void)
{
    cJSON *item = cJSON_CreateFalse();
    TEST_ASSERT_NOT_NULL(item);
    assert_has_type(item, cJSON_False);
    assert_value_fields_consistent(item);
    cJSON_Delete(item);
}

static void create_bool_should_have_consistent_fields(void)
{
    cJSON *true_item = cJSON_CreateBool(true);
    cJSON *false_item = cJSON_CreateBool(false);

    TEST_ASSERT_NOT_NULL(true_item);
    TEST_ASSERT_NOT_NULL(false_item);

    assert_has_type(true_item, cJSON_True);
    assert_has_type(false_item, cJSON_False);

    assert_value_fields_consistent(true_item);
    assert_value_fields_consistent(false_item);

    cJSON_Delete(true_item);
    cJSON_Delete(false_item);
}

static void create_number_should_have_consistent_fields(void)
{
    double test_values[] = {
        0.0, 1.0, -1.0, 3.14159, 1e100, -1e100,
        (double)INT_MAX, (double)INT_MIN, (double)INT_MAX + 1.0,
        (double)INT_MIN - 1.0, -0.0, 123456789.123456789
    };
    size_t i;

    for (i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++)
    {
        cJSON *item = cJSON_CreateNumber(test_values[i]);
        TEST_ASSERT_NOT_NULL(item);
        assert_has_type(item, cJSON_Number);
        assert_value_fields_consistent(item);
        cJSON_Delete(item);
    }
}

static void parse_true_should_have_consistent_fields(void)
{
    cJSON *item = cJSON_Parse("true");
    TEST_ASSERT_NOT_NULL(item);
    assert_has_type(item, cJSON_True);
    assert_value_fields_consistent(item);
    cJSON_Delete(item);
}

static void parse_false_should_have_consistent_fields(void)
{
    cJSON *item = cJSON_Parse("false");
    TEST_ASSERT_NOT_NULL(item);
    assert_has_type(item, cJSON_False);
    assert_value_fields_consistent(item);
    cJSON_Delete(item);
}

static void parse_number_should_have_consistent_fields(void)
{
    const char *test_numbers[] = {
        "0", "1", "-1", "3.14159", "1e100", "-1e100",
        "2147483647", "-2147483648", "2147483648", "-2147483649",
        "1.23456789e10", "-0.0"
    };
    size_t i;

    for (i = 0; i < sizeof(test_numbers) / sizeof(test_numbers[0]); i++)
    {
        cJSON *item = cJSON_Parse(test_numbers[i]);
        TEST_ASSERT_NOT_NULL_MESSAGE(item, test_numbers[i]);
        assert_has_type(item, cJSON_Number);
        assert_value_fields_consistent(item);
        cJSON_Delete(item);
    }
}

static void set_number_helper_should_maintain_consistency(void)
{
    cJSON *item = cJSON_CreateNumber(0.0);
    double test_values[] = {
        42.0, -42.0, 1e300, -1e300,
        (double)INT_MAX, (double)INT_MIN,
        (double)INT_MAX + 100.0, (double)INT_MIN - 100.0
    };
    size_t i;

    for (i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++)
    {
        cJSON_SetNumberValue(item, test_values[i]);
        assert_value_fields_consistent(item);
        TEST_ASSERT_EQUAL_DOUBLE(test_values[i], item->valuedouble);
    }

    cJSON_Delete(item);
}

static void duplicate_bool_should_not_have_stale_valuedouble(void)
{
    cJSON *true_item = cJSON_CreateTrue();
    cJSON *false_item = cJSON_CreateFalse();
    cJSON *dup_true, *dup_false;

    TEST_ASSERT_NOT_NULL(true_item);
    TEST_ASSERT_NOT_NULL(false_item);

    dup_true = cJSON_Duplicate(true_item, 0);
    dup_false = cJSON_Duplicate(false_item, 0);

    TEST_ASSERT_NOT_NULL(dup_true);
    TEST_ASSERT_NOT_NULL(dup_false);

    assert_has_type(dup_true, cJSON_True);
    assert_has_type(dup_false, cJSON_False);

    assert_value_fields_consistent(dup_true);
    assert_value_fields_consistent(dup_false);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, dup_true->valuedouble);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, dup_false->valuedouble);

    cJSON_Delete(true_item);
    cJSON_Delete(false_item);
    cJSON_Delete(dup_true);
    cJSON_Delete(dup_false);
}

static void duplicate_number_should_maintain_consistency(void)
{
    double test_values[] = {3.14159, 12345.6789, -0.0, 1e300};
    size_t i;

    for (i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++)
    {
        cJSON *original = cJSON_CreateNumber(test_values[i]);
        cJSON *copy = cJSON_Duplicate(original, 0);

        TEST_ASSERT_NOT_NULL(original);
        TEST_ASSERT_NOT_NULL(copy);

        assert_value_fields_consistent(copy);
        TEST_ASSERT_TRUE_MESSAGE(doubles_equal_bit_exact(original->valuedouble, copy->valuedouble),
            "Duplicated valuedouble should be bit-exact identical");

        cJSON_Delete(original);
        cJSON_Delete(copy);
    }
}

static void duplicate_complex_structure_should_maintain_consistency(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *copy, *num_item, *bool_item, *arr_item;

    cJSON_AddNumberToObject(root, "number", 42.5);
    cJSON_AddTrueToObject(root, "true_val");
    cJSON_AddFalseToObject(root, "false_val");

    arr_item = cJSON_CreateArray();
    cJSON_AddItemToArray(arr_item, cJSON_CreateNumber(1.5));
    cJSON_AddItemToArray(arr_item, cJSON_CreateTrue());
    cJSON_AddItemToArray(arr_item, cJSON_CreateFalse());
    cJSON_AddItemToObject(root, "array", arr_item);

    copy = cJSON_Duplicate(root, 1);
    TEST_ASSERT_NOT_NULL(copy);

    num_item = cJSON_GetObjectItem(copy, "number");
    bool_item = cJSON_GetObjectItem(copy, "true_val");
    assert_value_fields_consistent(num_item);
    assert_value_fields_consistent(bool_item);

    bool_item = cJSON_GetObjectItem(copy, "false_val");
    assert_value_fields_consistent(bool_item);

    arr_item = cJSON_GetObjectItem(copy, "array");
    assert_value_fields_consistent(cJSON_GetArrayItem(arr_item, 0));
    assert_value_fields_consistent(cJSON_GetArrayItem(arr_item, 1));
    assert_value_fields_consistent(cJSON_GetArrayItem(arr_item, 2));

    cJSON_Delete(root);
    cJSON_Delete(copy);
}

static void replace_item_via_pointer_bool_replaces_number(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON *num_item = cJSON_CreateNumber(100.5);
    cJSON *bool_item = cJSON_CreateTrue();

    cJSON_AddItemToArray(array, num_item);

    TEST_ASSERT_TRUE(cJSON_ReplaceItemViaPointer(array, num_item, bool_item));

    assert_value_fields_consistent(cJSON_GetArrayItem(array, 0));
    TEST_ASSERT_TRUE(cJSON_IsTrue(cJSON_GetArrayItem(array, 0)));

    cJSON_Delete(array);
}

static void replace_item_via_pointer_number_replaces_bool(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON *bool_item = cJSON_CreateFalse();
    cJSON *num_item = cJSON_CreateNumber(123.456);

    cJSON_AddItemToArray(array, bool_item);

    TEST_ASSERT_TRUE(cJSON_ReplaceItemViaPointer(array, bool_item, num_item));

    assert_value_fields_consistent(cJSON_GetArrayItem(array, 0));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetArrayItem(array, 0)));
    TEST_ASSERT_EQUAL_DOUBLE(123.456, cJSON_GetArrayItem(array, 0)->valuedouble);

    cJSON_Delete(array);
}

static void replace_item_in_array_bool_replaces_number(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON *num_item = cJSON_CreateNumber(500.0);
    cJSON *bool_item = cJSON_CreateFalse();

    cJSON_AddItemToArray(array, num_item);

    TEST_ASSERT_TRUE(cJSON_ReplaceItemInArray(array, 0, bool_item));

    assert_value_fields_consistent(cJSON_GetArrayItem(array, 0));
    TEST_ASSERT_TRUE(cJSON_IsFalse(cJSON_GetArrayItem(array, 0)));

    cJSON_Delete(array);
}

static void replace_item_in_object_number_replaces_bool(void)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON *num_item = cJSON_CreateNumber(789.123);

    cJSON_AddTrueToObject(obj, "key");

    TEST_ASSERT_TRUE(cJSON_ReplaceItemInObject(obj, "key", num_item));

    assert_value_fields_consistent(cJSON_GetObjectItem(obj, "key"));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(obj, "key")));
    TEST_ASSERT_EQUAL_DOUBLE(789.123, cJSON_GetObjectItem(obj, "key")->valuedouble);

    cJSON_Delete(obj);
}

static void set_bool_value_toggle_maintains_print_consistency(void)
{
    double test_values[] = {1e20, -0.0, (double)INT_MAX};
    size_t i;

    for (i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++)
    {
        cJSON *item = cJSON_CreateNumber(test_values[i]);
        char *printed;

        TEST_ASSERT_NOT_NULL(item);
        TEST_ASSERT_TRUE(cJSON_IsNumber(item));

        /* Toggle to false */
        cJSON_SetBoolValue(item, false);
        printed = cJSON_PrintUnformatted(item);
        TEST_ASSERT_EQUAL_STRING("false", printed);
        cJSON_free(printed);
        assert_value_fields_consistent(item);
        TEST_ASSERT_TRUE(cJSON_IsFalse(item));
        TEST_ASSERT_EQUAL_INT(0, item->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(0.0, item->valuedouble);

        /* Toggle to true */
        cJSON_SetBoolValue(item, true);
        printed = cJSON_PrintUnformatted(item);
        TEST_ASSERT_EQUAL_STRING("true", printed);
        cJSON_free(printed);
        assert_value_fields_consistent(item);
        TEST_ASSERT_TRUE(cJSON_IsTrue(item));
        TEST_ASSERT_EQUAL_INT(1, item->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, item->valuedouble);

        /* Toggle back to false */
        cJSON_SetBoolValue(item, false);
        printed = cJSON_PrintUnformatted(item);
        TEST_ASSERT_EQUAL_STRING("false", printed);
        cJSON_free(printed);
        assert_value_fields_consistent(item);
        TEST_ASSERT_TRUE(cJSON_IsFalse(item));
        TEST_ASSERT_EQUAL_INT(0, item->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(0.0, item->valuedouble);

        cJSON_Delete(item);
    }
}

static void set_bool_value_toggle_maintains_duplicate_consistency(void)
{
    double test_values[] = {1e20, -0.0, (double)INT_MAX};
    size_t i;

    for (i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++)
    {
        cJSON *item = cJSON_CreateNumber(test_values[i]);
        cJSON *dup;

        TEST_ASSERT_NOT_NULL(item);
        TEST_ASSERT_TRUE(cJSON_IsNumber(item));

        /* Toggle to false and duplicate */
        cJSON_SetBoolValue(item, false);
        dup = cJSON_Duplicate(item, 0);
        TEST_ASSERT_NOT_NULL(dup);
        assert_value_fields_consistent(dup);
        TEST_ASSERT_TRUE(cJSON_IsFalse(dup));
        TEST_ASSERT_EQUAL_INT(0, dup->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(0.0, dup->valuedouble);
        cJSON_Delete(dup);

        /* Toggle to true and duplicate */
        cJSON_SetBoolValue(item, true);
        dup = cJSON_Duplicate(item, 0);
        TEST_ASSERT_NOT_NULL(dup);
        assert_value_fields_consistent(dup);
        TEST_ASSERT_TRUE(cJSON_IsTrue(dup));
        TEST_ASSERT_EQUAL_INT(1, dup->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, dup->valuedouble);
        cJSON_Delete(dup);

        /* Toggle back to false and duplicate */
        cJSON_SetBoolValue(item, false);
        dup = cJSON_Duplicate(item, 0);
        TEST_ASSERT_NOT_NULL(dup);
        assert_value_fields_consistent(dup);
        TEST_ASSERT_TRUE(cJSON_IsFalse(dup));
        TEST_ASSERT_EQUAL_INT(0, dup->valueint);
        TEST_ASSERT_EQUAL_DOUBLE(0.0, dup->valuedouble);
        cJSON_Delete(dup);

        cJSON_Delete(item);
    }
}

static void print_parse_roundtrip_preserves_valuedouble_bit_exact(void)
{
    double test_values[] = {
        0.0, -0.0, 1.0, -1.0, 3.141592653589793,
        1e100, -1e100, 1e-300, -1e-300,
        (double)INT_MAX, (double)INT_MIN,
        (double)INT_MAX + 1.0, (double)INT_MIN - 1.0,
        123456789012345.6789, -98765432109876.54321,
        1.7976931348623157e+308, 2.2250738585072014e-308,
        9007199254740992.0, 9007199254740993.0
    };
    size_t i;

    for (i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++)
    {
        cJSON *original = cJSON_CreateNumber(test_values[i]);
        char *printed;
        cJSON *parsed;

        TEST_ASSERT_NOT_NULL(original);

        printed = cJSON_PrintUnformatted(original);
        TEST_ASSERT_NOT_NULL_MESSAGE(printed, "Failed to print number");

        parsed = cJSON_Parse(printed);
        TEST_ASSERT_NOT_NULL_MESSAGE(parsed, "Failed to parse printed number");

        assert_value_fields_consistent(parsed);

        TEST_ASSERT_TRUE_MESSAGE(doubles_equal_bit_exact(original->valuedouble, parsed->valuedouble),
            "Round-trip valuedouble should be bit-exact identical");

        cJSON_Delete(original);
        cJSON_Delete(parsed);
        cJSON_free(printed);
    }
}

static void print_parse_roundtrip_bool_preserves_consistency(void)
{
    cJSON *true_orig = cJSON_CreateTrue();
    cJSON *false_orig = cJSON_CreateFalse();
    cJSON *true_parsed, *false_parsed;
    char *printed;

    printed = cJSON_PrintUnformatted(true_orig);
    true_parsed = cJSON_Parse(printed);
    assert_value_fields_consistent(true_parsed);
    cJSON_free(printed);

    printed = cJSON_PrintUnformatted(false_orig);
    false_parsed = cJSON_Parse(printed);
    assert_value_fields_consistent(false_parsed);
    cJSON_free(printed);

    cJSON_Delete(true_orig);
    cJSON_Delete(false_orig);
    cJSON_Delete(true_parsed);
    cJSON_Delete(false_parsed);
}

static void sort_list_maintains_value_consistency(void)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON *sorted;

    cJSON_AddNumberToObject(obj, "num", 123.456);
    cJSON_AddTrueToObject(obj, "bool_true");
    cJSON_AddFalseToObject(obj, "bool_false");

    cJSONUtils_SortObject(obj);

    assert_value_fields_consistent(cJSON_GetObjectItem(obj, "num"));
    assert_value_fields_consistent(cJSON_GetObjectItem(obj, "bool_true"));
    assert_value_fields_consistent(cJSON_GetObjectItem(obj, "bool_false"));

    TEST_ASSERT_EQUAL_DOUBLE(123.456, cJSON_GetObjectItem(obj, "num")->valuedouble);
    TEST_ASSERT_TRUE(cJSON_IsTrue(cJSON_GetObjectItem(obj, "bool_true")));
    TEST_ASSERT_TRUE(cJSON_IsFalse(cJSON_GetObjectItem(obj, "bool_false")));

    sorted = cJSON_Duplicate(obj, 1);
    TEST_ASSERT_NOT_NULL(sorted);
    assert_value_fields_consistent(cJSON_GetObjectItem(sorted, "num"));
    assert_value_fields_consistent(cJSON_GetObjectItem(sorted, "bool_true"));
    assert_value_fields_consistent(cJSON_GetObjectItem(sorted, "bool_false"));

    cJSON_Delete(obj);
    cJSON_Delete(sorted);
}

static void apply_patches_maintains_value_consistency(void)
{
    cJSON *target = cJSON_Parse("{\"value\": 100.5}");
    cJSON *patches = cJSON_Parse("[{\"op\":\"replace\",\"path\":\"/value\",\"value\":true}]");
    int result;
    cJSON *value_item;

    TEST_ASSERT_NOT_NULL(target);
    TEST_ASSERT_NOT_NULL(patches);

    result = cJSONUtils_ApplyPatches(target, patches);
    TEST_ASSERT_EQUAL_INT(0, result);

    value_item = cJSON_GetObjectItem(target, "value");
    assert_value_fields_consistent(value_item);
    TEST_ASSERT_TRUE(cJSON_IsTrue(value_item));

    cJSON_Delete(patches);

    patches = cJSON_Parse("[{\"op\":\"replace\",\"path\":\"/value\",\"value\":42.5}]");
    result = cJSONUtils_ApplyPatches(target, patches);
    TEST_ASSERT_EQUAL_INT(0, result);

    value_item = cJSON_GetObjectItem(target, "value");
    assert_value_fields_consistent(value_item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(value_item));
    TEST_ASSERT_EQUAL_DOUBLE(42.5, value_item->valuedouble);

    cJSON_Delete(target);
    cJSON_Delete(patches);
}

static void generate_patches_with_value_comparison(void)
{
    cJSON *from = cJSON_Parse("{\"num\": 100, \"flag\": true}");
    cJSON *to = cJSON_Parse("{\"num\": 200, \"flag\": false}");
    cJSON *patches;
    int patch_count;

    TEST_ASSERT_NOT_NULL(from);
    TEST_ASSERT_NOT_NULL(to);

    patches = cJSONUtils_GeneratePatches(from, to);
    TEST_ASSERT_NOT_NULL(patches);

    patch_count = cJSON_GetArraySize(patches);
    TEST_ASSERT_EQUAL_INT(2, patch_count);

    cJSON_Delete(from);
    cJSON_Delete(to);
    cJSON_Delete(patches);
}

static void merge_patch_maintains_value_consistency(void)
{
    cJSON *target = cJSON_Parse("{\"num\": 999.9, \"flag\": false}");
    cJSON *patch = cJSON_Parse("{\"num\": 111.1, \"flag\": true}");
    cJSON *result;
    cJSON *num_item, *flag_item;

    TEST_ASSERT_NOT_NULL(target);
    TEST_ASSERT_NOT_NULL(patch);

    result = cJSONUtils_MergePatch(target, patch);
    TEST_ASSERT_NOT_NULL(result);

    num_item = cJSON_GetObjectItem(result, "num");
    flag_item = cJSON_GetObjectItem(result, "flag");

    assert_value_fields_consistent(num_item);
    assert_value_fields_consistent(flag_item);

    TEST_ASSERT_EQUAL_DOUBLE(111.1, num_item->valuedouble);
    TEST_ASSERT_TRUE(cJSON_IsTrue(flag_item));

    cJSON_Delete(result);
    cJSON_Delete(patch);
}

static void merge_patch_replace_number_with_bool(void)
{
    cJSON *target = cJSON_Parse("{\"value\": 123.456}");
    cJSON *patch = cJSON_Parse("{\"value\": true}");
    cJSON *result = cJSONUtils_MergePatch(target, patch);
    cJSON *value_item;

    TEST_ASSERT_NOT_NULL(result);

    value_item = cJSON_GetObjectItem(result, "value");
    assert_value_fields_consistent(value_item);
    TEST_ASSERT_TRUE(cJSON_IsTrue(value_item));

    cJSON_Delete(result);
    cJSON_Delete(patch);
}

static void merge_patch_replace_bool_with_number(void)
{
    cJSON *target = cJSON_Parse("{\"value\": false}");
    cJSON *patch = cJSON_Parse("{\"value\": 789.012}");
    cJSON *result = cJSONUtils_MergePatch(target, patch);
    cJSON *value_item;

    TEST_ASSERT_NOT_NULL(result);

    value_item = cJSON_GetObjectItem(result, "value");
    assert_value_fields_consistent(value_item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(value_item));
    TEST_ASSERT_EQUAL_DOUBLE(789.012, value_item->valuedouble);

    cJSON_Delete(result);
    cJSON_Delete(patch);
}

static void apply_patches_root_replace_number_to_bool(void)
{
    cJSON *target = cJSON_CreateNumber(42.5);
    cJSON *patches = cJSON_Parse("[{\"op\":\"replace\",\"path\":\"\",\"value\":true}]");
    int result;

    TEST_ASSERT_NOT_NULL(target);
    TEST_ASSERT_NOT_NULL(patches);

    result = cJSONUtils_ApplyPatches(target, patches);
    TEST_ASSERT_EQUAL_INT(0, result);
    assert_value_fields_consistent(target);
    TEST_ASSERT_TRUE(cJSON_IsTrue(target));

    cJSON_Delete(target);
    cJSON_Delete(patches);
}

static void apply_patches_root_replace_bool_to_number(void)
{
    cJSON *target = cJSON_CreateFalse();
    cJSON *patches = cJSON_Parse("[{\"op\":\"replace\",\"path\":\"\",\"value\":123.456}]");
    int result;

    TEST_ASSERT_NOT_NULL(target);
    TEST_ASSERT_NOT_NULL(patches);

    result = cJSONUtils_ApplyPatches(target, patches);
    TEST_ASSERT_EQUAL_INT(0, result);
    assert_value_fields_consistent(target);
    TEST_ASSERT_TRUE(cJSON_IsNumber(target));
    TEST_ASSERT_EQUAL_DOUBLE(123.456, target->valuedouble);

    cJSON_Delete(target);
    cJSON_Delete(patches);
}

#if 0
static void create_reference_maintains_value_consistency(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON *num = cJSON_CreateNumber(123.456);
    cJSON *ref;

    cJSON_AddItemToArray(array, num);

    ref = cJSON_CreateArrayReference(array);
    TEST_ASSERT_NOT_NULL(ref);

    cJSON_AddItemToArray(ref, cJSON_CreateTrue());
    cJSON_AddItemToArray(ref, cJSON_CreateNumber(789.0));

    assert_value_fields_consistent(cJSON_GetArrayItem(ref, 1));
    assert_value_fields_consistent(cJSON_GetArrayItem(ref, 2));

    cJSON_Delete(ref);
    cJSON_Delete(array);
}
#endif



static void int_max_min_saturation_maintains_consistency(void)
{
    cJSON *item;

    item = cJSON_CreateNumber((double)INT_MAX + 1000.0);
    assert_value_fields_consistent(item);
    TEST_ASSERT_EQUAL_INT(INT_MAX, item->valueint);
    cJSON_Delete(item);

    item = cJSON_CreateNumber((double)INT_MIN - 1000.0);
    assert_value_fields_consistent(item);
    TEST_ASSERT_EQUAL_INT(INT_MIN, item->valueint);
    cJSON_Delete(item);

    item = cJSON_CreateNumber(1e300);
    assert_value_fields_consistent(item);
    TEST_ASSERT_EQUAL_INT(INT_MAX, item->valueint);
    cJSON_Delete(item);

    item = cJSON_CreateNumber(-1e300);
    assert_value_fields_consistent(item);
    TEST_ASSERT_EQUAL_INT(INT_MIN, item->valueint);
    cJSON_Delete(item);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(create_true_should_have_consistent_fields);
    RUN_TEST(create_false_should_have_consistent_fields);
    RUN_TEST(create_bool_should_have_consistent_fields);
    RUN_TEST(create_number_should_have_consistent_fields);

    RUN_TEST(parse_true_should_have_consistent_fields);
    RUN_TEST(parse_false_should_have_consistent_fields);
    RUN_TEST(parse_number_should_have_consistent_fields);

    RUN_TEST(set_number_helper_should_maintain_consistency);

    RUN_TEST(duplicate_bool_should_not_have_stale_valuedouble);
    RUN_TEST(duplicate_number_should_maintain_consistency);
    RUN_TEST(duplicate_complex_structure_should_maintain_consistency);

    RUN_TEST(replace_item_via_pointer_bool_replaces_number);
    RUN_TEST(replace_item_via_pointer_number_replaces_bool);
    RUN_TEST(replace_item_in_array_bool_replaces_number);
    RUN_TEST(replace_item_in_object_number_replaces_bool);

    RUN_TEST(set_bool_value_toggle_maintains_print_consistency);
    RUN_TEST(set_bool_value_toggle_maintains_duplicate_consistency);

    RUN_TEST(print_parse_roundtrip_preserves_valuedouble_bit_exact);
    RUN_TEST(print_parse_roundtrip_bool_preserves_consistency);

    RUN_TEST(sort_list_maintains_value_consistency);

    RUN_TEST(apply_patches_maintains_value_consistency);
    RUN_TEST(generate_patches_with_value_comparison);
    RUN_TEST(merge_patch_maintains_value_consistency);
    RUN_TEST(merge_patch_replace_number_with_bool);
    RUN_TEST(merge_patch_replace_bool_with_number);
    RUN_TEST(apply_patches_root_replace_number_to_bool);
    RUN_TEST(apply_patches_root_replace_bool_to_number);

    /* RUN_TEST(create_reference_maintains_value_consistency); */

    RUN_TEST(int_max_min_saturation_maintains_consistency);

    return UNITY_END();
}
