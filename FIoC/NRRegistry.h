#pragma once

#include <FIoC/commons.h>

namespace fioc
{
   template <template <typename ... > class _Map, typename _CommonType = void, typename ...Args>
   class NonRefectiveRegistry
   {
   public:
      using CommonType = _CommonType;
      using Key = std::type_index;
      using Value = std::unique_ptr<DefaultConstructorFunctor>;
      using Map = _Map< Key, Value, Args ...>; //< The type of internal container (might come in handy)

      template<typename BindType, typename ...Args>
      CommonType* resolve(Args... args)
      {
         auto it = container.find(Key{typeid(BindType)});
         if(it == container.end())
         {
            return nullptr;
         }
         FactoryFunctor<CommonType*, Args...> *factoryFunctor = static_cast<FactoryFunctor<CommonType*, Args...> *>(it->second.get());
         factoryFunctor->arguments = std::make_tuple(args...);
         return static_cast<CommonType*>((*it->second)());
      }

      template<typename CreatedType, typename...Args>
      struct IntermediateReturn
      {
         IntermediateReturn(Map& map, std::unique_ptr<FactoryFunctor<CreatedType*, Args...>> ff)
            : container(map)
         {
            factoryFunctor = std::move(ff);
         }

         template<typename BindType>
         void forType()
         {
            container.insert_or_assign(Key{typeid(BindType)}, std::move(factoryFunctor));
         }

         template<typename...Args>
         IntermediateReturn<CreatedType, Args...> withConstructor()
         {
            std::unique_ptr<FactoryFunctor<CreatedType*, Args...>> factoryFunctor = std::make_unique<FactoryFunctor<CreatedType*, Args...>>();
            factoryFunctor->f = [](Args... args) { return new CreatedType(args...); };

            return IntermediateReturn<CreatedType, Args...>(container, std::move(factoryFunctor));
         }

         template<typename ...Args>
         IntermediateReturn<CreatedType, Args...> buildWithFactory(std::function<CreatedType*(Args...)> f)
         {
            std::unique_ptr<FactoryFunctor<CreatedType*, Args...>> factoryFunctor = std::make_unique<FactoryFunctor<CreatedType*, Args...>>();
            factoryFunctor->f = f;

            return IntermediateReturn<CreatedType, Args...>(container, std::move(factoryFunctor));
         }

      protected:

         Map& container;
         std::unique_ptr<FactoryFunctor<CreatedType*, Args...>> factoryFunctor;
      };

      template<typename CreatedType>
      IntermediateReturn<CreatedType> registerType()
      {
         std::unique_ptr<FactoryFunctor<CreatedType*>> factoryFunctor;

         if constexpr(std::is_default_constructible_v<CreatedType>)
         {
            factoryFunctor = make_unique<FactoryFunctor<CreatedType*>>();
            factoryFunctor->f = []() { return new CreatedType(); };
         }
         else
         {
            factoryFunctor = make_unique<NullFactory<CreatedType*>>();
         }

         return IntermediateReturn<CreatedType>{container, std::move(factoryFunctor)};
      }


   protected:
      Map container; //< Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };
}