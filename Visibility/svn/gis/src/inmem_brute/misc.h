
/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * Jeremy Fishman's Lexer program -- misc.h
 *
 *  Some basic defines and header includes that my be used in any of the files
 *  in the program.  If this looks similar to
 *  http://www.fastcgi.com/devkit/include/fcgimisc.h, that because I was
 *  looking around for a standard header file that defines these things, and
 *  couldn't seem to find it, except for this one from FastCGI.  I've coppied
 *  some of the defines from there, with my own ASSERT macro, and with an extra
 *  include or two.
 **/

#ifndef MISC_H
#define MISC_H

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef false
#define false FALSE
#endif

#ifndef TRUE
#define TRUE  (1)
#endif

#ifndef true
#define true TRUE
#endif

#ifndef NDEBUG
#  define ASSERT(x,s) { \
  if (!(x)) { \
    printf(s "\nerrno = %d\n", errno); \
    assert(false); \
  } \
}
#  define DEBUG(x) (x);
#else
#  define ASSERT(x,s) (x);
#  define DEBUG(x) (0);
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef NDEBUG
#  define debug_switch(v, stmt) { if (v) {stmt;} }
#else
#  define debug_switch(v, stmt) { (stmt); }
#endif


#endif /* MISC_H */
