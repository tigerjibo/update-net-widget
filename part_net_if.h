/* net/if.h -- declarations for inquiring about network interfaces
   Copyright (C) 1997-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

// Part of net/if.h
// Copied here and used instead of net/if.h
// because that conflicts with linux/if.h

#ifndef _NET_IF_PART_H
#define _NET_IF_PART_H	1


/* Length of interface name.  */
#define IF_NAMESIZE	16

struct if_nameindex
  {
    unsigned int if_index;	/* 1, 2, ... */
    char *if_name;		/* null terminated name: "eth0", ... */
  };


__BEGIN_DECLS

/* Convert an interface name to an index, and vice versa.  */
extern unsigned int if_nametoindex (const char *__ifname) __THROW;
extern char *if_indextoname (unsigned int __ifindex, char *__ifname) __THROW;

/* Return a list of all interfaces and their indices.  */
extern struct if_nameindex *if_nameindex (void) __THROW;

/* Free the data returned from if_nameindex.  */
extern void if_freenameindex (struct if_nameindex *__ptr) __THROW;

__END_DECLS

#endif /* net/if.h */
