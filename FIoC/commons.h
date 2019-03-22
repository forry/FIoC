#pragma once

#include <type_traits>
#include <typeindex>
#include <functional>
#include <memory>

namespace fioc
{
   /**
    * Pure abstract interface for default constructor call.
    */
   class DefaultConstructorFunctor
   {
   public:
      virtual void* operator()() = 0;
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