#ifndef __MYSTL_UTILITY_H__
#define __MYSTL_UTILITY_H__

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
        
    };
}



#endif