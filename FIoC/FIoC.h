#pragma once

#include <string>
#include <functional>

namespace fioc
{
   
   template <template <typename ... > class _Map, typename ...Args>
   class Builder
   {
   public:
      using Map = _Map< std::string, std::function<void*()>, Args ...>;

      template<typename T>
      T* resolve()
      {
         auto it = container.find(typeid(T).name());
         if(it == container.end())
         {
            return nullptr;
         }
         return static_cast<T*>(it->second());
      }

      template<typename T, typename ... Args >
      T* resolve(Args... args)
      {
         return new T(args...);
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

      template<typename T>
      ImmRet<T> registerType()
      {
         //update semantic
         container[typeid(T).name()] = []() {return new T; };

         return ImmRet<T>{container};
      }

   protected:
      Map container; /// Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };

}
