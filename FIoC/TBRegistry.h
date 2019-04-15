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
    * fioc::TBRegistry<std::map> builder;
    * fioc::TBRegistry<std::map, AbstractClass> builder2; //builder for the AbstractClass type objects
    * \endcode
    * 
    * Registering simple default constructible, we register Type A for type B (B is a 'key' type in this example)
    * \code{.cpp}
    * builder.registerType<A>().forType<B>();
    * A* a = (A*)builder.resolve<B>(); //returns new object of type A but casted as void*
    * 
    * builder2.registerType<A>().forType<B>(); // A should be now the subclass of AbstractClass
    * AbstractClass *subclass = builder.resolve<B>();
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
    * A* a = (A*)builder.resolveByInstance(b, 4);
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
   class TBRegistry
   {
   public:
      using CommonType = _CommonType;
      using Key = std::type_index;
      using Value = std::unique_ptr<DefaultConstructorFunctor>;
      using Map = _Map< Key, Value, Args ...>; //< The type of internal container (might come in handy)
      

      /**
       * Creates Object of the type that was binded for type BindType. If no object was bound to that type it returns nullptr.
       * 
       * \tparam BindType The "key" type.
       * \tparam Args Argument types for the constructor of a factory function that was registered with BindType.
       * \param args Arguments for the factory/ctor.
       * \return The pointer to a newly created object (and its ownership) casted as CommonType*.
       */
      template<typename BindType, typename  CommonType_ = CommonType, typename ...Args>
      std::enable_if_t<!is_unique_ptr_v<CommonType>, CommonType_*> resolve(Args... args)
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

      template<typename BindType, typename  CommonType_ = CommonType, typename ...Args>
      std::enable_if_t<is_unique_ptr_v<CommonType>, CommonType_> resolve(Args... args)
      {
         auto it = container.find(Key{typeid(BindType)});
         if(it == container.end())
         {
            return nullptr;
         }
         FactoryFunctor<CommonType::pointer, Args...> *factoryFunctor = static_cast<FactoryFunctor<CommonType::pointer, Args...> *>(it->second.get());
         factoryFunctor->arguments = std::make_tuple(args...);
         
         return CommonType(static_cast<CommonType::pointer>((*factoryFunctor)()));
      }

      /**
       * This function is a modification of the resolve(). It adds the possibility to get the BindType ("key" type) from the pointer to an existing object.
       * it applies the typeid operator to instance so its type can be a pointer to a super class. This is for the use-case when you have e.g. a collection of
       * pointers to interfaces of types that are registered and you want to created the object of the bound types without knowing exactly which types the pointers
       * in collection are referring to.
       * 
       * \tparam T Superclass of some "key" type.
       * \tparam Args Types of arguments for the registered ctor/factory.
       * \param instance Pointer to an existing instance of some registered "key" type.
       * \param args Arguments for the registered ctor/factory.
       * \return Returns the newly created object (and its ownership) for the bound type cast as CommonType*.
       */
      template<typename T, typename  CommonType_ = CommonType, typename ...Args>
      std::enable_if_t<!is_unique_ptr_v<CommonType>, CommonType_*> resolveByInstance(T* instance, Args...args)
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

      template<typename T, typename  CommonType_ = CommonType, typename ...Args>
      std::enable_if_t<is_unique_ptr_v<CommonType>, CommonType_> resolveByInstance(T* instance, Args...args)
      {
         Key key{typeid(*instance)};
         auto it = container.find(key);
         if(it == container.end())
         {
            return nullptr;
         }
         FactoryFunctor<CommonType::pointer, Args...> *factoryFunctor = static_cast<FactoryFunctor<CommonType::pointer, Args...> *>(it->second.get());
         factoryFunctor->arguments = std::make_tuple(args...);
         return CommonType(static_cast<CommonType::pointer>((*factoryFunctor)()));
      }

      template<typename CreatedType, typename...Args>
      struct IntermediateReturn
      {
         IntermediateReturn(Map& map, std::unique_ptr<FactoryFunctor<CreatedType*, Args...>> ff)
            : container(map)
         {
            factoryFunctor = std::move(ff);
         }

         /**
          * Last function of the register chain which states the "key" type we are registering the bound type to.
          * \tparam KeyType Type for which we are registering the associated bound type.
          * 
          * \see TBRegistry for example usage.
          */
         template<typename KeyType>
         void forType()
         {
            container.insert_or_assign(Key{typeid(KeyType)}, std::move(factoryFunctor));
         }

         /**
          * This configures the bound type to be constructed with constructor with the signature given by the template Args.
          * 
          * \tparam Args This pack is used to select the constructor of the bound type.
          * \return Returns the object for the chain continuation.
          * 
          * \see TBRegistry for example usage.
          */
         template<typename...Args>
         IntermediateReturn<CreatedType, Args...> buildWithConstructor()
         {
            std::unique_ptr<FactoryFunctor<CreatedType*, Args...>> factoryFunctor = std::make_unique<FactoryFunctor<CreatedType*, Args...>>();
            factoryFunctor->f = [](Args... args) { return new CreatedType(args...); };

            return IntermediateReturn<CreatedType, Args...>(container, std::move(factoryFunctor));
         }

         /**
          * This configures the bound type to be constructed with factory with the signature given by the template Args.
          * 
          * \tparam Args Signature of the factory function.
          * \param f The factory function used to create the object of the bound type. The argument is a std::function which implies many possible uses.
          * \return Returns the object for the chain continuation.
          */
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

      /**
       * Begins the function chain for registering a BoundType to a 'key' type. If this call is not continued by
       * buildWith* method the default constructor is assumed. Then if the BoundType doesn't have the default constructor
       * the NullFactory is supplied resulting in the resolve call always returning null object. Note that unless the chain is ended
       * with forType() method the registration is NOT complete a nd NOTHING gets registered. The ongoing configuration is
       * passes by the return of the chain.
       * 
       * \tparam BoundType Type of the object that is desired to be constructed via a resolve call later.
       * \return Returns the object for the chain continuation.
       */
      template<typename BoundType>
      IntermediateReturn<BoundType> registerType()
      {
         std::unique_ptr<FactoryFunctor<BoundType*>> factoryFunctor;

         if constexpr(std::is_default_constructible_v<BoundType>)
         {
            factoryFunctor = make_unique<FactoryFunctor<BoundType*>>();
            factoryFunctor->f = []() { return new BoundType(); };
         }
         else
         {
            factoryFunctor = make_unique<NullFactory<BoundType*>>();
         }

         return IntermediateReturn<BoundType>{container, std::move(factoryFunctor)};
      }


   protected:
      Map container; //< Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };
}