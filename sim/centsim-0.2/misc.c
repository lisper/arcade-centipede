#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void fatal_error (int n, char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  exit (n);
}


int my_stricmp (char *s, char *t)
{
  for ( ; tolower (*s) == tolower (*t); s++, t++)
    if (*s == '\0')
      return 0;
  return *s - *t;
}

int my_stricmpn (char *s, char *t, int n)
{
  int i;
  for (i = 0; i < n; i++)
    if (tolower (*s) == tolower (*t))
      s++, t++;
    else
      return (*s - *t);

  return (0);
}

