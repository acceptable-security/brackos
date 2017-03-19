unsigned int ipow(unsigned int base, unsigned int exponent);

#if defined(__i386__)
    #define log2(X) (31 - __builtin_clz((X) | 1))
#else
    #if defined(__x86_64__)
        #define log2(X) (63 - __builtin_clzll((X) | 1))
    #endif
#endif

#define min(a, b) ((a) > (b) ? (b) : (a))
#define max(a, b) ((a) < (b) ? (b) : (a))
