#include <stdio.h>

#include "smart_pointer.h"

# if SHARED_PTR_USE_MEMORY
#  include <memory>
#  define plm_shared_ptr std::shared_ptr
# elif TR1_SHARED_PTR_USE_TR1_MEMORY
#  include <tr1/memory>
#  define plm_shared_ptr std::tr1::shared_ptr
# elif TR1_SHARED_PTR_USE_MEMORY
#  include <memory>
#  define plm_shared_ptr std::tr1::shared_ptr
# endif

#include "dlib/smart_pointers.h"

class A {
public:
    int *val;
public:
    A () {
        val = new int;
        *val = 3;
    }
    ~A () {
        delete val;
    }
private:
    A (const A&);
};

void
foo_1 (dlib::shared_ptr<A> a)
{
    (*a->val) ++;
    printf ("A %d\n", *(a->val));
}

void
test_1 ()
{
    dlib::shared_ptr<A> a (new A);
    for (int i = 0; i < 10; i++) {
        foo_1 (a);
    }
}

class B {
public:
    /* Smart pointer support */
//    typedef B Self;
//    typedef plm_shared_ptr<Self> Pointer;
    SMART_POINTER_SUPPORT (B);
public:
    int *val;
public:
    B () {
        val = new int;
        *val = 3;
    }
    ~B () {
        delete val;
    }
//public:
//    static B::Pointer New () {
//        return B::Pointer (new B);
//    }
private:
    B (const B&);
};

void
foo_2 (B::Pointer b)
{
    (*b->val) ++;
    printf ("B %d\n", *(b->val));
}

void
test_2 ()
{
    B::Pointer b = B::New (new B);
    for (int i = 0; i < 10; i++) {
        foo_2 (b);
    }
#if defined (commentout)
    /* This doesn't compile */
    b = 0;
#endif
}

class C {
public:
    typedef plm_shared_ptr<C> Pointer;
#if defined (commentout)
    template<class U1, class U2, class U3,
             class U4, class U5, class U6
             > static C::Pointer New (
                 U1 u1, U2 u2, U3 u3,
                 U4 u4, U5 u5, U6 u6
             ) {
        return C::Pointer (new C (u1, u2, u3, u4, u5, u6));
    }
#endif
    template<class U1, class U2, class U3,
             class U4, class U5, class U6
             > static C::Pointer New (
                 U1& u1, U2& u2, U3& u3,
                 U4& u4, U5& u5, U6& u6
             ) {
        return C::Pointer (new C (u1, u2, u3, u4, u5, u6));
    }
public:
    int *val;
public:
    C (const A& a, B& b, A& c, B& d, B& e, A& f) {
        val = new int;
        *val = 3;
    }
    ~C () {
        delete val;
    }
};

void
test_3 ()
{
    A a;
    B b;
    C::Pointer c = C::New (a, b, a, b, b, a);
}

int 
main (int argc, char* argv[])
{

    test_1();
    test_2();
    return 0;
}
