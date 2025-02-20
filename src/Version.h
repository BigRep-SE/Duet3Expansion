/*
 * Version.h
 *
 *  Created on: 1 Sep 2019
 *      Author: David
 */

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#ifndef VERSION
#ifndef VERSIONTAG
#define VERSION		"3.6.LOC"
#else
# define VERSION_STR(x) #x
# define VERSION_TO_STR(x) VERSION_STR(x)
# define VERSION VERSION_TO_STR(VERSIONTAG)
#endif
#endif

#include <General/IsoDate.h>

#if 0
// Use this for official releases
# define TIME_SUFFIX
#else
// Use this for internal builds
# define TIME_SUFFIX		" " __TIME__
#endif

#endif /* SRC_VERSION_H_ */
