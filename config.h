/*
cpu_logger configuration
------------------------
available features:
    FEATURE_LOAD        measures the cpu load
    FEATURE_STEAL       measures the cpu steal (inside of a vm)
    FEATURE_MEMORY      measures the used memory (swap is ignored, and linux file cache as well)
*/

#define MEASURE_GUEST_TIME // for KVM, just 16 bytes more (runtime) memory required, enabled by default

#define FEATURE_LOAD
#define FEATURE_STEAL
#define FEATURE_MEMORY
