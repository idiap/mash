#ifndef _TESTS_H_
#define _TESTS_H_

#define CHECK(value) \
    do \
    { \
        try { \
            if (!(value)) { \
                cout << "Failure at line " << __LINE__ << ": " << #value << endl; \
                return -1; \
            } \
        } \
        catch (...) { \
            cout << "Unhandled exception in CHECK(#value)" << endl; \
            return -1; \
        } \
    } while (0)


#define CHECK_EQUAL(expected, actual) \
    do \
    { \
        try { \
            if ((expected) != (actual)) { \
                cout << "Failure at line " << __LINE__ << ": expected " << expected << ", got " << actual << endl; \
                return -1; \
            } \
        } \
        catch (...) { \
            cout << "Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")" << endl; \
            return -1; \
        } \
    } while (0)


#define CHECK_EXIT(value) \
    do \
    { \
        try { \
            if (!(value)) { \
                cout << "Failure at line " << __LINE__ << ": " << #value << endl; \
                _exit(1); \
            } \
        } \
        catch (...) { \
            cout << "Unhandled exception in CHECK(#value)" << endl; \
            _exit(1); \
        } \
    } while (0)


#define CHECK_EQUAL_EXIT(expected, actual) \
    do \
    { \
        try { \
            if ((expected) != (actual)) { \
                cout << "Failure at line " << __LINE__ << ": expected " << expected << ", got " << actual << endl; \
                _exit(1); \
            } \
        } \
        catch (...) { \
            cout << "Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")" << endl; \
            _exit(1); \
        } \
    } while (0)
    
    
#endif
