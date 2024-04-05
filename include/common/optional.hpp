#pragma once

#include <optional>
#include <functional>
#include <utility>
#include <type_traits>

// This is a C++23 feature, but we can implement it ourselves for the lab pcs
namespace opt
{
    template <typename T>
    class optional : public std::optional<T>
    {
        using std::optional<T>::optional;

#if __cpp_lib_optional < 202110L
    public:
        template <class F>
        constexpr auto
        and_then(F &&f) &
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, T &>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>);
            if (std::optional<T>::has_value())
                return std::invoke(std::forward<F>(f), **this);
            else
                return U{};
        }
        template <class F>
        constexpr auto and_then(F &&f) const &
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, const T &>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>);
            if (std::optional<T>::has_value())
                return std::invoke(std::forward<F>(f), **this);
            else
                return U{};
        }
        template <class F>
        constexpr auto and_then(F &&f) &&
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, T>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>);
            if (std::optional<T>::has_value())
                return std::invoke(std::forward<F>(f), std::move(**this));
            else
                return U{};
        }
        template <class F>
        constexpr auto and_then(F &&f) const &&
        {
            using U = std::remove_cvref_t<std::invoke_result_t<F, const T>>;
            static_assert(std::__is_optional_v<std::remove_cvref_t<U>>);
            if (std::optional<T>::has_value())
                return std::invoke(std::forward<F>(f), std::move(**this));
            else
                return U{};
        }

        template <class F>
        constexpr auto transform(F &&f) &
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, T &>>;
            if (std::optional<T>::has_value())
                return optional<U>(std::invoke(std::forward<F>(f), **this));
            else
                return optional<U>{};
        }
        template <class F>
        constexpr auto transform(F &&f) const &
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, const T &>>;
            if (std::optional<T>::has_value())
                return optional<U>(std::invoke(std::forward<F>(f), **this));
            else
                return optional<U>{};
        }
        template <class F>
        constexpr auto transform(F &&f) &&
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
            if (std::optional<T>::has_value())
                return optional<U>(std::invoke(std::forward<F>(f), std::move(**this)));
            else
                return optional<U>{};
        }
        template <class F>
        constexpr auto transform(F &&f) const &&
        {
            using U = std::remove_cv_t<std::invoke_result_t<F, const T>>;
            if (std::optional<T>::has_value())
                return optional<U>(std::invoke(std::forward<F>(f), std::move(**this)));
            else
                return optional<U>{};
        }

        template <class F>
        constexpr optional or_else(F &&f) const &
        {
            using U = std::invoke_result_t<F>;
            static_assert(std::is_same_v<std::remove_cvref_t<U>, optional<T>>);
            return std::optional<T>::has_value() ? *this : std::forward<F>(f)();
        }
        template <class F>
        constexpr optional or_else(F &&f) &&
        {
            using U = std::invoke_result_t<F>;
            static_assert(std::is_same_v<std::remove_cvref_t<U>, optional<T>>);
            return std::optional<T>::has_value() ? std::move(*this) : std::forward<F>(f)();
        }
#endif
    };

} // namespace opt