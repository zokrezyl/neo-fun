#pragma once

#include "./declval.hpp"
#include "./fwd.hpp"

namespace neo {

#define INVOKE_BODY(...)                                                                           \
    noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; }                             \
    { return __VA_ARGS__; }                                                                        \
    static_assert(true)

namespace invoke_detail {

template <typename Func>
struct invoker {
    template <typename... Args>
    static constexpr decltype(auto) invoke(Func&& fn, Args&&... args)
        INVOKE_BODY(((Func &&)(fn))((Args &&)(args)...));
};

template <typename Func, typename... Args>
constexpr static bool is_noexcept = requires(Func&& fn, Args&&... args) {
    { invoker<Func>::invoke(NEO_FWD(fn), NEO_FWD(args)...) }
    noexcept;
};

}  // namespace invoke_detail

/**
 * @brief "Invoke" an invocable object. Like std::invoke, but cleaner and less debug overhead
 *
 * @tparam Func The invocable. Must be a function, or object that has an operator() overload
 * @tparam Args The arguments to pass to the invocable.
 *
 * This is the base overload that will catch things that are callable, including with operator()
 */
template <typename Func, typename... Args>
constexpr decltype(auto) invoke(Func&& fn,
                                Args&&... args)          //
    noexcept(invoke_detail::is_noexcept<Func, Args...>)  //
    requires requires {
    invoke_detail::invoker<Func>::invoke(NEO_FWD(fn), NEO_FWD(args)...);
}
{ return invoke_detail::invoker<Func>::invoke(NEO_FWD(fn), NEO_FWD(args)...); }

/// Overload for a regular member object pointer
template <typename Owner, typename T>
struct invoke_detail::invoker<T(Owner::*)> {
    static constexpr decltype(auto) invoke(T(Owner::*memptr), auto&& self)
        INVOKE_BODY(self.*memptr);
};

template <typename Owner, typename T>
struct invoke_detail::invoker<T(Owner::*&)> : invoker<T(Owner::*)> {};
template <typename Owner, typename T>
struct invoke_detail::invoker<T(Owner::*const&)> : invoker<T(Owner::*)> {};

/// Overload (group) of member function pointers
#define DECL_MEMFUN_INV(Qual)                                                                      \
    template <typename Ret, typename Owner, typename... FuncArgs>                                  \
    struct invoke_detail::invoker<Ret (Owner::*)(FuncArgs...) Qual> {                              \
        static constexpr decltype(auto)                                                            \
        invoke(Ret (Owner::*memfun)(FuncArgs...) Qual, auto&& self, auto&&... args)                \
            INVOKE_BODY((NEO_FWD(self).*memfun)(NEO_FWD(args)...));                                \
    };                                                                                             \
    template <typename Ret, typename Owner, typename... FuncArgs>                                  \
    struct invoke_detail::invoker<Ret (Owner::*&)(FuncArgs...) Qual>                               \
        : invoker<Ret (Owner::*)(FuncArgs...) Qual> {};                                            \
    template <typename Ret, typename Owner, typename... FuncArgs>                                  \
    struct invoke_detail::invoker<Ret (Owner::*const&)(FuncArgs...) Qual>                          \
        : invoker<Ret (Owner::*)(FuncArgs...) Qual> {}

DECL_MEMFUN_INV();
DECL_MEMFUN_INV(const);
DECL_MEMFUN_INV(volatile);
DECL_MEMFUN_INV(const volatile);
DECL_MEMFUN_INV(noexcept);
DECL_MEMFUN_INV(const noexcept);
DECL_MEMFUN_INV(volatile noexcept);
DECL_MEMFUN_INV(const volatile noexcept);

DECL_MEMFUN_INV(&);
DECL_MEMFUN_INV(const&);
DECL_MEMFUN_INV(volatile&);
DECL_MEMFUN_INV(const volatile&);
DECL_MEMFUN_INV(&noexcept);
DECL_MEMFUN_INV(const& noexcept);
DECL_MEMFUN_INV(volatile& noexcept);
DECL_MEMFUN_INV(const volatile& noexcept);

DECL_MEMFUN_INV(&&);
DECL_MEMFUN_INV(const&&);
DECL_MEMFUN_INV(volatile&&);
DECL_MEMFUN_INV(const volatile&&);
DECL_MEMFUN_INV(&&noexcept);
DECL_MEMFUN_INV(const&& noexcept);
DECL_MEMFUN_INV(volatile&& noexcept);
DECL_MEMFUN_INV(const volatile&& noexcept);

#undef DECL_MEMFUN_INV

#undef INVOKE_BODY

template <typename Func, typename... Args>
using invoke_result_t = decltype(neo::invoke(NEO_DECLVAL(Func), NEO_DECLVAL(Args)...));

}  // namespace neo
