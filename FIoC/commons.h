#pragma once

#include <type_traits>
#include <functional>

namespace fioc
{

   template <class T> struct is_unique_ptr : std::false_type {};
   template <class T> struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

   template<typename T>
   inline constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

   /**
    * Pure abstract interface for default constructor call.
    */
   class DefaultConstructorFunctor
   {
   public:
      virtual void* operator()() = 0;
   };

   template<typename _Ptr>
   struct CustomDeleter
   {
      void operator()(_Ptr* ptr)
      {
         delete ptr;
      }
   };


   /**
    * This is general function call wrapper.
    * This allows you to call the function you don't know nothing about (return value, arguments) until the compile time.
    * It gives us the possibility to call an arbitrary constructor on the registered type.
    *
    * \tparam R Return type.
    * \tparam ARGS Call arguments.
    */
   template <typename R, typename ...ARGS>
   class FactoryFunctor : public DefaultConstructorFunctor
   {
   public:
      R retVal;
      std::tuple<ARGS...> arguments;

      std::function<R(ARGS...)> f;

      virtual void* operator()() override
      {
         this->retVal = this->callFunc(typename std::index_sequence_for<ARGS...>{});
         return retVal;
      };

      template<int ...S>
      R callFunc(std::index_sequence<S...>)
      {
         return f(std::get<S>(arguments) ...);
      }
   };

   template <typename R, typename ...ARGS>
   class NullFactory : public FactoryFunctor<R, ARGS...>
   {
   public:
      NullFactory() : FactoryFunctor<R, ARGS...>()
      {
         f = [](ARGS... args) {return nullptr; };
      }
   };
}