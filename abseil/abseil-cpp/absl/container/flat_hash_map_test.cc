// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/container/flat_hash_map.h"

#include <memory>

#include "absl/base/internal/raw_logging.h"
#include "absl/container/internal/hash_generator_testing.h"
#include "absl/container/internal/unordered_map_constructor_test.h"
#include "absl/container/internal/unordered_map_lookup_test.h"
#include "absl/container/internal/unordered_map_members_test.h"
#include "absl/container/internal/unordered_map_modifiers_test.h"
#include "absl/types/any.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {
namespace {
using ::absl::container_internal::hash_internal::Enum;
using ::absl::container_internal::hash_internal::EnumClass;
using ::testing::_;
using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

// Check that absl::flat_hash_map works in a global constructor.
struct BeforeMain {
  BeforeMain() {
    absl::flat_hash_map<int, int> x;
    x.insert({1, 1});
    ABSL_RAW_CHECK(x.find(0) == x.end(), "x should not contain 0");
    auto it = x.find(1);
    ABSL_RAW_CHECK(it != x.end(), "x should contain 1");
    ABSL_RAW_CHECK(it->second, "1 should map to 1");
  }
};
const BeforeMain before_main;

template <class K, class V>
using Map = flat_hash_map<K, V, StatefulTestingHash, StatefulTestingEqual,
                          Alloc<std::pair<const K, V>>>;

static_assert(!std::is_standard_layout<NonStandardLayout>(), "");

using MapTypes =
    ::testing::Types<Map<int, int>, Map<std::string, int>,
                     Map<Enum, std::string>, Map<EnumClass, int>,
                     Map<int, NonStandardLayout>, Map<NonStandardLayout, int>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FlatHashMap, ConstructorTest, MapTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(FlatHashMap, LookupTest, MapTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(FlatHashMap, MembersTest, MapTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(FlatHashMap, ModifiersTest, MapTypes);

using UniquePtrMapTypes = ::testing::Types<Map<int, std::unique_ptr<int>>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FlatHashMap, UniquePtrModifiersTest,
                               UniquePtrMapTypes);

TEST(FlatHashMap, StandardLayout) {
  struct Int {
    explicit Int(size_t value) : value(value) {}
    Int() : value(0) { ADD_FAILURE(); }
    Int(const Int& other) : value(other.value) { ADD_FAILURE(); }
    Int(Int&&) = default;
    bool operator==(const Int& other) const { return value == other.value; }
    size_t value;
  };
  static_assert(std::is_standard_layout<Int>(), "");

  struct Hash {
    size_t operator()(const Int& obj) const { return obj.value; }
  };

  // Verify that neither the key nor the value get default-constructed or
  // copy-constructed.
  {
    flat_hash_map<Int, Int, Hash> m;
    m.try_emplace(Int(1), Int(2));
    m.try_emplace(Int(3), Int(4));
    m.erase(Int(1));
    m.rehash(2 * m.bucket_count());
  }
  {
    flat_hash_map<Int, Int, Hash> m;
    m.try_emplace(Int(1), Int(2));
    m.try_emplace(Int(3), Int(4));
    m.erase(Int(1));
    m.clear();
  }
}

// gcc becomes unhappy if this is inside the method, so pull it out here.
struct balast {};

TEST(FlatHashMap, IteratesMsan) {
  // Because SwissTable randomizes on pointer addresses, we keep old tables
  // around to ensure we don't reuse old memory.
  std::vector<absl::flat_hash_map<int, balast>> garbage;
  for (int i = 0; i < 100; ++i) {
    absl::flat_hash_map<int, balast> t;
    for (int j = 0; j < 100; ++j) {
      t[j];
      for (const auto& p : t) EXPECT_THAT(p, Pair(_, _));
    }
    garbage.push_back(std::move(t));
  }
}

// Demonstration of the "Lazy Key" pattern.  This uses heterogeneous insert to
// avoid creating expensive key elements when the item is already present in the
// map.
struct LazyInt {
  explicit LazyInt(size_t value, int* tracker)
      : value(value), tracker(tracker) {}

  explicit operator size_t() const {
    ++*tracker;
    return value;
  }

  size_t value;
  int* tracker;
};

struct Hash {
  using is_transparent = void;
  int* tracker;
  size_t operator()(size_t obj) const {
    ++*tracker;
    return obj;
  }
  size_t operator()(const LazyInt& obj) const {
    ++*tracker;
    return obj.value;
  }
};

struct Eq {
  using is_transparent = void;
  bool operator()(size_t lhs, size_t rhs) const {
    return lhs == rhs;
  }
  bool operator()(size_t lhs, const LazyInt& rhs) const {
    return lhs == rhs.value;
  }
};

TEST(FlatHashMap, LazyKeyPattern) {
  // hashes are only guaranteed in opt mode, we use assertions to track internal
  // state that can cause extra calls to hash.
  int conversions = 0;
  int hashes = 0;
  flat_hash_map<size_t, size_t, Hash, Eq> m(0, Hash{&hashes});
  m.reserve(3);

  m[LazyInt(1, &conversions)] = 1;
  EXPECT_THAT(m, UnorderedElementsAre(Pair(1, 1)));
  EXPECT_EQ(conversions, 1);
#ifdef NDEBUG
  EXPECT_EQ(hashes, 1);
#endif

  m[LazyInt(1, &conversions)] = 2;
  EXPECT_THAT(m, UnorderedElementsAre(Pair(1, 2)));
  EXPECT_EQ(conversions, 1);
#ifdef NDEBUG
  EXPECT_EQ(hashes, 2);
#endif

  m.try_emplace(LazyInt(2, &conversions), 3);
  EXPECT_THAT(m, UnorderedElementsAre(Pair(1, 2), Pair(2, 3)));
  EXPECT_EQ(conversions, 2);
#ifdef NDEBUG
  EXPECT_EQ(hashes, 3);
#endif

  m.try_emplace(LazyInt(2, &conversions), 4);
  EXPECT_THAT(m, UnorderedElementsAre(Pair(1, 2), Pair(2, 3)));
  EXPECT_EQ(conversions, 2);
#ifdef NDEBUG
  EXPECT_EQ(hashes, 4);
#endif
}

TEST(FlatHashMap, BitfieldArgument) {
  union {
    int n : 1;
  };
  n = 0;
  flat_hash_map<int, int> m;
  m.erase(n);
  m.count(n);
  m.prefetch(n);
  m.find(n);
  m.contains(n);
  m.equal_range(n);
  m.insert_or_assign(n, n);
  m.insert_or_assign(m.end(), n, n);
  m.try_emplace(n);
  m.try_emplace(m.end(), n);
  m.at(n);
  m[n];
}

TEST(FlatHashMap, MergeExtractInsert) {
  // We can't test mutable keys, or non-copyable keys with flat_hash_map.
  // Test that the nodes have the proper API.
  absl::flat_hash_map<int, int> m = {{1, 7}, {2, 9}};
  auto node = m.extract(1);
  EXPECT_TRUE(node);
  EXPECT_EQ(node.key(), 1);
  EXPECT_EQ(node.mapped(), 7);
  EXPECT_THAT(m, UnorderedElementsAre(Pair(2, 9)));

  node.mapped() = 17;
  m.insert(std::move(node));
  EXPECT_THAT(m, UnorderedElementsAre(Pair(1, 17), Pair(2, 9)));
}

bool FirstIsEven(std::pair<const int, int> p) { return p.first % 2 == 0; }

TEST(FlatHashMap, EraseIf) {
  // Erase all elements.
  {
    flat_hash_map<int, int> s = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    erase_if(s, [](std::pair<const int, int>) { return true; });
    EXPECT_THAT(s, IsEmpty());
  }
  // Erase no elements.
  {
    flat_hash_map<int, int> s = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    erase_if(s, [](std::pair<const int, int>) { return false; });
    EXPECT_THAT(s, UnorderedElementsAre(Pair(1, 1), Pair(2, 2), Pair(3, 3),
                                        Pair(4, 4), Pair(5, 5)));
  }
  // Erase specific elements.
  {
    flat_hash_map<int, int> s = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    erase_if(s,
             [](std::pair<const int, int> kvp) { return kvp.first % 2 == 1; });
    EXPECT_THAT(s, UnorderedElementsAre(Pair(2, 2), Pair(4, 4)));
  }
  // Predicate is function reference.
  {
    flat_hash_map<int, int> s = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    erase_if(s, FirstIsEven);
    EXPECT_THAT(s, UnorderedElementsAre(Pair(1, 1), Pair(3, 3), Pair(5, 5)));
  }
  // Predicate is function pointer.
  {
    flat_hash_map<int, int> s = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    erase_if(s, &FirstIsEven);
    EXPECT_THAT(s, UnorderedElementsAre(Pair(1, 1), Pair(3, 3), Pair(5, 5)));
  }
}

}  // namespace
}  // namespace container_internal
ABSL_NAMESPACE_END
}  // namespace absl
