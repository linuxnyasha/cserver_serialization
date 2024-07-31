module;
#include <boost/pfr.hpp>
export module cserver.serialization.core;
import std;
import utempl;

namespace cserver {

export template <typename T>
concept Serializator = true;

export struct Disabled {};

consteval auto Find(std::ranges::range auto&& range, auto value) -> std::size_t {
  return std::ranges::find(range, value) - range.begin();
};

template <typename F, typename Arg>
concept Function = std::same_as<F, Disabled> || [] {
  if constexpr(std::invocable<F, Arg>) {
    return !std::same_as<std::invoke_result_t<F, Arg>, void>;
  };
  return false;
}();

export template <typename T>
constexpr Disabled kSerialization{};

export template <typename T>
constexpr auto kDeserialization = kSerialization<T>;

export template <typename T>
concept AutoSerializable = !std::same_as<decltype(kSerialization<std::decay_t<T>>), const Disabled>;

export template <typename T>
concept AutoDeserializable = !std::same_as<decltype(kSerialization<std::decay_t<T>>), const Disabled>;

export template <typename T, typename Serializator>
concept AdlSerializable = requires(T a, Serializator serializator) { Serialize(a, serializator); };

export template <typename T, typename Serializator>
concept AdlDeserializable = requires(T t, Serializator s) { Deserialize(t, s); };

export template <typename Serializator, typename T>
struct FmtStruct {
  T* ptr;
};

export template <utempl::ConstexprString Name>
constexpr auto GetParam(auto& config) {};

export template <utempl::ConstexprString Name>
constexpr auto ContainsParam(auto& config) -> bool {};

export template <typename FieldType>
struct DefaultFieldConfig {
  using Type = FieldType;
  template <Function<FieldType> F = Disabled, typename Dummy = decltype([] {}), typename... Ts>
  struct Create {
    using Type = FieldType;
    bool useFmtCompile{};
    F transform{};
    utempl::Tuple<Ts...> params{};
    Dummy dummy{};
  };

  template <typename Serializator, typename T>
  static constexpr auto Transform(T& obj)
    requires AdlSerializable<T, Serializator> || AutoSerializable<T>
  {
    return FmtStruct<Serializator, T>(&obj);
  };
};

export template <typename FieldType>
struct FieldConfig : DefaultFieldConfig<FieldType> {};

template <typename T>
concept NonVoid = !std::same_as<T, void>;

template <typename T>
concept AttributeParameter = requires(T t) {
  { T::kName } -> std::same_as<const std::string_view&>;
  { GetParameter(t, Disabled{}) } -> NonVoid;
};

export template <typename T, AttributeParameter... Ts>
using Attributed = utempl::FieldAttribute<T, utempl::TypeList<Ts...>>;

export template <typename T, typename..., auto f = [] {}, auto R = utempl::OpenStruct<T, decltype(f)>()>
consteval auto OpenStruct() {
  return R;
};

export template <typename..., auto f = [] {}, auto R = utempl::CloseStruct<decltype(f)>()>
consteval auto CloseStruct() {
  return R;
};

struct FieldTag {};

struct FieldTagConfig {};

template <std::size_t>
struct FieldKey {};

template <std::size_t>
struct FieldKeyConfig {};

template <typename Old, typename Parameter>
constexpr auto GetParameter(Old&& old, Parameter&& parameter) {
  if constexpr(std::same_as<std::decay_t<Old>, Disabled>) {
    return std::forward<Parameter>(parameter)();
  } else {
    return std::forward<Old>(old);
  };
};

template <typename T>
concept Aggregate = std::is_aggregate_v<T>;

template <typename... Parameters, Aggregate Config>
consteval auto TransformFieldConfig(utempl::TypeList<Parameters...> list, Config&& config) {
  static constexpr auto names = std::array<std::string_view, sizeof...(Parameters)>{Parameters::kName...};
  static constexpr auto configNames = boost::pfr::names_as_array<Config>();
  return [&](auto... is) {
    return typename FieldConfig<typename std::decay_t<Config>::Type>::Create{[&] {
      if constexpr(std::ranges::find(names, configNames[*is]) != names.end()) {
        return GetParameter(boost::pfr::get<*is>(std::forward<Config>(config)),
                            decltype(utempl::Get<std::ranges::find(names, configNames[*is]) - names.begin()>(list)){});
      } else {
        return boost::pfr::get<*is>(std::forward<Config>(config));
      };
    }()...};
  } | utempl::kSeq<boost::pfr::tuple_size_v<Config>>;
};

export template <auto f = [] {},
                 auto I = utempl::loopholes::Counter<FieldTagConfig, decltype(f)>(),
                 auto II = utempl::loopholes::CountValue<FieldTag, decltype(f)>(),
                 typename T = decltype([] {
                   if constexpr(!requires { Magic(utempl::loopholes::Getter<FieldKeyConfig<I - 1>{}>{}); }) {
                     return typename decltype(Magic(utempl::loopholes::Getter<FieldKey<II - 1>{}>{}))::Type{};
                   } else {
                     return typename decltype(Magic(utempl::loopholes::Getter<FieldKeyConfig<I - 1>{}>{}))::Type{};
                   };
                 }()),
                 auto = utempl::loopholes::Injector<FieldKeyConfig<I>{},
                                                    [] {
                                                      if constexpr(requires { typename FieldConfig<T>::MainType{}; }) {
                                                        return utempl::kType<typename FieldConfig<T>::MainType>;
                                                      } else {
                                                        return utempl::kType<T>;
                                                      };
                                                    }()>{}>
using Config = FieldConfig<T>;

template <typename T, std::move_constructible... Ts>
struct SerializationConfig {
  utempl::Tuple<Ts...> configs;

  static constexpr auto kAttributes = [] {
    if constexpr(utempl::HasAttributes<T>) {
      return decltype(utempl::Get<0>(utempl::Get<0>(utempl::GetAttributes<T>()))){};
    } else {
      return utempl::kTypeList<>;
    };
  }();

  /* NOLINT */ constexpr SerializationConfig(utempl::TypeList<T>, Ts&&... configs) : configs(std::move(configs)...) {};

  template <utempl::ConstexprString Name>
  struct Info {
    static constexpr auto kName = Name;
    static constexpr std::size_t kIndex = ::cserver::Find(boost::pfr::names_as_array<T>(), Name);
    using Type = std::decay_t<decltype(boost::pfr::get<kIndex>(std::declval<T&>()))>;
  };

  template <utempl::ConstexprString Name,
            auto f = [] {},
            auto I = utempl::loopholes::Counter<FieldTag, decltype(f)>(),
            auto = utempl::loopholes::Counter<FieldTagConfig, decltype(f)>(),
            auto = utempl::loopholes::Injector<FieldKey<I>{}, Info<Name>{}>{}>
  consteval auto With() -> SerializationConfig<T, Ts...> {
    return std::move(*this);
  };
  template <auto f = [] {}>
  consteval auto SetFieldParams(Aggregate auto value) {
    constexpr auto I = Magic(utempl::loopholes::Getter<FieldKey<utempl::loopholes::CountValue<FieldTag, decltype(f)>() - 1>{}>{}).kIndex;
    return utempl::Unpack(this->configs, [&](auto... vs) {
      return [&](auto... is) {
        return []<typename... TTs>(TTs&&... args) -> SerializationConfig<T, std::decay_t<TTs>...> {
          return {utempl::kType<T>, std::forward<TTs>(args)...};
        }([&]<std::size_t II>(utempl::Wrapper<II> is, auto&& vs) {
                 if constexpr(I == II) {
                   return TransformFieldConfig(kAttributes, std::move(value));
                 } else {
                   return std::move(vs);
                 };
               }(is, vs)...);
      } | utempl::kSeq<sizeof...(Ts)>;
    });
  };
};

export template <typename T>
consteval auto CreateSerializationConfig() {
  static constexpr auto attributes = [] {
    if constexpr(utempl::HasAttributes<T>) {
      return typename std::decay_t<decltype(utempl::Get<0>(utempl::GetAttributes<T>()))>::Type{};
    } else {
      return utempl::kTypeList<>;
    };
  }();
  return [](auto... is) {
    return SerializationConfig{
        utempl::kType<T>,
        TransformFieldConfig(
            attributes, typename FieldConfig<std::decay_t<decltype(boost::pfr::get<*is>(std::declval<T&>()))>>::template Create<>{})...};
  } | utempl::kSeq<boost::pfr::tuple_size_v<T>>;
};

}  // namespace cserver
