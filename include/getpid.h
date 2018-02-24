#ifndef _GETPID_H_
#define _GETPID_H_

#ifdef _MSC_VER
# include <process.h>
# define getpid _getpid
#else /* _MSC_VER */
# include <unistd.h>
#endif /* _MSC_VER */

#endif /* _GETPID_H_ */
