#pragma once

#include <FIoC/commons.h>

#include <typeindex>
#include <memory>

namespace fioc
{
   /**
    * IoC container implementation that mimics some of the C# Unity IoC features.
    * It supports mocking and types that are not default-constructible.
    *
    * Example usages:
    *
    * Constructing simple IoC container.
    * \code{.cpp}
    * fioc::Builder<std::map> builder;
    * \endcode
    *
    * Simple 1:1 registration of the default constructible type.
    * \code{.cpp}
    * builder.registerType<A>(); //register type A as type A
    * unique_ptr<A> a(builder.resolve<A>()); //constructing of the object of type A
    * \endcode
    *
    * Registering type C as type A. Mostly that means that that A is and interface and C is its implementation, that user does not need to be aware of. That means that C must be subclass of A.
    * \code{.cpp}
    * builder.registerType<A>().as<C>(); //Register type C to be constructed when the A is asked for
    * unique_ptr<A> a(builder.resolve<A>()); //Returns the pointer of type A to newly constructed object of type C.
    * \endcode
    *
    * Register not default constructible Type under it subclass.
    * \code{.cpp}
    * builder.registerType<NoDefaultCtor, int, int>().as<NoDefaultCtorSub>(); //Register type NoDefaultCtor implemented by NoDefaultCtorSub  with Constructor that takes exactly two ints as arguments.
    * unique_ptr<NoDefaultCtor> n2(builder.resolve<NoDefaultCtor>(3, 4)); //Construct the NoDefaultCtorSub object with 3 and 4 as the constructor arguments (warning no implicit type deduction/casting here) and returns a pointer to NoDefaultCtor super class
    * \endcode
    *
    * \see main.cpp
    *
    * \tparam _Map Customizable container implementation. Should satisfy AssociativeContainer or UnorderedAssociativeContainer like std::map or std::unordered map.
    * \tparam Args Remaining template arguments the _Map type can have in addition to key and value type.
    */
   template <template <typename ... > class _Map, typename ...Args>
   class Registry
   {
   public:
      using Key = std::type_index;
      using Value = std::unique_ptr<DefaultConstructorFunctor>;
      using Map = _Map< Key, Value, Args ...>; //< The type of internal container (might come in handy)


      /**
       * Constructs the object of the requested type.
       * If the requested type has not been registered the call returns null.
       *
       * \tparam T Type of requested object.
       * \tparam Args Arguments to the registered constructor. Must be the same as the registerType call. Compiler infers them automatically.
       * \param args Actual arguments to the constructor. The types mus be exact. No implicit conversion here due to the template resolution.
       * \return The pointer to a newly created object (the caller assumes the ownership) or null if the type hasn't been registered.
       *
       * \see Builder
       */
      template<typename T, typename ... Args >
      T* resolve(Args... args)
      {
         auto it = container.find(Key{typeid(T)});
         if(it == container.end())
         {
            return nullptr;
         }
         FactoryFunctor<T*, Args...> *factoryFunctor = static_cast<FactoryFunctor<T*, Args...> *>(it->second.get());
         return factoryFunctor->f(args...);

      }


      /**
       * This is intermediate return structure from the registerType() method.
       * It allows us to chain the as() method to the registerType() call and inject another type
       * for the later resolution (mocking).
       *
       * \tparam T Original type we are mocking.
       */
      template<typename T>
      struct IntermediateReturn
      {
         using type = typename T;

         IntermediateReturn(Map &c) :container(c) {}

         /**
          * Replaces the constructor functor of type T with the constructor functor of type As. It is intended to
          * chain this call to the registerType call. Provides mocking functionality.
          *
          * \tparam As New type we construct when asked for type T. The type As has to be the same or subclass of T.
          *
          * \see Builder for example usages.
          */
         template<typename As>
         IntermediateReturn<T> as()
         {
            static_assert(std::is_base_of_v<T, As>, "Template type As is not a subclass of T");

            FactoryFunctor<T*> *factoryFunctor;

            if constexpr(std::is_default_constructible_v<As>)
            {
               factoryFunctor = new FactoryFunctor<T*>();
               factoryFunctor->f = []() { return new As(); };
            }
            else
            {
               factoryFunctor = new NullFactory<T*>();
            }

            container.insert_or_assign(Key{typeid(T)}, Value{factoryFunctor});

            return *this;
         }

         template<typename ...Args>
         void buildWithConstructor()
         {
            FactoryFunctor<T*, Args...> *factoryFunctor = new FactoryFunctor<T*, Args...>();
            factoryFunctor->f = [](Args... args) { return new T(args...); };
            container.insert_or_assign(Key{typeid(T)}, Value{factoryFunctor});
         }

         template<typename ...Args>
         void buildWithFactory(std::function<T*(Args...)> f)
         {
            FactoryFunctor<T*, Args...> *factoryFunctor = new FactoryFunctor<T*, Args...>();
            factoryFunctor->f = f;
            container.insert_or_assign(Key{typeid(T)}, Value{factoryFunctor});
         }

      protected:
         Map& container;
      };

      /**
       * Registers the constructor (factory) for type T with default constructor or factory that resolves to nullptr when the T is not default constructible.
       * When in need of a different constructor or a custom factory to be used chain this with the other calls.
       *
       * \tparam T Type to register.
       * \return Used for convenient mocking syntax \see IntermediateReturn.
       * 
       * \see Registry usage examples.
       */
      template<typename T>
      IntermediateReturn<T> registerType()
      {
         FactoryFunctor<T*> *factoryFunctor;

         if constexpr(std::is_default_constructible_v<T>)
         {
            factoryFunctor = new FactoryFunctor<T*>();
            factoryFunctor->f = []() { return new T(); };
         }
         else
         {
            factoryFunctor = new NullFactory<T*>();
         }

         container.insert_or_assign(Key{typeid(T)}, Value{factoryFunctor});
         return IntermediateReturn<T>{container};
      }

   protected:
      Map container; //< Map is e.g. std::map<std::string, std::function<void*()> >. container holds the factory functions
   };

}
