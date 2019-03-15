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
      return x + y;
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

int main(int argc, char* argv[])
{
   
   fioc::Registry<std::map /*,std::allocator<std::pair<std::string, std::function<void*()> > >*/ > builder;
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
