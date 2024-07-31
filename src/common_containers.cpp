export module SERIALIZATION_LIBRARY_NAME.common_containers;
import SERIALIZATION_LIBRARY_NAME.core;
import fmt;
import std;
import utempl;

namespace SERIALIZATION_LIBRARY_NAME {

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

}  // namespace SERIALIZATION_LIBRARY_NAME
