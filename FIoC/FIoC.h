#pragma once

#include <string>
#include <functional>

namespace fioc
{


   class IFunction
   {
   public:
      virtual void* call() = 0;
   };


   template <typename R, typename ...ARGS>
   class MyFunction : public IFunction
   {
   public:
      R retVal;
      std::tuple<ARGS...> arguments;

      std::function<R(ARGS...)> f;

      virtual void* call()
      {
         this->retVal = this->callFunc(/*typename gen_seq2<sizeof...(ARGS)>::type()*/ typename std::index_sequence_for<ARGS...>{});
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
      using Map = _Map< std::string, IFunction*, Args ...>;

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
         MyFunction<T*, Args...> *wtf = static_cast<MyFunction<T*, Args...> *>(it->second);
         wtf->arguments = std::make_tuple(args...);
         return static_cast<T*>(it->second->call());

         //return static_cast<T*>(static_cast<std::function<void*(Args...)>>(it->second()));
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
         MyFunction<T*, Args...> *wtf = new MyFunction<T*, Args...>();
         wtf->f = [](Args... args){ return new T(args...);};
         container[typeid(T).name()] = wtf;
      }

   protected:
      Map container; /// Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };

}
