#ifndef _BRACKOS_CONF
#define _BRACKOS_CONF

// Enable support for ACPI
#define BRACKOS_CONF_ACPI

// Enable support for symmetric multiprocessing
#define BRACKOS_CONF_SMP

// Enable support for the SLAB allocator
#define BRACKOS_CONF_SLAB

// Enable support for the SLUB allocator
// #define BRACKOS_CONF_SLUB


// There are verifications of the configuration and shouldn't be modified unless
// you are adding or removing certain configuration options.

#if !defined(BRACKOS_CONF_SLAB) && !defined(BRACKOS_CONF_SLUB)
#error "You must enable at least one memory allocator"
#endif

#if defined(BRACKOS_CONF_SLUB) && !defined(BRACKOS_CONF_SMP)
#error "You must use SMP when enabling SLUB"
#endif

#if defined(BRACKOS_CONF_SLAB) && defined(BRACKOS_CONF_SLUB)
#error "You can only define one memory allocator at at time"
#endif

#if defined(BRACKOS_CONF_SMP) && !defined(BRACKOS_CONF_ACPI)
#error "If you want to enable SMP you must enable ACPI"
#endif

#endif
