module;
#include <gtest/gtest.h>
export module tests.config;
import cserver.serialization;
import std;
import utempl;

namespace cserver {

struct SomeStruct1 {
  std::optional<std::string_view> field;
};

template <>
constexpr auto kSerialization<SomeStruct1> =
    CreateSerializationConfig<SomeStruct1>().With<"field">().SetFieldParams(Config<>::Create{.defaultValue = "Hello!"});

TEST(Configuration, Basic) {
  EXPECT_EQ(std::string_view{utempl::Get<0>(kSerialization<SomeStruct1>.configs).defaultValue}, std::string_view{"Hello!"});
};

struct DefaultValue {
  static constexpr std::string_view kName = "defaultValue";
  consteval auto operator()() {
    return "";
  };
};

struct SomeStruct2 {
  static_assert(OpenStruct<SomeStruct2>());
  Attributed<std::optional<std::string_view>, DefaultValue> field;
  static_assert(CloseStruct());
};

template <>
constexpr auto kSerialization<SomeStruct2> =
    CreateSerializationConfig<SomeStruct2>().With<"field">().SetFieldParams(Config<>::Create{.defaultValue = "Hello!"});

TEST(Configuration, Attributes) {
  EXPECT_EQ(std::string_view{utempl::Get<0>(kSerialization<SomeStruct2>.configs).defaultValue}, std::string_view{"Hello!"});
};

struct SomeStruct3 {
  static_assert(OpenStruct<SomeStruct3>());
  Attributed<std::optional<std::string_view>, DefaultValue> field;
  static_assert(CloseStruct());
};

template <typename T>
consteval auto GetParameter(T&&, DefaultValue value) {
  return "Hi!";
};

template <>
constexpr auto kSerialization<SomeStruct3> = CreateSerializationConfig<SomeStruct3>().With<"field">().SetFieldParams(
    Config<>::Create{.main = Config<>::Create{.dummy = [] {}},
                     .defaultValue = "Hello!"});  // Dummy for instantiate TransformFieldConfig with new GetParameter call

TEST(Configuration, AttributesWithCustomGetParameter) {
  EXPECT_EQ(std::string_view{utempl::Get<0>(kSerialization<SomeStruct3>.configs).defaultValue}, std::string_view{"Hi!"});
};

struct SomeStruct4 {
  std::optional<std::optional<std::string_view>> field;
};

template <>
constexpr auto kSerialization<SomeStruct4> = CreateSerializationConfig<SomeStruct4>().With<"field">().SetFieldParams(
    Config<>::Create{.main = Config<>::Create{.main = Config<>::Create{.dummy = [] {}}, .defaultValue = "Hi!"}, .defaultValue = "Hello!"});

TEST(Configuration, AttributesWithNestedType) {
  EXPECT_EQ(std::string_view{utempl::Get<0>(kSerialization<SomeStruct4>.configs).defaultValue}, std::string_view{"Hello!"});
  EXPECT_EQ(std::string_view{utempl::Get<0>(kSerialization<SomeStruct4>.configs).main.defaultValue}, std::string_view{"Hi!"});
};

}  // namespace cserver
