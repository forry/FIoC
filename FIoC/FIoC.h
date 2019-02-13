#pragma once

#include <string>
#include <functional>

namespace fioc
{

   template<typename _ReturnType, typename ...Arguments>
   class Functor
   {
   public:
      using ReturnType = _ReturnType;

      virtual ReturnType operator()(Arguments...args) = 0;
   };

   using ConstructorFunctor = Functor<void*>;


   template <typename R, typename ...ARGS>
   class FactoryFunctor : public ConstructorFunctor
   {
   public:
      R retVal;
      std::tuple<ARGS...> arguments;

      std::function<R(ARGS...)> f;

      virtual ConstructorFunctor::ReturnType operator()() override
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
      //using Map = _Map< std::string, std::function<void*()>, Args ...>;
      using Map = _Map< std::string, ConstructorFunctor*, Args ...>;

      /*template<typename T>
      T* resolve()
      {
         auto it = container.find(typeid(T).name());
         if(it == container.end())
         {
            return nullptr;
         }
         return static_cast<T*>(it->second());
      }*/

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

      template<typename RegisterType, typename AsType>
      void registerTypeAs()
      {
         
         container[typeid(RegisterType).name()] = []() {return new AsType; };
      }


      template<typename T>
      struct ImmRet
      {
         using type = typename T;

         ImmRet(Map &c):container(c){}

         template<typename AS>
         void as()
         {
            container[typeid(T).name()] = []() {return new AS; };
         }

         Map& container;
      };

      /*template<typename T>
      ImmRet<T> registerType()
      {
         //update semantic

         container[typeid(T).name()] = []() {return new T; };

         return ImmRet<T>{container};
      }*/

      template<typename T, typename ...Args>
      void registerType()
      {
         FactoryFunctor<T*, Args...> *factoryFunctor = new FactoryFunctor<T*, Args...>();
         factoryFunctor->f = [](Args... args){ return new T(args...);};
         container[typeid(T).name()] = factoryFunctor;
      }

   protected:
      Map container; /// Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };

}
