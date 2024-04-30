#pragma once

#include <optional>
#include <functional>
#include <utility>
#include <type_traits>

// This is a C++23 feature, but we can implement it ourselves for the lab pcs
namespace opt
{
    using std::nullopt;
    template <typename T>
    class optional : public std::optional<T>
    {
        using std::optional<T>::optional;

#if __cpp_lib_optional < 202110L
    public:
        // Used with a function that returns an optional
        template <class F>
        constexpr auto
        and_then(F &&f) &
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, T &>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>, "Return type of and_then must be optional");
            return std::optional<T>::has_value() ? std::invoke(std::forward<F>(f), **this) : U{};
        }
        // Used with a function that returns an optional
        template <class F>
        constexpr auto and_then(F &&f) const &
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, const T &>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>, "Return type of and_then must be optional");
            return std::optional<T>::has_value() ? std::invoke(std::forward<F>(f), **this) : U{};
        }
        // Used with a function that returns an optional
        template <class F>
        constexpr auto and_then(F &&f) &&
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, T>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>, "Return type of and_then must be optional");
            return std::optional<T>::has_value() ? std::invoke(std::forward<F>(f), std::move(**this)) : U{};
        }
        // Used with a function that returns an optional
        template <class F>
        constexpr auto and_then(F &&f) const &&
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, const T>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>, "Return type of and_then must be optional");
            return std::optional<T>::has_value() ? std::invoke(std::forward<F>(f), std::move(**this)) : U{};
        }

        // Used with a function that doesnt return an optional
        template <class F>
        constexpr auto transform(F &&f) &
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, T &>>;
            return std::optional<T>::has_value() ? optional<U>(std::invoke(std::forward<F>(f), **this)) : optional<U>{};
        }
        // Used with a function that doesnt return an optional
        template <class F>
        constexpr auto transform(F &&f) const &
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, const T &>>;
            return std::optional<T>::has_value() ? optional<U>(std::invoke(std::forward<F>(f), **this)) : optional<U>{};
        }
        // Used with a function that doesnt return an optional
        template <class F>
        constexpr auto transform(F &&f) &&
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
            return std::optional<T>::has_value() ? optional<U>(std::invoke(std::forward<F>(f), std::move(**this))) : optional<U>{};
        }
        // Used with a function that doesnt return an optional
        template <class F>
        constexpr auto transform(F &&f) const &&
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, const T>>;
            return std::optional<T>::has_value() ? optional<U>(std::invoke(std::forward<F>(f), std::move(**this))) : optional<U>{};
        }

        // Used with a function that returns an optional
        template <class F>
        constexpr auto or_else(F &&f) const &
        {
            using U = std::invoke_result_t<F>;
            static_assert(std::is_same_v<std::remove_cvref_t<U>, optional<T>>);
            return std::optional<T>::has_value() ? *this : std::forward<F>(f)();
        }
        // Used with a function that returns an optional
        template <class F>
        constexpr auto or_else(F &&f) &&
        {
            using U = std::invoke_result_t<F>;
            static_assert(std::is_same_v<std::remove_cvref_t<U>, optional<T>>);
            return std::optional<T>::has_value() ? std::move(*this) : std::forward<F>(f)();
        }
#endif
    };

} // namespace opt

namespace std
{
    // This adds the is_optional_v trait to the std namespace, which is used in the optional implementation
    // We need to add this so our class passes the static_assert in the optional implementation
    template <typename T>
    inline constexpr bool __is_optional_v<opt::optional<T>> = true;
} // namespace std