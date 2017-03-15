unsigned int ipow(unsigned int base, unsigned int exponent) {
    unsigned int result = 1;

    while ( exponent > 0 ) {
        if ( exponent & 1 ) {
            result *= base;
        }

        exponent >>= 1;
        base *= base;
    }

    return result;
}
