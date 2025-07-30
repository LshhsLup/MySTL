#ifndef __MYSTL_UTILITY_H__
#define __MYSTL_UTILITY_H__

#include <utility>
#include <type_traits>
std::pair<int,int> p1;

namespace mystl {

    //===========================================================
    //======================    pair    =========================
    //===========================================================
    template<class T1, class T2>
    struct pair {
        // member types
        using first_type = T1;
        using second_type = T2;

        // member object
        T1 first;
        T2 second;

        // constructors
        template<U1 = T1, U2 = T2, 
                typename = std::enable_if_t<std::is_default_constructible<U1>::value &&
                std::is_default_constructible<U2>::value>::type>
        constexpr pair() : first(), second() {}
    };
}



#endif