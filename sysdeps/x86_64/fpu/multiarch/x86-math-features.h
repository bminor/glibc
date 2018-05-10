#ifndef X86_MATH_FEATURES_H
#define X86_MATH_FEATURES_H

enum
  {
    x86_math_feature_initialized = 1 << 0,
    x86_math_feature_avx = 1 << 1,
    x86_math_feature_avx2 = 1 << 2,
    x86_math_feature_fma = 1 << 3,
    x86_math_feature_fma4 = 1 << 4,
    x86_math_feature_sse41 = 1 << 5,
  };

/* Return a combination of flags x86_math_feature_* above.  */
unsigned int __x86_math_features (void)
  __attribute__ ((const)) attribute_hidden;

#endif /* X86_MATH_FEATURES_H */
