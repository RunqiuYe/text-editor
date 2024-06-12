#include <assert.h>

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef REQUIRES
#undef REQUIRES
#endif

#ifdef ENSURES
#undef ENSURES
#endif

#ifdef IF_DEBUG
#undef IF_DEBUG
#endif

#ifdef DEBUG

#define ASSERT(COND) assert(COND)
#define REQUIRES(COND) assert(COND)
#define ENSURES(COND) assert(COND)
#define IF_DEBUG(CMD) {CMD;}

#else

#define ASSERT(COND) ((void)0)
#define REQUIRES(COND) ((void)0)
#define ENSURES(COND) ((void)0)
#define IF_DEBUG(CMD) ((void)0)

#endif
