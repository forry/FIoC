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

int main(int argc, char* argv[])
{
   
   fioc::Builder<std::map /*,std::allocator<std::pair<std::string, std::function<void*()> > >*/ > builder;
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
     

   builder.registerType<B, int>();
   unique_ptr<B> b((builder.resolve<B>(32)));
   if(b)
      cout << "B " << b->get() << endl;
   else cout << "not b" << endl;

   builder.registerType<NoDefaultCtor, int, int>();
   unique_ptr<NoDefaultCtor> n(builder.resolve<NoDefaultCtor>(3,7));
   cout << "NoDef " << n->get() << endl;

   builder.registerType<NoDefaultCtor, int, int>().as<NoDefaultCtorSub>();
   unique_ptr<NoDefaultCtor> n2(builder.resolve<NoDefaultCtor>(3, 4));
   cout << "NoDef " << n2->get() << endl;

   //Compile-time error - which is good since not only A is not a subclass of NoDefaultCtor but it doesn't have appropriate ctor signature
   /*
   builder.registerType<NoDefaultCtor, int, int>().as<A>();
   unique_ptr<NoDefaultCtor> n3(builder.resolve<NoDefaultCtor>(3, 4));
   cout << "NoDef " << n3->get() << endl;
   */

   // Another good compile-time error due to FalseInheritance is not a subclass of NoDefaultCtor
   /*
   builder.registerType<NoDefaultCtor, int, int>().as<FalseInheritance>();
   unique_ptr<NoDefaultCtor> n4(builder.resolve<NoDefaultCtor>(3, 4));
   cout << "NoDef " << n4->get() << endl;
   */

   return 0;
}
