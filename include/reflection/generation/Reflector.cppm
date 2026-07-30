/*! \file  Reflector.cppm
 *! \brief
 *!
 */
module;
#include <meta> // It compiles fine without this, but the IDE doesn't index it otherwise :(

export module refl.generation.Reflector;

import std;

export import refl.descriptors;


namespace refl {
  export template<typename T>
  const TypeInfo &reflect();
}

using namespace refl;

template<std::meta::info info>
consteval std::string_view get_name() {
  if constexpr (std::meta::has_identifier(info)) {
    return std::meta::identifier_of(info);
  } else {
    return std::meta::display_string_of(info);
  }
}

template<std::meta::info info>
std::vector<FieldInfo> enumerate_fields() {
  std::vector<FieldInfo> fields_;
  template for (constexpr auto &f: std::define_static_array(
                        std::meta::nonstatic_data_members_of(info, std::meta::access_context::unchecked()))) {
    static constexpr auto name = get_name<f>();
    fields_.emplace_back(FieldInfo{
            .name   = name,
            .offset = std::meta::offset_of(f),
            .type   = [] -> const TypeInfo   &{ return refl::reflect<typename[:std::meta::type_of(f):]>(); },
    });
  }
  return fields_;
}

template<std::meta::info info>
std::vector<BaseInfo> enumerate_bases() {
  std::vector<BaseInfo> bases;
  template for (constexpr auto &b:
                std::define_static_array(std::meta::bases_of(info, std::meta::access_context::unchecked()))) {
    AccessSpec access_spec;
    if constexpr (std::meta::is_public(b)) {
      access_spec = AccessSpec::PUBLIC;
    } else if constexpr (std::meta::is_protected(b)) {
      access_spec = AccessSpec::PROTECTED;
    } else if constexpr (std::meta::is_private(b)) {
      access_spec = AccessSpec::PRIVATE;
    }
    bases.emplace_back(BaseInfo{
            .type       = [] -> const TypeInfo       &{ return refl::reflect<typename[:std::meta::type_of(b):]>(); },
            .access     = access_spec,
            .is_virtual = std::meta::is_virtual(b),
    });
  }
  return bases;
}

template<typename T>
  requires std::is_class_v<T>
ClassInfo reflect_class() {
  static constexpr auto info = ^^T;
  ClassInfo             class_info{};

  class_info.base_classes = enumerate_bases<info>();
  class_info.fields       = enumerate_fields<info>();

  return class_info;
}

template<typename T>
  requires std::is_pointer_v<T>
PointerInfo reflect_pointer_type() {
  return PointerInfo{
          .pointee = refl::reflect<std::remove_pointer_t<T>>(),
  };
}

template<typename T>
  requires std::is_reference_v<T>
ReferenceInfo reflect_reference_type() {
  return ReferenceInfo{
          .pointee = refl::reflect<std::remove_reference_t<T>>(),
  };
}

template<std::meta::info info>
std::vector<EnumeratorInfo> enumerate_enum_enumerators() {
  static constexpr auto       u_type    = std::meta::underlying_type(info);
  static constexpr auto       enums     = std::define_static_array(std::meta::enumerators_of(info));
  static constexpr bool       is_signed = std::meta::is_signed_type(u_type);
  std::vector<EnumeratorInfo> enumerators;
  template for (constexpr auto &e: enums) {
    constexpr bool is_negative = is_signed and ([:e:] < static_cast<typename[:info:]>(0));
    constexpr auto u_val       = static_cast<typename[:u_type:]>([:e:]);
    constexpr auto val         = [] -> std::uintmax_t {
      if constexpr (is_negative) {
        return static_cast<std::uintmax_t>(-(u_val + 1)) + 1;
      } else {
        return static_cast<std::uintmax_t>(u_val);
      }
    }();
    enumerators.emplace_back(EnumeratorInfo{
            .name = std::meta::identifier_of(e),
            .value =
                    {
                            .magnitude   = val,
                            .is_negative = is_negative,
                    },
    });
  }
  return enumerators;
}

template<typename T>
  requires std::is_enum_v<T>
EnumInfo reflect_enum() {
  auto enums = enumerate_enum_enumerators<^^T>();
  return EnumInfo{
          .is_scoped       = std::is_scoped_enum_v<T>,
          .underlying_type = refl::reflect<std::underlying_type_t<T>>(),
          .enumerators     = std::move(enums),
  };
}

template<std::meta::info info>
std::vector<const TypeInfo *> enumerate_function_parameters() {
  static constexpr bool         is_variadic = std::meta::is_vararg_function(info);
  static constexpr auto         param_infos = std::define_static_array(std::meta::parameters_of(info));
  static constexpr auto         param_span = is_variadic ? param_infos.subspan(0, param_infos.size() - 1) : param_infos;
  std::vector<const TypeInfo *> params;
  template for (constexpr auto &e: param_span) {
    params.emplace_back(&refl::reflect<typename[:std::meta::type_of(e):]>());
  }
  return params;
}

template<typename T>
  requires std::is_function_v<T>
FunctionInfo reflect_function() {
  static constexpr auto info     = ^^T;
  const auto           &ret_type = refl::reflect<typename[:std::meta::return_type_of(info):]>();
  auto                  params   = enumerate_function_parameters<info>();
  return FunctionInfo{
          .return_type     = ret_type,
          .parameter_types = std::move(params),
          .is_noexcept     = std::meta::is_noexcept(info),
          .is_variadic     = std::meta::is_vararg_function(info),
  };
}

namespace refl {
  template<typename T>
  const TypeInfo &reflect() {
    static const TypeInfo info_ = [] -> TypeInfo {
      static constexpr auto info = ^^T;
      TypeInfo              result{};

      result.name = get_name<info>();

      if constexpr (not std::meta::is_function_type(info)) {
        result.size      = std::meta::size_of(info);
        result.alignment = std::meta::alignment_of(info);
        result.operations.init<T>(result.name);
      }

      if constexpr (std::meta::is_fundamental_type(info)) {
        result.kind = TypeInfo::FUNDAMENTAL;
      } else if constexpr (std::meta::is_class_type(info)) {
        result.kind = TypeInfo::CLASS;
        result.descriptor.emplace<ClassInfo>(reflect_class<T>());
      } else if constexpr (std::meta::is_pointer_type(info)) {
        result.kind = TypeInfo::POINTER;
        result.descriptor.emplace<PointerInfo>(reflect_pointer_type<T>());
      } else if constexpr (std::meta::is_reference_type(info)) {
        result.kind = TypeInfo::REFERENCE;
        result.descriptor.emplace<ReferenceInfo>(reflect_reference_type<T>());
      } else if constexpr (std::meta::is_enum_type(info)) {
        result.kind = TypeInfo::ENUM;
        result.descriptor.emplace<EnumInfo>(reflect_enum<T>());
      } else if constexpr (std::meta::is_function_type(info)) {
        result.kind = TypeInfo::FUNCTION;
        result.descriptor.emplace<FunctionInfo>(reflect_function<T>());
      } else {
        static_assert(false, "Unsupported reflected type");
      }

      return result;
    }();

    return info_;
  }
} // namespace refl
