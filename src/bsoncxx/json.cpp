// Copyright 2015 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <bsoncxx/json.hpp>

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include <bson.h>

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/exception/error_code.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/private/b64_ntop.h>
#include <bsoncxx/stdx/make_unique.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>

#include <bsoncxx/config/private/prelude.hpp>

namespace bsoncxx {
BSONCXX_INLINE_NAMESPACE_BEGIN

namespace {

void bson_free_deleter(std::uint8_t* ptr) {
    bson_free(ptr);
}

}  // namespace

std::string to_json(document::view view) {
    bson_t bson;
    bson_init_static(&bson, view.data(), view.length());

    size_t size;
    auto result = bson_as_json(&bson, &size);
    if (!result) throw exception(error_code::k_failed_converting_bson_to_json);

    const auto deleter = [](char* result) { bson_free(result); };
    const std::unique_ptr<char[], decltype(deleter)> cleanup(result, deleter);

    return {result, size};
}

document::value from_json(stdx::string_view json) {
    bson_error_t error;
    bson_t* result =
        bson_new_from_json(reinterpret_cast<const uint8_t*>(json.data()), json.size(), &error);

    if (!result) throw exception(error_code::k_json_parse_failure, error.message);

    std::uint32_t length;
    std::uint8_t* buf = bson_destroy_with_steal(result, true, &length);

    return document::value{buf, length, bson_free_deleter};
}

BSONCXX_INLINE_NAMESPACE_END
}  // namespace bsoncxx
