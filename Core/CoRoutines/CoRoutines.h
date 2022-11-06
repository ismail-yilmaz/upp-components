#ifndef _Upp_CoRoutines_h_
#define _Upp_CoRoutines_h_

#include <Core/Core.h>

#include <concepts>
#include <coroutine>
#include <type_traits>

namespace Upp {
    
using CoSuspend     = std::suspend_always;
using CoDontSuspend = std::suspend_never;


enum class CoRoutineType { Routine, Generator };

template<CoRoutineType R, typename T>
class CoRoutineT {

    template<class C, typename U>
    struct PromiseType
    {
        C get_return_object()                           { return C(C::Handle::from_promise(static_cast<U&>(*this))); }
        std::suspend_always initial_suspend()           { return {}; }
        std::suspend_always final_suspend() noexcept    { return {}; }
        void unhandled_exception()                      { exc = std::current_exception(); }
        std::exception_ptr exc;
    };
    
    template<typename U>
    struct ReturnValue : PromiseType<CoRoutineT, ReturnValue<U>>
    {
        template<std::convertible_to<U> L>
        void return_value(L&& val) { value = std::forward<L>(val); }
        void yield_value(U&&) = delete;
        U value {};
    };

    template<typename U>
    struct VoidValue : PromiseType<CoRoutineT, VoidValue<U>>
    {
        void return_void() {}
    };
    
    template<typename U>
    struct CurrentValue : PromiseType<CoRoutineT, CurrentValue<U>>
    {
        template<std::convertible_to<U> L>
        std::suspend_always yield_value(L&& val) { value = std::forward<L>(val); return {}; }
        std::suspend_never await_transform(U&&) = delete;
        void return_value(U&&) = delete;
        U value;
    };

public:
    using promise_type =
        typename std::conditional<
            std::is_void_v<T>,
                VoidValue<T>, typename std::conditional<
                    R == CoRoutineType::Routine,
                        ReturnValue<T>, CurrentValue<T>>::type>::type;
                        
    using Handle = typename std::coroutine_handle<promise_type>;

    bool Do()
        requires (R == CoRoutineType::Routine)
    {
        ASSERT(co);
        co.resume();
        Rethrow();
        return !co.done();
    }
    
    T Next() const
        requires (R == CoRoutineType::Generator)
    {
        ASSERT(co);
        co.resume();
        Rethrow();
        return co.promise().value;
    }

    T operator~() const
        requires (R == CoRoutineType::Generator)
    {
        return Next();
    }

    T PickNext()
        requires (R == CoRoutineType::Generator)
    {
        ASSERT(co);
        co.resume();
        Rethrow();
        return pick(co.promise().value);
    }

    T Get() const
        requires (R == CoRoutineType::Routine && !std::is_void_v<T>)
    {
        ASSERT(co);
        return co.promise().value;
    }

    T operator~() const
        requires (R == CoRoutineType::Routine && !std::is_void_v<T>)
    {
        return Get();
    }

    T Pick()
       requires (R == CoRoutineType::Routine && !std::is_void_v<T>)
    {
        ASSERT(co);
        return pick(co.promise().value);
    }

    CoRoutineT(Handle h)
    : co(h)
    {
    }
    
    CoRoutineT& operator=(CoRoutineT& r) noexcept
    {
        if(this != &r) {
            if(co)
                co.destroy();
            co = r.co;
            r.co = {};
        }
        return *this;
    }

    virtual ~CoRoutineT() noexcept
    {
        if(co)
            co.destroy();
    }
    
    CoRoutineT(CoRoutineT&& r) noexcept = default;
    CoRoutineT(const CoRoutineT&) = delete;
    CoRoutineT& operator=(const CoRoutineT&) = delete;


    // Iterator support for generator type.
    template<typename C, class U>
    class IteratorT {
    public: // C++20 iterator tags...
        using value_type        = U;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        value_type  operator*() const   { return cogen.co.promise().value;   }
        value_type  operator->() const  { return cogen.co.promise().value;   }
        IteratorT& operator++()         { cogen.Rethrow(); cogen.co.resume(); return *this; }
        IteratorT& operator++(int)      { cogen.Rethrow(); cogen.co.resume(); return *this; }
        bool operator==(const IteratorT&) const { return false; }
    
        IteratorT() = delete;
        IteratorT(C&&) = delete;
        IteratorT(C& r) : cogen{r}     {}
    
    private:
        C& cogen;
    };
    
    using Iterator = class IteratorT<CoRoutineT<CoRoutineType::Generator, T>, T>;
    
    Iterator begin()
        requires (R == CoRoutineType::Generator)
    {
        return ++Iterator(*this);
    }

    
    Iterator end()
        requires (R == CoRoutineType::Generator)
    {
        return Iterator(*this);
    }

private:
    void Rethrow() const
    {
        if(co.promise().exc)
            std::rethrow_exception(co.promise().exc);
    }

private:
    Handle co;
};

template<typename T>
using CoRoutine = CoRoutineT<CoRoutineType::Routine, T>;
template<typename T> requires (!std::is_void_v<T>)
using CoGenerator = CoRoutineT<CoRoutineType::Generator, T>;
}

#endif
