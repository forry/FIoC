#include <FIoC/FIoC.h>
#include <iostream>
#include <memory>
#include <map>

using namespace std;

class A
{
public:
   A()
   {}

   virtual int get(){return 1;}
};

class B
{
public:
   B(int i=0): val(i)
   {}
   int get(){return val;}
   int val;
};

class C:public A
{
public:
   int get() override
   {
      return 2;
   }
};

class D
{
public:
   int get()
   {
      return 3;
   }
};

class NoDefaultCtor
{
public:
   NoDefaultCtor(int a, int b)
   : x(a)
   , y(b)
   {
      
   }

   int get()
   {
      return x+y;
   }

   int x,y;
};

class NoDefaultCtorSub: public NoDefaultCtor
{
public:
   NoDefaultCtorSub(int a, int b)
      : NoDefaultCtor(a,b)
   {

   }

   int get()
   {
      return x - y;
   }

};

class FalseInheritance
{
public:
   FalseInheritance(int a,int b)
      : x(a)
      , y(b)
   {

   }

      int get()
   {
      return x + y;
   }

   int x, y;
};

class Factory
{
public:
   static NoDefaultCtorSub * create ()
   {
      return new NoDefaultCtorSub(5,6);
   }
};

class AnotherFac
{
public:
   NoDefaultCtorSub *operator()(int a) const
   {
      return new NoDefaultCtorSub(a, b);
   }

   int b;
};

//
class TypeProvider
{
public:
   using ProvidedType = A;

   virtual ~TypeProvider(){} //needed at least one virtual method to force the polymorfic type and VMT creation
};

class TypeProviderSub1 : public TypeProvider
{
public:
   using ProvidedType = B;

};

class MyKey: public string
{
public:
   MyKey() = delete;
   MyKey(const std::type_info& ti)
      :string(ti.name())
   {}

   friend 
   ostream& operator<<(ostream& out, const MyKey& v)
   {
      return out << v.c_str();
   }
};


template<typename _ReturnType, typename ...Arguments>
class Functor
{
public:
   using ReturnType = _ReturnType;

   virtual ReturnType operator()(Arguments...args) = 0;
};

class I2I : public std::function<string(int)>
{
public:
   I2I(string val)
   : x(std::move(val))
   {}
      
   virtual result_type operator()(int val) const
   {
      return x;
   }
protected:
   string x;
};

class Increment : public Functor<Increment>
{
public:
   int val;
   Increment(int x=0)
   : val(x)
   {}
   Increment operator()()override
   {
      return Increment{++val};
   }

   friend
   ostream& operator<<(ostream& out, const Increment& v)
   {
      return out << v.val;
   }
};


class Creator
{
public:
   class Selector
   {
   public:

      template<typename T>
      I2I operator[](const T& id)
      {
         return I2I{typeid(id).name()};
      }

      /*I2I operator[](std::type_index&& id)
      {
         return I2I{id.name()};
      }*/
   };

   Selector create;
};

class PDMSugar : fioc::Registry<std::map>
{
public:
  //Selector& createPDMforProduct = create; //name alias
};

template<typename _CommonType, typename T>
class TemplateAgregatePOC
{
public:
   typedef _CommonType CommonType;
   typedef T Type;
};

template<typename  Agregate>
class AgregateUser
{
public:
   typedef typename Agregate::CommonType CT;
   typedef typename Agregate::Type T;
};

class Calls
{
   class ReturnType
   {
   public:
      constexpr ReturnType()
      {}

      ReturnType& b()
      {
         state.b++;
         return *this;
      }

      ~ReturnType()
      {
         if(state.b != 1)
         {
            //report error
            cout << "inconsistent Calls state" << endl;
         }
      }
   protected:
      struct State
      {
         unsigned b = 0;
      } state;

   };
public:
   ReturnType a()
   {
      return ReturnType{};
   }
};

class UniquePtrFactory
{
public:
   static unique_ptr<A> create()
   {
      return make_unique<C>();
   }
};



int main(int argc, char* argv[])
{
   cout << std::boolalpha;
   ///
   TypeProvider tpr;
   TypeProviderSub1 tps1;
   TypeProvider *t = &tps1;
   cout << MyKey{typeid(B)} << endl;
   cout << "name " << typeid(*t).name() << endl;
   //cout << "name " << decltype(t) << endl;
   cout << "name " <<typeid(remove_pointer_t<decltype(*t)>).name() << endl;

   ///
   AgregateUser<TemplateAgregatePOC<A,B> > agrPOC;
   cout << "Agregate " << typeid(decltype(agrPOC)::CT).name() << " " << typeid(decltype(agrPOC)::T).name() << endl;
   ///
   //Increment i(0);
   cout << "increment " << Increment()()()()()() << endl;
   ///
   Calls calls;
   calls.a().b();
   ///

   /*PDMRegistry.registerPDM<PDM>().forProduct<Product>();
   PDMRegistry.createPDMforProduct[product*](product*,viewer*);*/
   ///

   unique_ptr<A> una(UniquePtrFactory::create());

   cout << "unique factory una " << una->get() << " " << (una->get() == 2) << endl;

   ///

   fioc::TBRegistry<std::map> nrBuilder;
   nrBuilder.registerType<C>().forType<B>();
   //nrBuilder.registerType<NoDefaultCtor>().forType<A>();
   //unique_ptr<A> nrResolved( static_pointer_cast<A>(nrBuilder.resolve<B>()) );
   unique_ptr<A> nr(static_cast<A*>(nrBuilder.resolve<B>()));
   //unique_ptr<A> nrResolved = static_pointer_cast<A>(std::move(nr));
   cout << "nrresolve " << nr->get() << endl;
   /*
   B bb;
   unique_ptr<A> nrbi(static_cast<A*>(nrBuilder.resolveByInstance(&bb)));
   cout << "nrbi " << nrbi->get() << endl;


   nrBuilder.registerType<NoDefaultCtor>().buildWithConstructor<int, int>().forType<A>();
   unique_ptr<NoDefaultCtor> nr2(static_cast<NoDefaultCtor*>(nrBuilder.resolve<A>(1, 1)));
   cout << "nr2 " << (nr2->get() == 2) << endl;

   nrBuilder.registerType<D>().forType<C>();
   unique_ptr<A> ac(make_unique<C>());
   unique_ptr<A> aa(make_unique<A>());
   unique_ptr<NoDefaultCtor> nrbi3(static_cast<NoDefaultCtor*>(nrBuilder.resolveByInstance(aa.get())));
   if(!nrbi3)
      cout << "nrbi is null as it should be " << endl;
   else
      cout << "some other type than expected " << nrbi3->get() << endl;

   unique_ptr<D> nrbi2(static_cast<D*>(nrBuilder.resolveByInstance(ac.get())));
   cout << "nrbi " << nrbi2->get() << endl;

   //nrResolved.reset( nrBuilder.resolve<A>());
   //cout << "nrresolve " << nrResolved->get() << endl; //runtime access violation as expected

   unique_ptr<D> nrr(static_cast<D*>(nrBuilder.resolve<A>()));
   if(nrr)
      cout << "nrresolve " << nrr->get() << endl;
   else
      cout << "nrr is null" << endl;


   nrBuilder.registerType<NoDefaultCtorSub>().buildWithFactory({Factory::create}).forType<A>();
   unique_ptr<NoDefaultCtorSub> nr3(static_cast<NoDefaultCtorSub*>(nrBuilder.resolve<A>()));
   cout << "nr3 " << nr3->get() << " " << (nr3->get() == -1) << endl;

   nrBuilder.registerType<NoDefaultCtorSub>().buildWithFactory({[](){ return new NoDefaultCtorSub(3,6);}}).forType<A>();
   unique_ptr<NoDefaultCtorSub> nr4(static_cast<NoDefaultCtorSub*>(nrBuilder.resolve<A>()));
   cout << "nr4 " << nr4->get() << " " << (nr4->get() == -3) << endl;
   */
   

   /////////////////////////////
   
   fioc::Registry<std::map> builder;
   builder.registerType<A>();
   builder.registerType<A>();
   unique_ptr<A> a(builder.resolve<A>());
   if(a)
      cout << "A " << a->get() <<endl;
   else
      cout << "A not found" << endl;

   //builder.registerTypeAs<A,C>();
   builder.registerType<A>().as<C>();
   a.reset(builder.resolve<A>());
   if(a)
      cout << "A as C " << a->get() << endl;
   else
      cout << "A not found" << endl;
     

   builder.registerType<B>().buildWithConstructor<int>();
   unique_ptr<B> b((builder.resolve<B>(32)));
   if(b)
      cout << "B " << b->get() << endl;
   else cout << "not b" << endl;

   builder.registerType<B>().buildWithConstructor<>();
   unique_ptr<B> b1((builder.resolve<B>()));
   cout << "B1 " << b1->get() << endl;
   
   builder.registerType<B>();
   unique_ptr<B> b2((builder.resolve<B>()));
   cout << "B2 " << b2->get() << endl;


   builder.registerType<NoDefaultCtor>().buildWithConstructor<int,int>();
   unique_ptr<NoDefaultCtor> n(builder.resolve<NoDefaultCtor>(3,7));
   cout << "NoDef " << n->get() << endl;
   
   builder.registerType<NoDefaultCtor>().as<NoDefaultCtorSub>().buildWithConstructor<int,int>();
   unique_ptr<NoDefaultCtor> n2(builder.resolve<NoDefaultCtor>(3, 4));
   cout << "NoDef " << n2->get() << endl;

   std::function<NoDefaultCtorSub*()> fac = Factory::create;

   builder.registerType<NoDefaultCtorSub>().buildWithFactory({Factory::create});
   unique_ptr<NoDefaultCtor> n3(builder.resolve<NoDefaultCtorSub>());
   cout << "NoDef " << n3->get() << endl;

   builder.registerType<NoDefaultCtor>().as<NoDefaultCtorSub>().buildWithFactory({Factory::create});
   unique_ptr<NoDefaultCtor> n4(builder.resolve<NoDefaultCtor>());
   cout << "NoDef " << n4->get() << endl;

   AnotherFac anotherFactory{2}; // register builder with one parameter stored and one requesting by resolve call
   //std::function<NoDefaultCtorSub*(const AnotherFac&,int)> anfacfunc = &AnotherFac::operator();
   //builder.registerType<NoDefaultCtor>().as<NoDefaultCtorSub>().buildWithFactory<int>({std::bind(std::function<NoDefaultCtorSub*(const AnotherFac&,int)>{&AnotherFac::operator()},anotherFactory,std::placeholders::_1)});
   builder.registerType<NoDefaultCtor>().as<NoDefaultCtorSub>().buildWithFactory<int>({[anotherFactory](int x){return anotherFactory(x);}});
   unique_ptr<NoDefaultCtor> n5(builder.resolve<NoDefaultCtor>(7));
   cout << "NoDef " << n5->get() << endl;

   //Compile-time error - which is good since not only A is not a subclass of NoDefaultCtor but it doesn't have appropriate ctor signature
   /*
   builder.registerType<NoDefaultCtor>().as<A>().buildWithConstructor<int,int>();
   unique_ptr<NoDefaultCtor> e3(builder.resolve<NoDefaultCtor>(3, 4));
   */

   // Another good compile-time error due to FalseInheritance is not a subclass of NoDefaultCtor
   /*
   builder.registerType<NoDefaultCtor>().as<FalseInheritance>();
   unique_ptr<NoDefaultCtor> e4(builder.resolve<NoDefaultCtor>(3, 4));
   */

   return 0;
}
