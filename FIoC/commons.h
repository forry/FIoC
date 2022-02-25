#pragma once

#include <type_traits>
#include <functional>

namespace fioc
{
   /**
    * Common class for functors so that we can specify a type for the container value.
    */
   class DefaultConstructorFunctor
   {
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
      std::function<R(ARGS...)> f;
   };

   template <typename R, typename ...ARGS>
   class NullFactory : public FactoryFunctor<R, ARGS...>
   {
   public:
      NullFactory() : FactoryFunctor<R, ARGS...>()
      {
         this->f = [](ARGS... args) {return nullptr; };
      }
   };
}