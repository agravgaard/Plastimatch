#include <stdio.h>
#include <map>
#include <string>

/* Simple test */
typedef void (*B) (int);

void c (int x) {
    printf ("x = %d\n", x);
}

template <B b> void a (int x) {
    b(x);
}

/* More complex */
#if defined (commentout)
/* This is illegal, typedef's can't be templated */
typedef template <class F> void (*E) (F);
void g<int> (int x) {
    printf ("x = %d\n", x);
}
#endif

template <typename F>
class G
{
public:
    static void g (F x) {
        printf ("x = %d\n", x);
    }
};


template < class G > void d (int x) {
    G::g (x);
}

/* Even more complex */
template < class J, template<class J> class I> void h (J x) {
    I<J>::g (x);
}

template < template<class J> class I, class J > void k (J x) {
    I<J>::g (x);
}

#if defined (commentout)
/* This doesn't seem to work */
template < template<class J> class I > void l (J x) {
    I<J>::g (x);
}
#endif

/* This is an example inheriting from std::map */
/* Cf. http://stackoverflow.com/questions/10477839/c-inheriting-from-stdmap 
   (The answer from Emilio Garavaglia, not the others) */
template <class L> class M : public std::map<std::string,L>
{
public:
    typename std::map<std::string,L>::iterator get_default (std::string s) {
        return this->begin();
    }
};

int main 
(
    int argc,
    char* argv[]
)
{
    int x = 1;
    a<c>(x);
    G<int>::g (x);
    d< G<int> >(x);
    h< int, G >(x);
    k< G, int >(x);

    M<int> m;
    M<int>::iterator mit = m.get_default("FOO");
}
