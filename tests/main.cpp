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


int main(int argc, char* argv[])
{
   
   fioc::Builder<std::map, std::less<std::string>/*,std::allocator<std::pair<std::string, std::function<void*()> > >*/ > builder;
   builder.registerType<A>();
   unique_ptr<A> a(builder.resolve<A>());
   if(a)
      cout << "A " << a->get() <<endl;
   else
      cout << "A not found" << endl;

   //builder.registerTypeAs<A,C>();
   /*builder.registerType<A>().as<C>();
   a.reset(builder.resolve<A>());
   if(a)
      cout << "A " << a->get() << endl;
   else
      cout << "A not found" << endl;
      */

   builder.registerType<B, int>();
   unique_ptr<B> b((builder.resolve<B>(32)));
   if(b)
      cout << "B " << b->get() << endl;
   else cout << "not b" << endl;

   builder.registerType<NoDefaultCtor, int, int>();
   unique_ptr<NoDefaultCtor> n(builder.resolve<NoDefaultCtor>(3,7));
   cout << "NoDef " << n->get() << endl;

   return 0;
}
