#pragma once

#include <FIoC/commons.h>

#include <typeindex>
#include <memory>

namespace fioc
{
   /**
    * Class that serves as a IoC builder registry but the type that is build doesn't need to relate to the type that is 'requested'.
    * It aims to provide a binding for two classes that are used together but one doesn't need to depend on another. It can be also used
    * to build certain family of classes based on the classes supplied as a 'key'.
    * 
    * Constructing simple container
    * \code{.cpp}
    * fioc::NonRefectiveRegistry<std::map> builder;
    * fioc::NonRefectiveRegistry<std::map, AbstractClass> builder2; //builder for the AbstractClass type objects
    * \endcode
    * 
    * Registering simple default constructible, we register Type A for type B (B is a 'key' type in this example)
    * \code{.cpp}
    * builder.registerType<A>().forType<B>();
    * A* a = (A*)builder.resolve<B>(); //returns new object of type A but casted as void*
    * 
    * builder2.registerType<A>().forType<B>(); // A should be now the subclass of AbstractClass
    * AbstractClass *subclass = builder.resolve<B>();    * 
    * \endcode
    * 
    * Register type A for type B but with different constructor or a factory
    * \code{.cpp}
    * builder.registerType<A>().buildWithConstructor<int>.forType<B>();
    * A* a = (A*)builder.resolve<B>(4);
    * 
    * //e.g. factory lambda
    * builder.registerType<A>().buildWithFactory<int>({[](int val){return new A(val + 4)}}).forType<B>();
    * A* a = (A*)builder.resolve<B>(4);
    * \endcode
    * 
    * You can also happen to have family of 'key' classes. You have only the pointer to some superclass of the key type (B).
    * You can use this pointer in the resolveByInstance(). First argument is the pointer in question and other arguments are the
    * arguments of the registered factory or constructor.
    * \code{.cpp}
    * SuperClassOfB *b = new B;
    * builder.registerType<A>().buildWithFactory<int>({[](int val){return new A(val + 4)}}).forType<B>();
    * A* a = (A*)builder.resolveByInstance(b, 4);    * 
    * \endcode
    * 
    * There has to be a certain order of the calls int the chains otherwise the item don't get registered or the resolve could return nullptr or
    * a not expected result. For example if you register type A to be resolved with a constructor/factory taking one int as a parameter and you 
    * don't supply it/them to the resolve call, you might get anything from possible crash (if the type A is badly designed), nullptr, default values
    * passed to the factory/ctor (first use) or the parameters of the last resolve call (that got saved there in the wrapper functor).
    * 
    * \tparam _Map Customizable container implementation. Should satisfy AssociativeContainer or UnorderedAssociativeContainer like std::map or std::unordered map.
    * \tparam _CommonType The type that should be returned by the resolve function. If not supplied the default is void*.
    */
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

      template<typename T, typename ...Args>
      CommonType* resolveByInstance(T* instance, Args...args)
      {
         Key key{typeid(*instance)};
         auto it = container.find(key);
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
         IntermediateReturn<CreatedType, Args...> buildWithConstructor()
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