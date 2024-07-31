export module cserver.serialization.common_containers;
import cserver.serialization.core;
import fmt;
import std;
import utempl;

namespace cserver {

template <typename T, typename To>
concept OptionalDefault = std::same_as<T, Disabled> || std::convertible_to<T, std::optional<To>>;

export template <typename T>
struct FieldConfig<std::optional<T>> {
  using MainType = T;
  template <OptionalDefault<T> DefaultValueType = Disabled, typename Main = Disabled>
  struct Create {
    using Type = std::optional<T>;
    Main main{};
    DefaultValueType defaultValue{};
  };
};

}  // namespace cserver
