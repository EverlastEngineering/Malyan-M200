#ifndef _MALLOCSTATS_H_
#define _MALLOCSTATS_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------- Declarations from dlmalloc.c ------------------------*/
#define MALLINFO_FIELD_TYPE size_t

#define STRUCT_MALLINFO_DECLARED 1
struct mallinfo {
  MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
  MALLINFO_FIELD_TYPE smblks;   /* always 0 */
  MALLINFO_FIELD_TYPE hblks;    /* always 0 */
  MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
  MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
  MALLINFO_FIELD_TYPE fordblks; /* total free space */
  MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
};
/* -----------------------------------------------------------------------*/

struct mallinfo __iar_dlmallinfo(void);
void __iar_dlmalloc_stats(void);

#ifdef __cplusplus
}
#endif

#endif
