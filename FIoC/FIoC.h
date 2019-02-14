#pragma once

#include <string>
#include <functional>

namespace fioc
{
   class DefaultConstructorFunctor
   {
   public:
      virtual void* operator()() = 0;
   };


   template <typename R, typename ...ARGS>
   class FactoryFunctor : public DefaultConstructorFunctor
   {
   public:
      R retVal;
      std::tuple<ARGS...> arguments;

      std::function<R(ARGS...)> f;

      virtual void* operator()() override
      {
         this->retVal = this->callFunc( typename std::index_sequence_for<ARGS...>{});
         return retVal;
      };

      template<int ...S>
      R callFunc(std::index_sequence<S...>)
      {
         return f(std::get<S>(arguments) ...);
      }
   };
   
   template <template <typename ... > class _Map, typename ...Args>
   class Builder
   {
   public:
      using Map = _Map< std::string, DefaultConstructorFunctor*, Args ...>;

      template<typename T, typename ... Args >
      T* resolve(Args... args)
      {
         auto it = container.find(typeid(T).name());
         if(it == container.end())
         {
            return nullptr;
         }
         FactoryFunctor<T*, Args...> *factoryFunctor = static_cast<FactoryFunctor<T*, Args...> *>(it->second);
         factoryFunctor->arguments = std::make_tuple(args...);
         return static_cast<T*>((*it->second)());

      }

      template<typename T, typename ...Args>
      struct ImmRet
      {
         using type = typename T;

         ImmRet(Map &c):container(c){}

         template<typename As>
         void as()
         {
            static_assert(std::is_base_of_v<T,As>, "Template type As is not a subclass of T");

            FactoryFunctor<T*, Args...> *factoryFunctor = new FactoryFunctor<T*, Args...>();
            factoryFunctor->f = [](Args... args) { return new As(args...); };
            container[typeid(T).name()] = factoryFunctor;
         }

      protected:
         Map& container;
      };

      template<typename T, typename ...Args>
      ImmRet<T, Args...> registerType()
      {
         FactoryFunctor<T*, Args...> *factoryFunctor = new FactoryFunctor<T*, Args...>();
         factoryFunctor->f = [](Args... args){ return new T(args...);};
         container[typeid(T).name()] = factoryFunctor;

         return ImmRet<T, Args...>{container};
      }

   protected:
      Map container; /// Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };

}
