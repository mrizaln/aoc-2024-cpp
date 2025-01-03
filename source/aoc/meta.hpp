#pragma once

#include <tuple>
#include <variant>

namespace aoc::meta
{
    template <typename>
    struct ToTupleTraits
    {
        static_assert(false, "ToTuple not specialized for this type");
    };

    template <typename... Ts>
    struct ToTupleTraits<std::variant<Ts...>>
    {
        using Type = std::tuple<Ts...>;
    };

    template <typename T>
    using ToTuple = typename ToTupleTraits<T>::Type;

    template <typename>
    struct ToVariantTraits
    {
        static_assert(false, "ToVariant not specialized for this type");
    };

    template <typename... Ts>
    struct ToVariantTraits<std::tuple<Ts...>>
    {
        using Type = std::variant<Ts...>;
    };

    template <typename T>
    using ToVariant = typename ToVariantTraits<T>::Type;

    template <typename Tuple, typename Fn>
    constexpr void for_each_tuple(Fn&& fn)
    {
        const auto handler = [&]<std::size_t... I>(std::index_sequence<I...>) {
            (fn.template operator()<std::tuple_element_t<I, Tuple>>(), ...);
        };
        handler(std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <typename Tuple, typename Fn>
    constexpr auto for_each_tuple(const Tuple& tuple, Fn&& fn)
    {
        if constexpr (std::same_as<void, std::invoke_result_t<Fn, std::tuple_element_t<0, Tuple>>>) {
            const auto handler = [&]<std::size_t... I>(std::index_sequence<I...>) {
                (fn(std::get<I>(tuple)), ...);
            };
            handler(std::make_index_sequence<std::tuple_size_v<Tuple>>());
        } else {
            const auto handler = [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::tuple{ fn(std::get<I>(tuple))... };
            };
            return handler(std::make_index_sequence<std::tuple_size_v<Tuple>>());
        }
    }
}
