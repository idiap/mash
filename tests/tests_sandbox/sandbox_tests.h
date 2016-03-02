#ifndef _TESTS_H_
#define _TESTS_H_

#define CHECK(value) \
    do \
    { \
        try { \
            if (!(value)) { \
                cout << "Failure at line " << __LINE__ << ": " << #value << endl; \
                setWardenContext(0); \
                _exit(1); \
            } \
        } \
        catch (...) { \
            cout << "Unhandled exception in CHECK(#value)" << endl; \
            setWardenContext(0); \
            _exit(1); \
        } \
    } while (0)


#define CHECK_EQUAL(expected, actual) \
    do \
    { \
        try { \
            if ((expected) != (actual)) { \
                cout << "Failure at line " << __LINE__ << ": expected " << expected << ", got " << actual << endl; \
                setWardenContext(0); \
                _exit(1); \
            } \
        } \
        catch (...) { \
            cout << "Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")" << endl; \
            setWardenContext(0); \
            _exit(1); \
        } \
    } while (0)
    
    
#endif
