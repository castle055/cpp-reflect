/*! \file  optics.cppm
 *! \brief 
 *!
 */

export module reflect.optics;

import std;
import packtl;
export import reflect;

namespace refl { namespace detail {
    template<typename>
    struct member_pointer_type;

    template<typename T, typename FT>
    struct member_pointer_type<FT T::*> {
      using field_type  = FT;
      using object_type = T;
    };


    template<typename T>
    using member_pointer_field_type_t = typename member_pointer_type<T>::field_type;
  }

  export template<typename T>
  concept OpticConcept = requires
  {
    typename T::target_type;
    typename T::focus_type;
  };

  export template<typename TargetT, typename FocusT>
  class Affine {
  public:
    using target_type = TargetT;
    using focus_type  = std::optional<std::reference_wrapper<const FocusT>>;

    std::function<std::optional<std::reference_wrapper<const FocusT>>(const TargetT &)> get;
    std::function<void(TargetT &, const FocusT &)> set;

    explicit Affine(
      std::function<std::optional<std::reference_wrapper<const FocusT>>(const TargetT &)> get,
      std::function<void(TargetT &, const FocusT &)> set
    ) : get(std::move(get)), set(std::move(set)) {
    }

    // and_then(OpticConcept auto other) const {
    // }
  };

  export template<typename TargetT, typename FocusT>
  class Lens {
  public:
    using target_type = TargetT;
    using focus_type  = FocusT;

    std::function<const focus_type&(const target_type &)> get;
    std::function<void(TargetT &, const FocusT &)> set;

    template<typename... T>
    explicit Lens(auto T::*... fields) //
      requires (std::same_as<TargetT, typename packtl::get_first<T
                               ...>::type> and std::same_as<
                  std::remove_const_t<FocusT>, detail::member_pointer_field_type_t<
                    typename packtl::get_last<decltype(fields)
                      ...>::type>> and is_valid_path<TargetT, decltype(fields)...>) {
    }

    template<OpticConcept T>
    Affine<TargetT, typename T::focus_type> and_then(T other) const {
    }
  };

  export template<typename... T>
  Lens(
    auto T::*... fields
  ) -> Lens<typename packtl::get_first<T
              ...>::type, detail::member_pointer_field_type_t<
              typename packtl::get_last<decltype(fields)
                ...>::type>>;


  export template<typename TargetT, typename FocusT>
    requires packtl::is_type<std::variant, TargetT>::value
  class Prism {
  public:
    using target_type = TargetT;
    using focus_type  = std::optional<std::reference_wrapper<const FocusT>>;

    std::function<focus_type(const target_type &)> get;
    std::function<void(TargetT &, const FocusT &)> set;
  };
}
