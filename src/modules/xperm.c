/*

  xperm.c

  (C) Jose M. Martin-Garcia 2003-2006.
      See http://metric.iem.csic.es/Martin-Garcia/xAct/
      Modifications by Kasper Peeters <kasper.peeters@aei.mpg.de>

  This program is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
 *  These are a collection of C-functions that find Strong Generating
 *  Sets, Coset representatives and Double-coset representatives.
 *  The algorithms are based on Butler and Portugal et al.
 *
 *  xperm can be used alone or from the Mathematica package xPerm.
 *
 *  20 June - 5 July 2003.
 *   3 September 2004. Eliminated MAX variables.
 *   9 November 2005. Corrected treatment of SGS of group D.
 *   6 May 2006. All arrays declared dynamically to avoid stack limits.
 *     Thanks to Kasper Peeters for spotting and correcting this.
 *
 *  Main ideas:
 *      - Permutations are represented using Images notation.
 *      - Generating sets with m n-permutations are stored as lists of
 *	  length m*n.
 *	- #define VERBOSE_* to turn on log output.                PPC
 *
 *  Comments:
 *	- Permutations are assumed to have degree n>0.
 *	- Lists can have length 0 or positive.
 *
 *  This is not ANSI-C. There are some gcc extensions:
 *	- ISO C forbids nested functions
 *	- ISO C89 forbids mixed declarations and code
 *	- ISO C90 does not support `long long'
 *	- ISO C90 forbids variable-size arrays
 *
 */

/* KP: added to make this compile */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xperm.h"
#define USE_MEMCPY
/* KP: end */

/*********************************************************************
 *                             PROTOTYPES                            *
 *********************************************************************/

/* Output */
void print_perm(int *p, int n, int nl);
void print_array_perm(int *perms, int m, int n, int nl);
void print_list(int *list, int n, int nl);
void print_array(int *array, int m, int n, int nl);

/* Lists */
int equal_list(int *list1, int *list2, int n);
void copy_list(int *list1, int *list2, int n);
int position(int i, int *list, int n);
int position_list(int *matrix, int m, int *row, int n);
void zeros(int *list, int n);
void range(int *list, int n);
void complement(int *all, int al, int *part, int pl, int n,
		int *com, int *cl);
void sort(int *list, int *slist, int l);
void sortB(int *list, int *slist, int l, int *B, int Bl);
int minim(int *list, int n);
int maxim(int *list, int n);
void intersection(int *list1, int l1, int *list2, int l2, int *list,
		int *l);

/* Permutations */
int isid(int *list, int n );
void product(int *p1, int *p2, int *p, int n);
void inverse(int *p, int *ip, int n);
int onpoints(int point, int *p, int n);
void stable_points(int *p, int n, int *list, int *m);
int first_nonstable_point(int *p, int n);
void nonstable_points(int *list1, int l1, int *GS, int m, int n,
	int *list2, int *l2);
void stabilizer(int *points, int k, int *GS, int m, int n, int *subGS, 
	int *mm);
void one_orbit(int point, int *GS, int m, int n, int *orbit, int *ol);
void all_orbits(int *GS, int m, int n, int *orbits);
void schreier_vector(int point, int *GS, int m, int n, int *nu, int *w);
void trace_schreier(int point, int *nu, int *w, int *perm, int n);

/* Algorithms */
long long int order_of_group(int *base, int bl, int *GS, int m, int n);
int perm_member(int *p, int *base, int bl, int *GS, int m, int n);
void schreier_sims_step(int *base, int bl, int *GS, int m, int n, int i,
	int *T, int mm, int *newbase, int *nbl, int **newGS, int *nm,
	int *num);
void schreier_sims(int *base, int bl, int *GS, int m, int n,
	int *newbase, int *nbl, int **newGS, int *nm, int *num);
void coset_rep(int *p, int n, int *base, int bl, int *GS, int *m,
	int *freeps, int fl, int *cr);
void double_coset_rep(int *g, int n, int *base, int bl, int *GS, int m,
	int *dummypairs, int dpl, int *dummysetlabels, int sym, int *dcr);
/* void canonical_perm(int *perm,
	int SGSQ, int *base, int bl, int *GS, int m, int n,
	int *freeps, int fl, int *dummyps, int dl, int ob, int metricQ,
	int *cperm); */ // KP


/*********************************************************************
 *                         PRINTING FUNCTIONS                        *
 *********************************************************************/

/**********************************************************************/

/* print_perm. JMM, 22 June 2003
 *
 * This function prints a permutation p of degree n. If nl=1(0) adds
 * (does not) a newline. */

void print_perm(int *p, int n, int nl) {
        int i;
        if (isid(p,n)) printf("id");
        else {
                printf("(");
                printf("%d", p[0]);          /* No comma */
                for (i=1; i<n; i++) {
                        printf(",%d", p[i]); /* Comma separated */
                }
                printf(")");
        }
        if (nl) printf("\n");
}

/**********************************************************************/

/* print_array_perm. JMM, 22 June 2003
 *
 * This function prints an array of m n-permutations.
 * If nl=1 (0) adds (does not) a newline after each row.
 * There are no commas between permutations. */

void print_array_perm(int *perms, int m, int n, int nl) {
        int j;
        printf("{");
        if (nl) printf("\n");
        for(j=0; j<m; j++) {
                printf(" ");
                print_perm(perms+j*n, n, nl);
        }
        if (nl) printf("}\n");
        else printf(" }\n");
}

/**********************************************************************/

/* print_list. JMM, 22 June 2003
 *
 * This function prints a list of length n in curly brackets.
 * If nl=1 (0) it adds (does not) a newline. */

void print_list(int *list, int n, int nl) {
        int i;
        printf("{");
        if (n>0) printf("%d", list[0]);            /* No comma */
        for (i=1; i<n; i++) printf(",%d", list[i]);/* Comma separated */
        printf("}");
        if (nl) printf("\n");
}

/**********************************************************************/

/* print_array. JMM, 22 June 2003
 *
 * This function prints an array of dimensions m x n in curly brackets.
 * If nl=1 (0) adds (does not) a newline after each row.
 * There are no commas between lists. */

void print_array(int *array, int m, int n, int nl) {
        int j;
        printf("{");
        if(nl) printf("\n");
        for(j=0; j<m; j++) {
                printf(" ");
                print_list(array+j*n, n, nl);
        }
        if (!nl) printf(" ");
        printf("}\n");
}

/**********************************************************************/


/*********************************************************************
 *                          GENERIC FUNCTIONS                        *
 *********************************************************************/

/**********************************************************************/

/* equal_list. JMM, 27 June 2003
 *
 * Checks that list1 and list2 (both with length n) are equal.
 * If n=0 the result is 1.
 */

/* Old code
int equal_list(int *list1, int *list2, int n) {
	while(n--) { * Run from n-1 to 0 *
		if(*(list1+n) != *(list2+n)) return(0); * different *
	}
	return(1); * equal *
}
*/

/* KP, 7 May 2006 */
int equal_list(int *list1, int *list2, int n) {
	if (n==0) return 1;
	if (memcmp(list1, list2, n*sizeof(int))==0) return 1;
	else return 0;
}

/**********************************************************************/

/* copy_list. JMM, 27 June 2003
 *
 * Copies first n elements of list1 onto list2. If n=0 nothing is done.
 */

/* Old code
void copy_list(int *list1, int *list2, int n) {
	while(n--) *(list2+n) = *(list1+n);
}
*/

/* KP, 7 May 2006 */
void copy_list(int *list1, int *list2, int n) {
	if (n==0) return;
//	memcpy(list2, list1, n*sizeof(int));
	memmove(list2, list1, n*sizeof(int));
}

/**********************************************************************/

/* position. JMM, 20 June 2003
 *
 * This function finds the position of the integer i in list (length n),
 * counting from 1 to n, or else gives 0 if i is not in list. Note that
 * it starts from the end so that if i appears several times in list it
 * will only find the last one. If n=0 the result is 0.
 *
 * More than 60% of time in the Schreier-Sims algorithm is spent here!
 *
 * Note: it is not possible to use  memchr  because it only works with
 * characters (integers up to 255).
 */

int position(int i, int *list, int n) {
	while(n--) {
		if (*(list+n) == i) return(n+1);
	}
	return(0);
}

/**********************************************************************/

/* position_list. JMM, 28 June 2003
 *
 * This function gives the position of list row (length n) in the
 * matrix of dimensions m, n. If row is not present in matrix, 0
 * is returned.
 */

int position_list(int *matrix, int m, int *row, int n) {
	while(m--) {
		if(equal_list(matrix+m*n, row, n)) return(m+1);
	}
	return(0);
}

/**********************************************************************/

/* zeros, range. JMM, 22 June 2003
 *
 * These functions generate lists of n zeros or the range 1...n,
 * respectively, storing the result in list.
 */

/* Old code
void zeros(int *list, int n) {
	while(n--) *(list+n) = 0;
}
*/

/* KP, 7 May 2006 */
void zeros(int *list, int n) {
	if (n==0) return;
	memset(list, 0, n*sizeof(int));
}

void range(int *list, int n) {
        while(n--) *(list+n) = n+1;
}

/**********************************************************************/

/* complement. JMM, 28 June 2003
 *
 * This function gives all sublists of length n in list all (length al)
 * that are not members of list part (length pl), storing the result
 * in list com (length cl).
 * We assume that enough space (typically al n-lists) has been already
 * allocated for com.
 */

void complement(int *all, int al, int *part, int pl, int n,
		int *com, int *cl) {

	int i;

	*cl = 0;
	for(i=0; i<al; i++) {
		if (!position_list(part, pl, all+i*n, n)) {
			copy_list(all+i*n, com+(*cl)*n, n);
			(*cl)++;
		}
	}

}

/**********************************************************************/

/* sort. JMM, 30 June 2003
 * 
 * Sorts list (length l) storing the result in slist (length l, too).
 */

void sort(int *list, int *slist, int l) {

	int i,j;
	int tmp, mini; /* mini is a position, not an element of list */

#ifdef VERBOSE_LISTS						/*PPC*/
	printf("sort: Sorting list "); print_list(list, l, 1);	/*PPC*/
#endif								/*PPC*/
	copy_list(list, slist, l);
	for (i=0; i<l-1; i++) {
	    /* Assume points at positions 0...i-1 are already sorted */
	    /* Compare point at position i with points to the right */
	    mini = i;
	    for(j=i+1; j<l; j++) {
		if ( slist[j] < slist[mini] ) mini = j;
	    }
	    /* Swap points i and mini */
	    tmp = slist[i];
	    slist[i] = slist[mini];
	    slist[mini] = tmp;
	}
#ifdef VERBOSE_LISTS						/*PPC*/
	printf("sort: with result "); print_list(slist, l, 1);	/*PPC*/
#endif								/*PPC*/
}

/* sortB. JMM, 30 June 2003
 * 
 * Sorts list (length l) according to the order given by B (length Bl),
 * storing the result in slist (length l, too). Elements not in B
 * are pushed to the end, and sorted using sort.
 */

void sortB(int *list, int *slist, int l, int *B, int Bl) {

	int sl;
	int *tmp=  (int*)malloc(l*sizeof(int)), tmpl;
	int *stmp= (int*)malloc(l*sizeof(int));

#ifdef VERBOSE_LISTS						/*PPC*/
	printf("sortB: Sorting list "); print_list(list, l, 1);	/*PPC*/
	printf("sortB: using base "); print_list(B, Bl, 1);	/*PPC*/
#endif								/*PPC*/
	/* Elements of list in B, keeping order of B, moved to slist */
	intersection(B, Bl, list, l, slist, &sl);
	/* Other elements of list are appended to slist */
        complement(list, l, B, Bl, 1, tmp, &tmpl);
	/* Check that tmpl+sl==l */
	if(tmpl+sl != l) printf("Error in sortB\n");
	/* Sort the latter integers using sort */
	sort(tmp, stmp, tmpl);
	copy_list(stmp, slist+sl, tmpl);
#ifdef VERBOSE_LISTS						/*PPC*/
	printf("sortB: with result "); print_list(slist, l, 1);	/*PPC*/
#endif								/*PPC*/
	/* Free allocated memory */
	free(tmp);
	free(stmp);

}

/**********************************************************************/

/* minim, maxim. JMM, 2 July 2003
 *
 * These functions find the minimum and maximum elements of a given
 * list of length n, using normal integer ordering. Names min, max are
 * reserved.
 */

int minim(int *list, int n) {

	int m=list[n-1];

	while(n--) {
		if (list[n]<m) m=list[n];
	}
	return(m);
	
}

int maxim(int *list, int n) {

	int m=list[n-1];

	while(n--) {
		if (list[n]>m) m=list[n];
	}
	return(m);
	
}

/**********************************************************************/

/* intersection. JMM, 3 July 2003
 * 
 * This function returns in list (length l) the intersection of the
 * elements of lists list1 (length l1) and list2 (length l2). Note that
 * we return unique (that is, non-repeated) elements. Returned elements
 * keep the original order in list1.
 * We assume that enough space (typical l1 or l2 integers) has been
 * already allocated for list.
 */

void intersection(int *list1, int l1, int *list2, int l2, int *list,
		int *l) {

	int i, j, a;

	*l = 0;
	for(i=0; i<l1; i++) { /* Range over elements of list1 */
		a = list1[i];
		for(j=0; j<l2; j++) { /* Range over elements of list2 */
                        /* a is in list2 and not yet in list, append */
			if ((list2[j] == a) && (!position(a, list, *l)))
 				list[(*l)++] = a;
		}
	}
	
}

/**********************************************************************/


/*********************************************************************
 *                        PERMUTATIONS FUNCTIONS                     *
 *********************************************************************/

/**********************************************************************/

/* isid. JMM, 20 June 2003
 *
 * This function detects whether the n-permutation p is the identity
 * (returning 1) or not (returning 0). */

int isid(int *p, int n ) {

        while(n--) {
		if (*(p+n)!=n+1) return(0); /* Not the identity */
	}
        return(1); /* Identity */

}

/**********************************************************************/

/* product. JMM, 20 June 2003
 *
 * Product of n-permutations p1 and p2 (from left to right) storing the
 * result in the n-permutation p. Note that the points of the
 * permutations go from 1 to n, while we want to add from 0 to n-1 to
 * the pointer p2. That's why we need to add -1.
 * This function is highly efficient, but rather criptic. */

void product(int *p1, int *p2, int *p, int n) {

	while(n--) *(p++) = *(p2-1+*(p1++));

}

/**********************************************************************/

/* inverse. JMM, 20 June 2003
 *
 * Inverse of the n-permutation p, storing the result in the
 * n-permutation ip. Again we need to add -1 to the pointer ip.
 * Conversely, note that we need to add +1 to n because n ranges from
 * 0 to n-1 in the while loop. */

void inverse(int *p, int *ip, int n) {

        while(n--) *(ip-1+*(p+n)) = n+1 ;

}

/**********************************************************************/

/* onpoints. JMM, 20 June 2003
 *
 * Image of point under the n-permutation p. If point is larger than
 * the degree n we return point. */

int onpoints(int point, int *p, int n) {
	
        if (point <= n) return(*(p-1+point));
        else return(point);

}

/**********************************************************************/

/* stable_points. JMM, 20 June 2003
 *
 * This function finds the points which remain stable under the
 * n-permutation p, storing the result in list of length m (<=n).
 * With n=0 we get an empty list (m=0).
 * We assume that enough space (typically n integers) has been already
 * allocated for list.
 */

void stable_points(int *p, int n, int *list, int *m) {

        int i;

	*m = 0;
        for(i=1; i<=n; i++) {
                if ( onpoints(i, p, n) == i ) list[(*m)++] = i;
        }

}

/**********************************************************************/

/* first_nonstable_point. JMM, 30 June 2003
 *
 * This function gives the smallest moved point by the permutation p.
 * If no point is moved the result is 0. If n=0 the result is 0.
 * This is a restricted form of stable_points.
 */

int first_nonstable_point(int *p, int n) {

	int i;

	for(i=1; i<=n; i++) {
		if (onpoints(i, p, n) != i) return(i);
	}
	return(0);
}

/**********************************************************************/

/* nonstable_points. JMM, 30 June 2003
 *
 * This function gives a set of points (list2, length l2) such that
 * none of the permutations in GS fixes them all. The first l1 points
 * of list2 will be those of list1, even if they are all stable under
 * GS. If m=0 the resulting list2 is just list1.
 * We assume that enough space (typically n integers) has been already
 * allocated for list2.
 */

void nonstable_points(int *list1, int l1, int *GS, int m, int n,
	int *list2, int *l2) {

	int i, j;

	copy_list(list1, list2, l1); *l2 = l1; /* Initialize list2 */
	for(j=0; j<m; j++) { /* Range over permutations of GS */
		int stable=1;
		for(i=0; i<*l2; i++) {
			if (onpoints(list2[i], GS+j*n, n) != list2[i]) {
				stable=0;
				break;
			}
		}
                /* If all points already in list2 are stable under the
                   permutation, append the smallest nonstable point */
		if (stable) {
			list2[*l2] = first_nonstable_point(GS+j*n, n);
			(*l2)++;
		}
	}
}

/**********************************************************************/

/* stabilizer. JMM, 22 June 2003
 *
 * This function finds the permutations that stabilize the set of k
 * points among the GeneratingSet GS with m n-permutations. With m=0
 * we return an empty subset (mm=0).
 * We assume that enough space (typically m*n integers) has been already
 * allocated for subGS.
 */

void stabilizer(int *points, int k, int *GS, int m, int n,
                int *subGS, int *mm) {

        int i, j;
        int stable;

        *mm=0;
        for(j=0; j<m; j++) {
                stable=1;
                for(i=0; i<k; i++) {
                        if (onpoints(points[i],GS+n*j,n) != points[i]) {
                                stable = 0;
                                break;
                        }
                }
                if (stable) {
			copy_list(GS+n*j, subGS+(*mm)*n, n);
                        ++(*mm);
                }
        }

}

/**********************************************************************/

/* one_orbit. JMM, 22 June 2003
 *
 * Orbit of a given point under a GeneratingSet GS with m 
 * n-permutations. The result is stored in orbit (length ol).
 * With m=0 the orbit is just {point}.
 * We assume that enough space (typically n integers) has been already
 * allocated for orbit.
 */

void one_orbit(int point, int *GS, int m, int n, int *orbit, int *ol) {

	int np=0;  /* Index of current element in the orbit */
	int gamma; /* Current element in the orbit */
	int mp;    /* Index of current permutation in GS */
	int newgamma;

	orbit[0] = point;
	*ol = 1;
	while(np < *ol) {
		gamma = orbit[np];
		for(mp=0; mp<m; mp++) {
			newgamma = onpoints(gamma, GS+mp*n, n);
			if (!position(newgamma, orbit, *ol))
				orbit[(*ol)++] = newgamma;
		}
		np++;
	}

}

/**********************************************************************/

/* all_orbits. JMM, 1 July 2003
 *
 * This function returns all orbits of the generating set GS (with
 * m n-permutations). Note that we use a very special notation: the
 * result is a vector orbits of length n such that all points marked
 * with 1 belong to the same orbit; all points marked with 2 belong to
 * another orbit, and so on. With m=0 we return the list {1,2,...,n}.
 */

void all_orbits(int *GS, int m, int n, int *orbits) {

	int i, j;                                   /* Counters */
	int *orbit= (int*)malloc(n*sizeof(int)), ol;/* Computed orbit */
	int or=1;                                   /* Orbit index */

	/* Initialize orbits */
	memset(orbits, 0, n*sizeof(int));

	/* Compute orbits */
	for (i=1; i<=n; i++) { /* Points */
		if(orbits[i-1]==0) { /* New orbit condition */
			/* Compute new orbit */
			one_orbit(i, GS, m, n, orbit, &ol);
			/* Mark points of new orbit with index or */
			for (j=0; j<ol; j++) orbits[orbit[j]-1] = or;
			/* Increment index for next orbit */
			or++;
		}
	}

	/* Free allocated memory */
	free(orbit);

}

/**********************************************************************/

/* one_schreier_orbit. JMM, 22 June 2003
 *
 * Orbit of point under the GeneratingSet GS with m n-permutations.
 * The result is stored in orbit and the vectors nu of permutations and
 * w of backward points. If init=1 both nu and w are reset to 0.
 *
 * Note: the profiler shows that roughly half of the time in this
 * function is spent in the subroutine `position'.
 */

void one_schreier_orbit(int point, int *GS, int m, int n,
		int *orbit, int *ol, int *nu, int *w, int init) {

	int np;    /* Index of current element in the orbit */
	int gamma; /* Current element in the orbit */
	int mp;    /* Index of current permutation in GS */
	int *perm= (int*)malloc(n*sizeof(int));
	int newgamma;

	/* Initialize schreier with zeros if required */
	memset(orbit, 0, n*sizeof(int));
	if (init) {
		memset(nu, 0, n*n*sizeof(int));
		memset(w, 0, n*sizeof(int));
	}
	/* First element of orbit. There is no backward pointer */
	orbit[0] = point;
	*ol = 1;
	/* Other elements of orbit */
	np = 0;
	while(np < *ol) {
		gamma = orbit[np];
		for(mp=0; mp<m; mp++) {
			copy_list(GS+mp*n, perm, n);
			newgamma = onpoints(gamma, perm, n);
			if (position(newgamma, orbit, *ol));
			else {
				/* Append to orbit */
				orbit[(*ol)++] = newgamma;
				/* Perm moving gamma to newgamma */
				copy_list(perm, nu+(newgamma-1)*n, n);
				/* Gamma backward pointer of newgamma */
				*(w+newgamma-1) = gamma;
			}
		}
		np++;
	}
	/* Free allocated memory */
	free(perm);

}

/**********************************************************************/

/* schreier_vector. JMM, 22 June 2003
 *
 * All orbits of a given GeneratingSet GS with m n-permutations, with
 * combined vectors of backward permutations and pointers.
 */

void schreier_vector(int point, int *GS, int m, int n, int *nu, int *w){
	
        int i;    /* Point counter (from 1 to n) */
	int *orbit=      (int*)malloc(n*sizeof(int));
	int *usedpoints= (int*)malloc(n*sizeof(int));

/* KP: init to zero */
	for(i=0; i<n; ++i)
		 usedpoints[i]=0;

        int j=0;
	int ol;
        
	/* First orbit and initialization to zeros */
        one_schreier_orbit(point, GS, m, n, orbit, &ol, nu, w, 1);
	while(ol--) usedpoints[j++] = orbit[ol];
        /* Other orbits */
        for(i=1; i<=n; i++) {
            if (!position(i, usedpoints, n)) {
                  one_schreier_orbit(i, GS, m, n, orbit, &ol, nu, w, 0);
		  while(ol--) usedpoints[j++] = orbit[ol];
	    }
        }
	/* Free allocated memory */
	free(orbit);
	free(usedpoints);

}

/**********************************************************************/

/* trace_schreier. JMM, 22 June 2003
 *
 * This function traces the Schreier vector (orbits, nu, w) with the
 * point given, returning in perm a permutation wich moves the first
 * point of the corresponding orbit to point.
 */

void trace_schreier(int point, int *nu, int *w, int *perm, int n) {
	
        int i;
        int found=0;
	int *newperm= (int*)malloc(n*sizeof(int));
        
	for(i=0; i<n; i++) {
                if (*(w+point-1) == 0) {
                        range(perm, n);
                        found=1;
                        break;
                }
        }
        if (!found) {
                trace_schreier(*(w+point-1), nu, w, newperm, n);
                product(newperm, nu+(point-1)*n, perm, n);
        }
	free(newperm);

}

/**********************************************************************/

/* order_of_group. JMM, 27 June 2003
 *
 * This function calculates the order of the group generated by the SGS
 * given by base (with bl points) and GS (with m n-permutations).
 *
 * JMM, 12 March 2006: changed type to long long int (8 bytes).
 * This allows numbers up to ~ 10^19.
 */

long long int order_of_group(int *base, int bl, int *GS, int m, int n) {

	if (m==0) return(1);
	else {
		int *stab=  (int*)malloc(m*n*sizeof(int)), sl;
		int *orbit= (int*)malloc(  n*sizeof(int)), ol;
		one_orbit(base[0], GS, m, n, orbit, &ol);
		stabilizer(base, 1, GS, m, n, stab, &sl);
		long long int ret=ol* order_of_group(base+1,bl-1,stab,sl,n);
		free(stab);
		free(orbit);
		return ret;
	}
}

/**********************************************************************/

/* perm_member. JMM, 27 June 2003
 *
 * Checks whether permutation p belongs to the group generated by the
 * SGS given by base (length bl) and GS (m n-permutations). If m=0 the
 * result is 1.
 */

int perm_member(int *p, int *base, int bl, int *GS, int m, int n) {

	if (bl==0) return( isid(p, n) );
	else {
		int *pp=    (int*)malloc(  n*sizeof(int));
		int *ip=    (int*)malloc(  n*sizeof(int));
		int *orbit= (int*)malloc(  n*sizeof(int)), ol;
		int *w=     (int*)malloc(  n*sizeof(int));
		int *nu=    (int*)malloc(n*n*sizeof(int));
		int *stab=  (int*)malloc(m*n*sizeof(int)), sl;
		int point, ret;

		one_schreier_orbit(base[0], GS,m,n, orbit,&ol, nu,w, 1);
		point = onpoints(base[0], p, n);
		if (position(point, orbit, ol)) {
			trace_schreier(point, nu, w, pp, n);
			inverse(pp, ip, n);
			product(p, ip, pp, n);
			stabilizer(base, 1, GS, m, n, stab, &sl);
			ret = perm_member(pp, base+1, bl-1, stab,sl,n);
		} else ret = 0;

		free(pp);
		free(ip);
		free(orbit);
		free(w);
		free(nu);
		free(stab);

		return(ret);
	}

}

/**********************************************************************/

/* schreier_sims. JMM, 28-30 June 2003
 *
 * This function computes a Strong Generating Set for the group of
 * permutations generated by GS (with m n-permutations). It is
 * possible to give the first bl (possibly 0) elements of the base,
 * and the code computes all the others. There are possibly (and
 * probably) redundant points in the final base. n=0 is not consistent
 * here.
 * We assume that enough space (at least, and typically, m*n integers)
 * has been already allocated for newGS. We also assume that newGS can
 * be reallocated. That's why we do not send a pointer, but a pointer
 * to that pointer. That is, it needs to be reallocated in a different
 * subroutine, and that cannot be done with a normal pointer!
 */

void schreier_sims(int *base, int bl, int *GS, int m, int n,
	int *newbase, int *nbl, int **newGS, int *nm, int *num) {

#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("*****************************************\n");	/*PPC*/
	printf("******** SCHREIER-SIMS ALGORITHM ********\n");	/*PPC*/
	printf("*****************************************\n\n");/*PPC*/
#endif								/*PPC*/
	/* Note the cycle: base -> newbase -> base2 -> newbase -> ... */

	/* Copy base into newbase, adding more points if needed */
	nonstable_points(base, bl, GS, m, n, newbase, nbl);
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("Original base:"); print_list(base, bl, 1);	/*PPC*/
	printf("New base:"); print_list(newbase, *nbl, 1);	/*PPC*/
#endif								/*PPC*/
	/* Initialize newGS=GS */
	copy_list(GS, *newGS, m*n); *nm = m;
	if (*nbl==0) { /* Problem. Return input sets  */
		copy_list(base, newbase, bl); *nbl = bl;
		return;
	}

	/* Allocate memory for intermediate SGS and stabilizer */
	int i;                                         /* Base index */
	int *base2= (int*)malloc(  n*sizeof(int)), bl2;/* Interm base */
	int *GS2=   (int*)malloc(m*n*sizeof(int)), m2; /* Interm GS */
	int *stab=  (int*)malloc(m*n*sizeof(int)), mm; /* Stabilizer */

	/* Main loop */
	m2 = *nm; /* Initially GS2 will be just newGS */
	for(i=(*nbl); i>0; i--) {
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("\nComputing SGS for H^(%d)\n", i-1);		/*PPC*/
#endif								/*PPC*/
		if (*nm > m2) { /* Reallocate GS2 and stab */
			GS2 =  (int*)realloc(GS2,  (*nm)*n*sizeof(int));
			stab = (int*)realloc(stab, (*nm)*n*sizeof(int));
		}
		/* Copy newbase into base2 */
		copy_list(newbase, base2, *nbl); bl2=*nbl;
		copy_list(*newGS, GS2, (*nm)*n); m2=*nm;
		/* Compute newbase from base2 */
		stabilizer(base2, i-1, GS2, m2, n, stab, &mm);
		schreier_sims_step(base2, bl2, GS2, m2, n, i, 
			stab, mm, newbase, nbl, newGS, nm, num);
	}

	/* Free allocated memory */
	free(base2);
	free(GS2);
	free(stab);

#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("\n*****************************************\n");/*PPC*/
	printf("************ END OF ALGORITHM ***********\n");	/*PPC*/
	printf("*****************************************\n\n");/*PPC*/
#endif								/*PPC*/
}

/* Intermediate function for schreier_sims.
 *
 * Assuming that S^(i-1) is a GenSet for H^(i-1) and that [B, S^(i)]
 * are a StrongGenSet for H^(i), find a StrongGenSet for H^(i-1).
 * T is the set of additional generators in S^(i-1) since the previous
 * call to the procedure with the present value of i. Assume that a
 * StrongGenSet of < S^(i-1) - T > (the previous value of H^(i-1)) is
 * included in B and S. The present value of nu^(i-1) must be an
 * extension of the previous value.
 *
 * base (length bl): tentative base for the whole group
 * GS (m n-permutations): tentative StrongGenSet for the whole group
 * i: current order in the hierarchy of stabilizers
 *
 * Sizes on input:
 *	i <= bl <= n   Not changed
 *      mm <= m        Not changed
 *      nbl = bl       nbl will increase
 *      nm = m         nm will increase
 */

void schreier_sims_step(int *base, int bl, int *GS, int m, int n,
	int i, int *T, int mm,
	int *newbase, int *nbl, int **newGS, int *nm, int *num) {

	/* Declarations */
	/* Counters */
	int c, j=0, jj, level;
	/* Intermediate permutations */
	int *p=   (int*)malloc(n*sizeof(int));
	int *ip=  (int*)malloc(n*sizeof(int));
	int *pp=  (int*)malloc(n*sizeof(int));
	int *ppp= (int*)malloc(n*sizeof(int));
	/* Stabilizer of base[1...i-1] */
	int *Si= (int*)malloc(m*n*sizeof(int)), Sil;
	/* Old stabilizer. Here we could use mm*n rather than m*n */
	int *oldSi= (int *)malloc(m*n*sizeof(int)), oldSil;
	/* Orbit of base[i] */
	int *Deltai= (int*)malloc(  n*sizeof(int)), Deltail;
	int *w=      (int*)malloc(  n*sizeof(int));
	int *nu=     (int*)malloc(n*n*sizeof(int));
	/* Old orbit */
	int *oldDeltai= (int*)malloc(  n*sizeof(int)), oldDeltail;
	int *oldw=      (int*)malloc(  n*sizeof(int));
	int *oldnu=     (int*)malloc(n*n*sizeof(int));
	/* Generators to check */
	int *genset= (int*)malloc(m*n*sizeof(int)), gensetl;
	/* Loops */
	int gamma, gn, sn;
	int *s= (int*)malloc(n*sizeof(int));
	int *g= (int*)malloc(n*sizeof(int));
	/* Stabilizer */
	int *stab =  (int*)malloc(m*n*sizeof(int)), stabl;
	int *stabps= (int*)malloc(  n*sizeof(int)), stabpsl;

#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("******** schreier_sims_step ********\n");	/*PPC*/
	printf("base:"); print_list(base, bl, 1);		/*PPC*/
	printf("GS (%d perms of degree %d):", m, n);		/*PPC*/
	print_array_perm(GS, m, n, 1);				/*PPC*/
#endif								/*PPC*/
	/* Initialize newbase=base and newGS=GS (and their lengths) */
	/* They are already equal on input. Always true? */
	copy_list(base, newbase, bl);
	*nbl = bl;
	copy_list(GS, *newGS, m*n);
	*nm = m;
	/* Original generating sets. We get Sil<=m and oldSil<=mm */
	stabilizer(base, i-1, GS, m, n, Si, &Sil);
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("Stabilizer of first %d points of base ", i-1);	/*PPC*/
	print_list(base, i-1, 0); printf(" :\n");		/*PPC*/
	print_array_perm(Si, Sil, n, 1);			/*PPC*/
#endif								/*PPC*/
	complement(Si, Sil, T, mm, n, oldSi, &oldSil);
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("Previous stabilizer of %d points:\n", i-1);	/*PPC*/
	print_array_perm(oldSi, oldSil, n, 1);			/*PPC*/
#endif								/*PPC*/
	/* Basic orbits */
	one_schreier_orbit(base[i-1], Si, Sil, n,
		Deltai, &Deltail, nu, w, 1);
	one_schreier_orbit(base[i-1], oldSi, oldSil, n,
		oldDeltai, &oldDeltail, oldnu, oldw, 1);
	/* Check that Deltai is an extension of oldDeltai */
	for(c=0; c<n; c++) {
		if(w[c]!=oldw[c] && oldw[c]!=0) {
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("Deltai[%d] modified to match oldDeltai\n", c);  /*PPC*/
#endif								/*PPC*/
			copy_list(oldnu+c*n, nu+c*n, n);
			w[c] = oldw[c];
		}
	}
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("Orbit: "); print_list(Deltai, Deltail, 1);	/*PPC*/
#endif								/*PPC*/

	/* Loop gn over elements gamma of basic orbit */
	for(gn=0; gn<Deltail; gn++) {

	    gamma = Deltai[gn];

#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("  gamma=%d\n", gamma);				/*PPC*/
#endif								/*PPC*/
	    /* In both cases here we get gensetl<=m */
	    if (position(gamma, oldDeltai, oldDeltail)) {
		copy_list(T, genset, mm*n);
		gensetl=mm;
	    } else {
		copy_list(Si, genset, Sil*n);
		gensetl=Sil; 
	    }

	    /* Loop sn over generators s in genset */
	    for (sn=0; sn<gensetl; sn++) {

		copy_list(genset+sn*n, s, n);
		(*num)++;

		/* Compute Schreier generator */
		trace_schreier(gamma, nu,w, p,n);
		trace_schreier(onpoints(gamma,s,n), nu,w, pp,n);
		inverse(pp, ip, n);
		product(p, s, ppp, n);
		product(ppp, ip, g, n);
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("    g(%d)=", *num); print_perm(g, n, 1);	/*PPC*/
#endif								/*PPC*/

		/* Compute stabilizer. Reallocate to maximum size */
		stab = (int*)realloc(stab, (*nm)*n*sizeof(int));
		stabilizer(newbase, i, *newGS, *nm, n, stab, &stabl);
		/* If g is not in subgroup H^(i) */
		if(!isid(g, n)) {
                if ((stabl==0)||(!perm_member(g, newbase+i,*nbl-i, stab,stabl, n))) {
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("      g not in H^(%d)\n", i);			/*PPC*/
#endif								/*PPC*/
		    /* Enlarge newGS. We need reallocation */
		    *newGS = (int*)realloc(*newGS, ((*nm)+1)*n*sizeof(int));
		    copy_list(g, (*newGS)+(*nm)*n, n);
		    (*nm)++;
		    /* Extend newbase if needed, so that no strong
		     * generator fixes all base points. */
		    stable_points(g, n, stabps, &stabpsl);
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("      Stable under g: "); 			/*PPC*/
	print_list(stabps, stabpsl, 1);				/*PPC*/
#endif								/*PPC*/
		    /* If g moves a point of newbase then set j */
		    for(jj=0; jj<*nbl; jj++) {
			if(!position(newbase[jj], stabps, stabpsl)) {
			    j = jj+1;
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("      g moves %d in newbase\n", newbase[jj]);	/*PPC*/
#endif								/*PPC*/
			    break;
			}
		    /* else choose a new point */
			j = *nbl+1;
		    }
		    if (j == *nbl+1) {
			for(jj=1; jj<=n; jj++) {
			    if(!position(jj, stabps, stabpsl) &&
			       !position(jj, newbase, *nbl)) {
				newbase[*nbl]=jj;
				(*nbl)++;
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("      Point %d added to newbase\n", jj);	/*PPC*/
#endif								/*PPC*/
				break;
			    }
			}
		    }
		    /* Ensure we still have a base and SGS for
		     * H^(i+1) */
		    for(level=j; level>i; level--) {
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("\nEnsuring H^(%d) at level %d\n", i+1, level);	/*PPC*/
#endif								/*PPC*/
		        schreier_sims_step(newbase,*nbl,*newGS,*nm, n,
				level, g, 1,
				newbase,nbl,newGS,nm,num);
		    }
#ifdef VERBOSE_SCHREIER						/*PPC*/
	printf("***** Finished check of H(%d) ******\n\n", i+1);/*PPC*/
#endif								/*PPC*/
		}
		}
	    }
	}

	/* Free allocated memory */
	free(p);
	free(ip);
	free(pp);
	free(ppp);
	free(Si);
	free(oldSi);
	free(Deltai);
	free(w);
	free(nu);
	free(oldDeltai);
	free(oldw);
	free(oldnu);
	free(genset);
	free(s);
	free(g);
	free(stab);
	free(stabps);

}

/**********************************************************************
 *
 * Notations:
 *  - A permutation g corresponding to a given tensor configuration is
 *    understood as acting on index-numbers giving tensor-slot numbers.
 *    That is, p = i^g (which is p = onpoints(i, g)) means that i is the
 *    index at slot p in the configuration g. Equivalently, i = p^ig,
 *    where ig is the inverse of g.
 *  - The base of a SGS for the group S represents an ordering of the
 *    slots of the tensor. Changing the base can be understood as a
 *    change of priorities for index canonicalization.
 *    
 */

/**********************************************************************/

/* coset_rep. JMM, 30 June 2003
 *
 * This algorithm is encoded from Renato Portugal et al.
 *
 * This function gives a canonical representant of a n-permutation p
 * in the group described by a SGS (pair base,GS) and the result is
 * stored in cr.
 * The free slots are different at return time.
 */

void coset_rep(int *p, int n, int *base, int bl, int *GS, int *m,
	int *freeps, int fl, int *cr) {

#ifdef VERBOSE_COSET						/*PPC*/
	printf("*****************************************\n");	/*PPC*/
	printf("***** COSET-REPRESENTATIVE ALGORITHM ****\n");	/*PPC*/
	printf("*****************************************\n\n");/*PPC*/
	printf("Permutation "); print_perm(p, n, 1);		/*PPC*/
	printf("Base: "); print_list(base, bl, 1);		/*PPC*/
#endif								/*PPC*/
	/* Trivial case without symmetries */
	if (*m==0) {
		copy_list(p, cr, n);
		return;
	}
	/* else */

	int i, j, k, b, pp;
	int *deltap=       (int*)malloc(  n*sizeof(int)), deltapl;
	int *deltapsorted= (int*)malloc(  n*sizeof(int));
	int *om=           (int*)malloc(  n*sizeof(int));
	int *perm=         (int*)malloc(  n*sizeof(int));
	int *perm2=        (int*)malloc(  n*sizeof(int));
	int *orbit=        (int*)malloc(  n*sizeof(int)), ol;
	int *orbit1=       (int*)malloc(  n*sizeof(int)), o1l;
	int *w=            (int*)malloc(  n*sizeof(int));
        int *nu=           (int*)malloc(n*n*sizeof(int));
	int *genset=       (int*)malloc(*m*n*sizeof(int)), gensetl;
	int *stab=         (int*)malloc(*m*n*sizeof(int)), mm;

        /* Copy p to perm and GS to genset, to avoid side effects.
	   Invert to have Portugal's notation: p gives slots from
	   indices; perm gives indices from slots */
	inverse(p, perm, n);
	copy_list(GS, genset, *m*n); gensetl=*m;

        /* Loop over elements of base */
	for(i=0; i<bl; i++) {
		b = base[i]; /* Slot to analyze */
		one_schreier_orbit(b, genset, gensetl, n,
			 orbit, &ol, nu, w, 1);
#ifdef VERBOSE_COSET						/*PPC*/
	printf("\nAnalyzing slot %d\n", b);			/*PPC*/
	printf("orbit: "); print_list(orbit, ol, 1);
	printf("freeps: "); print_list(freeps, fl, 1);
#endif								/*PPC*/
		intersection(orbit, ol, freeps, fl, orbit1, &o1l);
#ifdef VERBOSE_COSET						/*PPC*/
	printf("Free slots that can go to that slot: ");	/*PPC*/
	print_list(orbit1, o1l, 1);				/*PPC*/
#endif								/*PPC*/
		if (o1l==0) continue; /* Slot with no symmetries */
		/* else */
		for(j=0; j<o1l; j++) {
			deltap[j] = onpoints(orbit1[j], perm, n);
		}
		deltapl=o1l;
#ifdef VERBOSE_COSET						/*PPC*/
	printf("At those slots we resp. find indices deltap: ");/*PPC*/
		print_list(deltap, deltapl, 1);			/*PPC*/
#endif								/*PPC*/
		sortB(deltap, deltapsorted, deltapl, base, bl);
		k = position(deltapsorted[0], deltap, deltapl);
#ifdef VERBOSE_COSET						/*PPC*/
	printf("Least index: %d, at position k: %d of deltap\n",/*PPC*/
			 deltap[k-1], k);			/*PPC*/
#endif								/*PPC*/
		pp = orbit1[k-1];
#ifdef VERBOSE_COSET						/*PPC*/
	printf("That index is at tensor slot pp: %d\n", pp);	/*PPC*/
#endif								/*PPC*/
		/* Compute perm2 such that b^om = pp */
		trace_schreier(pp, nu, w, om, n);
#ifdef VERBOSE_COSET						/*PPC*/
	printf("We can move slot %d to slot %d with perm om:",	/*PPC*/
			pp, b);					/*PPC*/
		print_perm(om, n, 0); printf(" in S\n");	/*PPC*/
#endif								/*PPC*/
		product(om, perm, perm2, n);
#ifdef VERBOSE_COSET						/*PPC*/
	printf("New list of indices: ");			/*PPC*/
		print_perm(perm2, n, 1);			/*PPC*/
#endif								/*PPC*/
		copy_list(perm2, perm, n);
		/* New position of free indices */
		inverse(om, perm2, n);
		for(j=0; j<fl; j++) {
			freeps[j] = onpoints(freeps[j], perm2, n);
		}
#ifdef VERBOSE_COSET						/*PPC*/
	printf("Removing those perms that move slot %d\n", b);	/*PPC*/
#endif								/*PPC*/
		/* Note that we do not change base to have i as first
		   member of base. This is not general, but I think
		   here it is valid due to the nature of our problem */
		stabilizer(base+i, 1, genset, gensetl, n, stab, &mm);
		copy_list(stab, genset, mm*n); gensetl=mm;
	}
	/* Invert back to our notation */
	inverse(perm, cr, n);
	/* Return new SGS */
	copy_list(genset, GS, gensetl*n); *m=gensetl;
#ifdef VERBOSE_COSET						/*PPC*/
	printf("\n*****************************************\n");/*PPC*/
	printf("************ END OF ALGORITHM ***********\n");	/*PPC*/
	printf("*****************************************\n\n");/*PPC*/
#endif								/*PPC*/

	/* Free allocated memory */
	free(deltap);
	free(deltapsorted);
	free(om);
	free(perm);
	free(perm2);
	free(orbit);
	free(orbit1);
	free(w);
	free(nu);
	free(genset);
	free(stab);

}

/**********************************************************************/

/* double_coset_rep. JMM, 1-5 July 2003.
 * JMM, 9 November 2005. Corrected treatment of SGS of group D.
 *
 * This algorithm is encoded from Renato Portugal et al.
 *
 * This function gives a canonical representant for the n-permutation g
 * in the double coset given by the groups S, described by a SGS (pair
 * base, GS) and the group D, described by dpl pairs of dummies.
 * If sym=1 the metric is assumed to be symmetric. If sym=-1 the metric
 * is assumed to be antisymmetric (spinor calculus). If sym=0 there is
 * no metric. Note that dpl is the number of pairs and not the number of
 * dummies. Note also that n must be even. The result is stored in dcr.
 */

void double_coset_rep(int *g, int n, int *base, int bl, int *GS, int m,
	int *dummypairs, int dpl, int *dummysetlabels, int sym, int *dcr) {

   int i, j, l, /* ii,*/  jj, kk, c;         /* Counters */
	int oi, result=0;
	int *dummyps=       (int*)malloc(2*dpl*sizeof(int));
	int *dummytmp=      (int*)malloc(2*dpl*sizeof(int)), dummytmpl;
	int *newdummypairs= (int*)malloc(2*dpl*sizeof(int)), ndpl;
	int *otherdummies=  (int*)malloc(2*dpl*sizeof(int));
	int *bS=            (int*)malloc(    n*sizeof(int)), bSl;
	int *bSsort=        (int*)malloc(    n*sizeof(int));
	int *KS=            (int*)malloc(  m*n*sizeof(int)), KSl;
	int *KD=            (int*)malloc((2*dpl-1)*n*sizeof(int)), KDl;
	int *bD=            (int*)malloc(  dpl*sizeof(int)), bDl;
	int *nu=            (int*)malloc(  n*n*sizeof(int));
	int *w=             (int*)malloc(    n*sizeof(int));
	int *Deltab=        (int*)malloc(    n*sizeof(int)), Deltabl;
	int *DeltaD=        (int*)malloc(    n*sizeof(int));
	int *IMAGES=        (int*)malloc(    n*sizeof(int)), IMAGESl;
	int *IMAGESsorted=  (int*)malloc(    n*sizeof(int));
	int *p=             (int*)malloc(    n*sizeof(int));
	int *nuD=           (int*)malloc(  n*n*sizeof(int));
	int *wD=            (int*)malloc(    n*sizeof(int));
	int *Deltap=        (int*)malloc(    n*sizeof(int)), Deltapl;
	int *NEXT=          (int*)malloc(    n*sizeof(int)), NEXTl;
	int *L=             (int*)malloc(    n*sizeof(int)), Ll;
	int *L1=            (int*)malloc(    n*sizeof(int)), L1l;
	int *s=             (int*)malloc(    n*sizeof(int));
	int *d=             (int*)malloc(    n*sizeof(int));
	int *list1=         (int*)malloc(    n*sizeof(int)), list1l;
	int *list2=         (int*)malloc(    n*sizeof(int)), list2l;
	int *perm1=         (int*)malloc(    n*sizeof(int));
	int *perm2=         (int*)malloc(    n*sizeof(int));
	int *perm3=         (int*)malloc(    n*sizeof(int));
	int *s1=            (int*)malloc(    n*sizeof(int));
	int *d1=            (int*)malloc(    n*sizeof(int));

        /* Given a list L={i1, ..., il} the function TAB must return
           a pair (s1, d1) corresponding to L. The whole collection of
	   lists L and their pairs is stored in the array ALPHA of
	   structures alphastruct. Each L in ALPHA is actually
	   identified by its position l in the array. The structure
	   for a given L contains a list of values of l corresponding
	   to the n possible extensions of L.
	 */

	/* Define ALPHA and TAB */
	struct alphastruct {
		/* L */
		int L[n]; 	/* We assume that elements of L cannot
				   be repeated. I only have experimental
				   evidence for this */
		int Ll;
		/* s, d */
		int s[n];
		int d[n];
		/* other */
		int o[n];
				   
	} *ALPHA;
	int ALPHAl;
	int *ALPHAstep= (int*)malloc(n*sizeof(int));

	/* Initialize ALPHA to {} and TAB to {id, id} */
	ALPHAl= 1;
	ALPHA= (struct alphastruct*)malloc(sizeof(struct alphastruct));
	ALPHA[0].Ll=0;
	range(ALPHA[0].s, n);
	range(ALPHA[0].d, n);
	ALPHAstep[0]=0;
	ALPHAstep[1]=1;

	/* TAB */
	void TAB(int *L, int Ll, int *s1, int *d1, int n) {

		int i;
		int l=0;
		
		/* Search ALPHA for the l corresponding to L */
		for (i=0; i<Ll; i++) l=ALPHA[l].o[L[i]-1];
		/* Copy permutations of element l */
		copy_list(ALPHA[l].s, s1, n);
		copy_list(ALPHA[l].d, d1, n);

	} /* End of function TAB */

	/* Subroutines F1 and F2 */
	/* TAB1 is an element of S; TAB2 is an element of D */
	void F2(int *TAB1, int *gg, int *TAB2, int *perm, int n) {

		int *tmp1= (int*)malloc(n*sizeof(int));
		int *tmp2= (int*)malloc(n*sizeof(int));

		inverse(gg, tmp1, n);
		product(TAB1, tmp1, tmp2, n);
		product(tmp2, TAB2, perm, n);

		free(tmp1);
		free(tmp2);

	} /* End of function F2 */

	void F1(int *L, int Ll, int *gg, int *list, int *listl, int n) {

		int c, c1, c2;
		int *sigd= (int*)malloc(n*sizeof(int));
		int *TAB1= (int*)malloc(n*sizeof(int));
		int *TAB2= (int*)malloc(n*sizeof(int));
		int *tmp=  (int*)malloc(n*sizeof(int));

		TAB(L, Ll, TAB1, TAB2, n);

		/* Compute s.ig.d */
		F2(TAB1, gg, TAB2, sigd, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("With L="); print_list(L, Ll, 0);		/*PPC*/
	printf(" we get sigd: "); print_perm(sigd, n, 1);	/*PPC*/
#endif								/*PPC*/
		/* Images of Deltab under sigd. Note that tmp has length
		   Deltabl */
		for (c=0; c<Deltabl; c++)
			tmp[c] = onpoints(Deltab[c], sigd, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf(" which maps slots in Deltab to indices list: ");/*PPC*/
		print_list(tmp, Deltabl, 1);			/*PPC*/
#endif								/*PPC*/
		/* Orbits of DeltaD containing the points in tmp */
		for (c1=0; c1<Deltabl; c1++) {
		    oi = DeltaD[tmp[c1]-1];
		    if(oi) {
			for(c2=0; c2<n; c2++) {
			    if ((DeltaD[c2]==oi) && 
				(!position(c2+1, list, *listl)))
				 list[(*listl)++] = c2+1;
			}
		    }
		}
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf(" whose points belong to orbits ");		/*PPC*/
	print_list(list, *listl, 1);				/*PPC*/
#endif								/*PPC*/
		free(sigd);
		free(TAB1);
		free(TAB2);
		free(tmp);
	} /* End of function F1 */

        /* Consistency check */
	int consistency(int *array, int m, int n) {

		int *arrayp= (int*)malloc(m*n*sizeof(int)), arraypl;
		int *arrayn= (int*)malloc(m*n*sizeof(int)), arraynl;
		int i, ip, in, ret;

#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("Checking consistency in m:%d, n:%d\n", m, n);	/*PPC*/
	print_array_perm(array, m, n, 1);			/*PPC*/
#endif								/*PPC*/

		/* Detect sign of permutation */
		arraypl=0;
		arraynl=0;
		for(i=0; i<m; i++) {
		    if (array[i*n+n-2]<array[i*n+n-1]) /* Positive */
			copy_list(array+i*n, arrayp+(arraypl++)*n, n);
		    else                               /* Negative */
			copy_list(array+i*n, arrayn+(arraynl++)*n, n);
		}
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("Found positive perms: %d\n", arraypl);		/*PPC*/
	printf("Found negative perms: %d\n", arraynl);		/*PPC*/
#endif								/*PPC*/
		/* Here there are n*n comparisons */
		ret = 1; /* True */
		for (in=0; in<arraynl; in++) {
		    for (ip=0; ip<arraypl; ip++) {
			if (equal_list(arrayp+ip*n, arrayn+in*n, n-2)) {
			    ret = 0; /* False */
			    break;
			}
		    }
		}
#ifdef VERBOSE_DOUBLE						/*PPC*/
	if (ret) printf("Found no problem in check\n");		/*PPC*/
	else printf("Found perm with two signs.\n");		/*PPC*/
#endif								/*PPC*/
		free(arrayp);
		free(arrayn);
		return(ret);

	} /* End of funcion consistency */

	/* (Slot-) positions of dummies */
	for (i=0; i<2*dpl; i++) {
		dummyps[i] = onpoints(dummypairs[i], g, n);
	}
//	printf("dummyps    = "); print_list(dummyps, 2*dpl, 1); // KP
//	printf("dummypairs = "); print_list(dummypairs, 2*dpl, 1); // KP

	/* Initialize KS */
	copy_list(GS, KS, m*n); KSl=m;

	/* Extend base to cover all positions of dummies.
	 * We sort the missing dummies  */
	intersection(base, bl, dummyps, 2*dpl, bS, &bSl);
	complement(dummyps, 2*dpl, base, bl, 1, dummytmp, &dummytmpl);
	sort(dummytmp, otherdummies, dummytmpl);
	copy_list(otherdummies, bS+bSl, dummytmpl);
	bSl = bSl + dummytmpl;
	/* Generate associated base for sorting names of dummies */
	sort(dummypairs,otherdummies,2*dpl);
	sort(bS, dummytmp, bSl);
	for (i=0; i<bSl; i++) {
		bSsort[i]=otherdummies[position(bS[i],dummytmp,bSl)-1];
	}
	

#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("dummytmp: ");					/*PPC*/
	print_list(dummytmp, bSl, 1);				/*PPC*/
	printf("otherdummies: ");				/*PPC*/
	print_list(otherdummies, bSl, 1);			/*PPC*/
	printf("bS: ");						/*PPC*/
	print_list(bS, bSl, 1);					/*PPC*/
	printf("bSsort: ");					/*PPC*/
	print_list(bSsort, bSl, 1);				/*PPC*/
#endif								/*PPC*/

	/**** SGS of group D for a given list of dummy pairs ****/
	void SGSD(int *dpairs, int dpl, int *dummysetlabels, int n, int sym, int firstd,
				 int *KD, int *KDl, int *bD, int *bDl) {
		if (dpl==0) {
			*KDl=0;
			*bDl=0;
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("Empty KD\n");					/*PPC*/
#endif								/*PPC*/
			return;
			} /* else */
//	print_list(dpairs, 2*dpl, 1);				/*PPC*/ // KP
		
		int *range_perm = (int*)malloc(        n*sizeof(int));
		int *KD1 =        (int*)malloc((dpl-1)*n*sizeof(int));
		int *KD2 =        (int*)malloc(    dpl*n*sizeof(int));
		int i,j;
		
		/* Rearrange dummies if required */
		if (firstd) { 
//			printf("firstd = %d\n", firstd); // KP
			/* Find position of dummy and relative
			   position of its pair */
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("Rearrange dummies. dpairs: ");			/*PPC*/
			print_list(dpairs, 2*dpl, 1);				/*PPC*/
#endif								/*PPC*/
			int pos;
			pos = position(firstd, dpairs, 2*dpl)-1;
			if (pos==-1) { /* firstd not in dpairs */
				/* Do nothing */
				} 
			else {
				int tmp;
				if (pos%2==0) {
					/* 1st member: Do nothing */
					} 
				else {
					pos=pos-1;
					/* 2nd member: Swap all pairs */
					/* Is this valid in spinor calculus? */
					for (j=0; j<dpl; j++) {
						tmp= dpairs[2*j];
						dpairs[2*j]=dpairs[2*j+1];
						dpairs[2*j+1]=tmp;
						}
					}
				if (pos==0) {
					/* Do nothing */
					} 
				else {
					/* Swap two pairs of dummies */
//					printf("swapping %d and %d with %d and %d\n", dpairs[0],dpairs[1],firstd,dpairs[pos+1]); // KP
//					if(dummysetlabels[dpairs[0]]==dummysetlabels[firstd]) {
//						printf("yes\n");
					tmp=dpairs[0];
					dpairs[0]=firstd;
					dpairs[pos]=tmp;
					tmp=dpairs[1];
					dpairs[1]=dpairs[pos+1];
					dpairs[pos+1]=tmp;
//						}
					}
				}
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("Rearrange dummies. Now dpairs: ");		/*PPC*/
			print_list(dpairs, 2*dpl, 1);				/*PPC*/
#endif								/*PPC*/
			} /* End of if(firstd) */
		
		range(range_perm, n);
		/* KD1: exchange indices. Ex: Cycles[{1,3},{2,4}] */
		for (i=0; i<dpl-1; i++) {
			/* Copy the identity */
			copy_list(range_perm, KD1+i*n, n);
			/* Swap elements of consecutive pairs */
			// KP: If the two consecutive pairs sit in different sets, don't swap
//			printf("%d ?= %d\n", dummysetlabels[2*i], dummysetlabels[2*i+2]); // KP
			if(dummysetlabels[dpairs[2*i]]==dummysetlabels[dpairs[2*i+2]]) {
//				printf("swapping elements %d %d with %d %d\n", dpairs[2*i], dpairs[2*i+1], dpairs[2*i+2], dpairs[2*i+3]); // KP
				KD1[ i*n+dpairs[2*i  ]-1 ] = dpairs[2*i+2];
				KD1[ i*n+dpairs[2*i+2]-1 ] = dpairs[2*i  ];
				KD1[ i*n+dpairs[2*i+1]-1 ] = dpairs[2*i+3];
				KD1[ i*n+dpairs[2*i+3]-1 ] = dpairs[2*i+1];
				}
			}
		/* KD2: exchange indices of the same pair */
		if (sym == 1) { /* Symmetric metric */
			for (i=0; i<dpl; i++) {
				/* Copy the identity */
				copy_list(range_perm, KD2+i*n, n);
				/* Swap elements of pair */
				KD2[i*n+dpairs[2*i]-1] = dpairs[2*i+1];
				KD2[i*n+dpairs[2*i+1]-1] = dpairs[2*i];
				/* Keep positive sign */
				}
			*KDl = 2*dpl-1;
			} 
		else if (sym == -1) {
			for (i=0; i<dpl; i++) {
				/* Copy the identity */
				copy_list(range_perm, KD2+i*n, n);
				/* Swap elements of pair */
				KD2[i*n+dpairs[2*i]-1] = dpairs[2*i+1];
				KD2[i*n+dpairs[2*i+1]-1] = dpairs[2*i];
				/* Set negative sign */
				KD2[i*n+n-2] = n;
				KD2[i*n+n-1] = n-1;
				}
			*KDl = 2*dpl-1;
			} 
		else if (sym == 0) {
			/* Do nothing */
			*KDl = dpl-1;
			} 
		else {
			/* Unknown value of sym */
			}
		/* KD */
		copy_list(KD1, KD, (dpl-1)*n);
		if (sym!=0) copy_list(KD2, KD+(dpl-1)*n, dpl*n);
		/* base of group D */
		for (i=0; i<dpl; i++) {
/*			if (dpairs[2*i]<dpairs[2*i+1]) */
			bD[i] = dpairs[2*i];
/*			else 	bD[i] = dpairs[2*i+1]; */
			}
		*bDl=dpl;
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("KD: "); print_array_perm(KD, *KDl, n, 1);	/*PPC*/ // KP
		printf("bD: "); print_list(bD, *bDl, 1);		/*PPC*/
#endif								/*PPC*/
		free(range_perm);
		free(KD1);
		free(KD2);
		} /* End of function SGSD */
	
	/* Initialize working list of dummies */
	copy_list(dummypairs, newdummypairs, 2*dpl);
	ndpl= dpl;
	
	/*******************/
	/**** Main loop ****/
	/*******************/
	
	for (i=1; i<bSl; i++) {
		int b=bS[i-1];
		
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("\n************** Loop i=%d *************\n", i);/*PPC*/
		printf("Analyzing slot bS[%d]=%d of tensor\n", i-1, b);	/*PPC*/
#endif								/*PPC*/
		/* Schreier vector of S */
		schreier_vector(b, KS, KSl, n, nu, w);
		one_orbit(b, KS, KSl, n, Deltab, &Deltabl);
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("Under S, slot %d go to slots Deltab: ",b);	/*PPC*/
		print_list(Deltab, Deltabl, 1);				/*PPC*/
#endif								/*PPC*/
		/* Compute SGS for group D. Do not rearrange dummies */
		SGSD(newdummypairs, ndpl, dummysetlabels, n, sym, 0, KD, &KDl, bD, &bDl);
		/* Orbits of D */
		all_orbits(KD, KDl, n, DeltaD);
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("Orbits of indices: DeltaD: ");			/*PPC*/
		print_list(DeltaD, n, 1);				/*PPC*/
#endif								/*PPC*/
		/* Images of b under elements of S.ig.D.
			Deltab and DeltaD are used by F1 */
		IMAGESl=0;
		for (c=ALPHAstep[i-1]; c<ALPHAstep[i]; c++)
	    	F1(ALPHA[c].L, ALPHA[c].Ll, g, IMAGES, &IMAGESl, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("At slot %d we can have indices IMAGES: ", b);	/*PPC*/
		print_list(IMAGES, IMAGESl, 1);				/*PPC*/
		printf("IMAGESl: %d\n", IMAGESl);			/*PPC*/
#endif								/*PPC*/
		/* If there are no images we have finished */
		if(IMAGESl==0) continue;
		
		/* Find minimum index */
		sortB(IMAGES, IMAGESsorted, IMAGESl, bSsort, bSl);
		p[i-1] = IMAGESsorted[0];
		/* NOTE THAT WE DO NOT USE bS FOR SORTING */
		/* p[i-1] = minim(IMAGES, IMAGESl); */
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("The least of them is p[i-1]=%d\n", p[i-1]);	/*PPC*/
#endif								/*PPC*/
		/* Recompute SGS of D. Rearrange dummies */
		if (ndpl>0 && p[i-1]!=newdummypairs[0]) {
			SGSD(newdummypairs, ndpl, dummysetlabels, n, sym, p[i-1],
				  KD, &KDl, bD, &bDl);
			} else { /* Do nothing */
#ifdef VERBOSE_DOUBLE						/*PPC*/
				printf("Rearrangement of base of D not required.\n");	/*PPC*/
#endif								/*PPC*/
				}
		/* Schreier vector of D */
		schreier_vector(p[i-1], KD, KDl, n, nuD, wD);
		/* Orbit of p[i-1] under D */
		one_orbit(p[i-1], KD, KDl, n, Deltap, &Deltapl);
#ifdef VERBOSE_DOUBLE						/*PPC*/
		printf("The orbit of index %d is Deltap: ", p[i-1]);	/*PPC*/
		print_list(Deltap, Deltapl, 1);				/*PPC*/
		printf("Looking for dgs moving index %d to slot %d\n",	/*PPC*/
				 p[i-1], b);				/*PPC*/
#endif								/*PPC*/
		/* Calculate ALPHA and TAB */
		ALPHAstep[i+1]=ALPHAstep[i];
		for(l=ALPHAstep[i-1]; l<ALPHAstep[i]; l++) {
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("Loop with l=%d\n", l);				/*PPC*/
#endif								/*PPC*/
			Ll=ALPHA[l].Ll;
			copy_list(ALPHA[l].L, L, Ll);
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("L: "); print_list(L, Ll, 1);			/*PPC*/
#endif								/*PPC*/
			copy_list(ALPHA[l].s, s, n);
			copy_list(ALPHA[l].d, d, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("TAB[L]={");					/*PPC*/
			print_perm(s, n, 0);					/*PPC*/
			printf(", ");						/*PPC*/
			print_perm(d, n, 0);					/*PPC*/
			printf("}\n");						/*PPC*/
#endif								/*PPC*/
			list1l=Deltabl;
			for (c=0; c<list1l; c++) 
				list1[c]=onpoints(Deltab[c], s, n);
			inverse(d, perm1, n);
			product(perm1, g, perm2, n);
			list2l=Deltapl;
			for (c=0; c<list2l; c++) 
				list2[c]=onpoints(Deltap[c], perm2, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("NEXT: intersection of sets of slots ");		/*PPC*/
			print_list(list1, list1l, 0); printf(" and ");		/*PPC*/
			print_list(list2, list2l, 1);				/*PPC*/
#endif								/*PPC*/
			intersection(list1, list1l, list2, list2l,
							 NEXT, &NEXTl);
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("Intermediate slots NEXT: ");			/*PPC*/
			print_list(NEXT, NEXTl, 1);				/*PPC*/
#endif								/*PPC*/
			
			for(jj=0; jj<NEXTl; jj++) {
				j = NEXT[jj];
				inverse(s, perm1, n);
				trace_schreier(onpoints(j,perm1,n), nu,w, perm2,n);
				product(perm2, s, s1, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
				printf("From slot %d to interm. slot %d use s1: ",	/*PPC*/
						 b, j); print_perm(s1, n, 1);			/*PPC*/
#endif								/*PPC*/
				inverse(g, perm1, n);
				product(perm1, d, perm2, n);
				trace_schreier(onpoints(j,perm2,n),nuD,wD,perm3,n);
				inverse(perm3, perm1, n);
				product(d, perm1, d1, n);
				copy_list(L, L1, Ll); L1l=Ll;
				L1[L1l++] = j;
#ifdef VERBOSE_DOUBLE						/*PPC*/
				printf("d1: "); print_perm(d1, n, 1);			/*PPC*/
				printf("L1: "); print_list(L1, L1l, 1);			/*PPC*/
#endif								/*PPC*/
				
				kk=ALPHAstep[i+1];
				ALPHA[l].o[j-1] = kk;
				ALPHAl++;
				ALPHAstep[i+1]++;
				ALPHA = (struct alphastruct*)realloc(ALPHA, ALPHAl*
																 sizeof(struct alphastruct));
				copy_list(L1, ALPHA[kk].L, L1l);
				ALPHA[kk].Ll = L1l;
				copy_list(s1, ALPHA[kk].s, n);
				copy_list(d1, ALPHA[kk].d, n);
				
				F2(s1, g, d1, perm1, n);
#ifdef VERBOSE_DOUBLE						/*PPC*/
				printf("This gives the new index configuration: ");	/*PPC*/
				inverse(perm1, perm2, n);				/*PPC*/
				print_perm(perm2, n, 1);				/*PPC*/
				for(ii=0; ii<i; ii++) {					/*PPC*/
					inverse(g, perm1, n);				/*PPC*/
					product(s1, perm1, perm2, n);			/*PPC*/
					product(perm2, d1, perm3, n);			/*PPC*/
					printf("Checking slot %d with point %d: ",	/*PPC*/
							 bS[ii], p[ii]);				/*PPC*/
					if(onpoints(bS[ii], perm3, n)==p[ii])		/*PPC*/
						printf("True\n");			/*PPC*/
					else printf("*************FALSE************\n");/*PPC*/
					}							/*PPC*/
#endif								/*PPC*/
				}
			}
		{ /* Verify if there are 2 equal permutations
			  of opposite sign in SgD */
			int *array= (int*)malloc(n*(ALPHAstep[i+1]-ALPHAstep[i])
											 *sizeof(int));
			int arrayl= 0;
#ifdef VERBOSE_DOUBLE						/*PPC*/
			printf("Astep[i-1]=%d, Astep[i]=%d, Astep[i+1]=%d\n",	/*PPC*/
					 ALPHAstep[i-1], ALPHAstep[i], ALPHAstep[i+1]);	/*PPC*/
#endif 								/*PPC*/
			for(l=ALPHAstep[i]; l<ALPHAstep[i+1]; l++) {
				F2(ALPHA[l].s, g, ALPHA[l].d, perm1, n);
	            copy_list(perm1, array+(arrayl++)*n, n);
	        }
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("Perform check.\n");				/*PPC*/
#endif								/*PPC*/
	        if (!consistency(array, arrayl, n)) {
		    result=0;
		    break;
	        } else result=1;
	        free(array);
	    }
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("Result of check: %d\n", result);		/*PPC*/
#endif								/*PPC*/
	    /* Find the stabilizers S^(i+1) and D^(i+1) */
	    stabilizer(&bS[i-1], 1, KS, KSl, n, KS, &KSl);
            if (p[i-1]==newdummypairs[0]) {
	        for (j=2;j<2*ndpl;j++)
                    newdummypairs[j-2]=newdummypairs[j];
	        if (ndpl>0) ndpl= ndpl-1;
            }
#ifdef VERBOSE_DOUBLE						/*PPC*/
	printf("Remove perms of KS moving slot %d\n", b);	/*PPC*/
	printf("Remove perms of KD moving index %d\n", p[i-1]);	/*PPC*/
#endif								/*PPC*/

	}

	/* Result */
	if (result==0) {
		zeros(perm1, n);
	}
	else {
		l=ALPHAstep[i-1];
		F2(ALPHA[l].s, g, ALPHA[l].d, perm2, n);
		inverse(perm2, perm1, n);
	}
	copy_list(perm1, dcr, n);

	/* Free allocated memory */
	free(ALPHA);
	free(ALPHAstep);
	free(dummyps);
	free(dummytmp);
	free(newdummypairs);
	free(otherdummies);
	free(bS);
	free(bSsort);
	free(KS);
	free(KD);
	free(bD);
	free(nu);
	free(w);
	free(Deltab);
	free(DeltaD);
	free(IMAGES);
	free(IMAGESsorted);
	free(p);
	free(nuD);
	free(wD);
	free(Deltap);
	free(NEXT);
	free(L);
	free(L1);
	free(s);
	free(d);
	free(list1);
	free(list2);
	free(perm1);
	free(perm2);
	free(perm3);
	free(s1);
	free(d1);
		
}

/**********************************************************************/

/* canonical_perm. JMM, 5 July 2003
 *
 * This function finds a canonical representant of the permutation perm
 * in the double_coset with groups S and D. The symmetry group S is
 * given through the generating set GS (with m n-permutations), that
 * can be a SGS (then SGSQ must be 1) or not (then SGSQ must be 0).
 * In the former case base (length bl) contains the associated base;
 * in the latter case a SGS will be constructed using the Schreier_Sims
 * algorithm using the points in base as the first points for the base.
 * The "dummy" group D is given through the list dummyps of dl pairs
 * of positions of dummies. The list freeps (length fl) contains the
 * positions of the free indices. Clearly 2*dl+fl+2=n (this is checked).
 * Finally ob=1 (ob=0) means that the base used in the double_coset
 * algorithm will be (will not necessarily be) an ordered base 1,2,3,...
 * The result is stored in the permutation cperm.
 * Note that we input a pointer to GS, and not a pointer to a pointer
 * because there is no need to return a modified GS.
 */

void canonical_perm(int *perm,
	int SGSQ, int *base, int bl, int *GS, int m, int n,
	int *freeps, int fl, int *dummyps, int dl, int *dummysetlabels, int ob, int metricQ,
	int *cperm) {

	int i;
	int *newfreeps= (int*)malloc(fl*sizeof(int));
	int *iperm=     (int*)malloc(n *sizeof(int));
	int *perm1=     (int*)malloc(n *sizeof(int));
	int *hperm=     (int*)malloc(n *sizeof(int));
	int *perm2=     (int*)malloc(n *sizeof(int));
	int *newbase=   (int*)malloc(n *sizeof(int)), newbl;
	int **newGS = NULL;
	int *pointer;
	int newm;
	int *tmpbase=   (int*)malloc(n *sizeof(int)), tmpbl;
	int *dummies=   (int*)malloc(2*dl*sizeof(int));
	int num=0;

	pointer= (int*)malloc(m*n*sizeof(int));
	newGS= &pointer;

#ifdef VERBOSE_CANON						/*PPC*/
	printf("can: input base: ");				/*PPC*/
	print_list(base, bl, 1);				/*PPC*/
#endif								/*PPC*/

	if (!SGSQ) { /* Compute a Strong Generating Set */
		nonstable_points(base, bl, GS, m, n,
			tmpbase, &tmpbl);
		schreier_sims(tmpbase, tmpbl, GS, m, n,
			newbase, &newbl, newGS, &newm, &num);
//      int kk, kk2;
//   FILE *oo=fopen("/tmp/xperm.log", "w");
//      fprintf(oo, "computed by SS :\n");
//      for(kk=0; kk<newm; ++kk) {
//         for(kk2=0; kk2<n; ++kk2)
//            fprintf(oo, "%d ", (*newGS)[kk*n+kk2]);
//         fprintf(oo, "\n");
//         }
//      fprintf(oo, "\n");
//      for(kk=0; kk<newbl; ++kk)
//         fprintf(oo, "%d ", newbase[kk]);
//      fprintf(oo, "\n");
//   fclose(oo);
	} else {
		copy_list(base, newbase, bl); newbl=bl;
		copy_list(GS, *newGS, m*n); newm=m;
	}

#ifdef VERBOSE_CANON						/*PPC*/
	printf("can: SGS computed.\n");				/*PPC*/
#endif								/*PPC*/

	if ((newbl!=0) && (ob)) {
		range(newbase, newbl=maxim(newbase, newbl));
#ifdef VERBOSE_CANON						/*PPC*/
        printf("can: newbase: ");				/*PPC*/
        print_list(newbase, newbl, 1);				/*PPC*/
#endif								/*PPC*/
	
	}

	copy_list(freeps, newfreeps, fl);
	coset_rep(perm, n, newbase, newbl, *newGS, &newm, 
		newfreeps, fl, perm1);
#ifdef VERBOSE_CANON						/*PPC*/
	printf("can: Canonical perm after coset algorithm: ");	/*PPC*/
	print_perm(perm1, n, 1);				/*PPC*/
	printf("New positions of free indices: ");		/*PPC*/
	print_list(newfreeps, fl, 1);				/*PPC*/
#endif								/*PPC*/

	if (dl==0) { /* No dummy indices */
		copy_list(perm1, cperm, n);
		} 
	else {
		complement(newbase, newbl, newfreeps, fl, 1, tmpbase, &tmpbl);
		copy_list(tmpbase, newbase, tmpbl); newbl=tmpbl;
		stabilizer(newfreeps, fl, *newGS, newm, n, *newGS, &newm);
#ifdef VERBOSE_CANON						/*PPC*/
		printf("can: newbase after fixing: ");			/*PPC*/
		print_list(newbase, newbl, 1);				/*PPC*/
		printf("can: newGS after fixing: ");			/*PPC*/
		print_array_perm(*newGS, newm, n, 1);			/*PPC*/
#endif								/*PPC*/
		/* New dummies */
		inverse(perm, iperm, n);
//		printf("dummyps = "); print_list(dummyps, 2*dl, 1); // KP
//		printf("iperm = "); print_list(iperm, n, 1); // KP
		for (i=0; i<2*dl; i++) {
			dummies[i] = onpoints(dummyps[i], iperm, n);
			}
//		printf("dummies = "); print_list(dummies, 2*dl, 1); // KP
#ifdef VERBOSE_CANON						/*PPC*/
		printf("can: dummies: "); print_list(dummies, 2*dl, 1);	/*PPC*/
#endif								/*PPC*/
		/* Apply dummy-indices algorithm */
#ifdef VERBOSE_CANON						/*PPC*/
		printf("can: Starting double_coset algorithm.\n");	/*PPC*/
#endif								/*PPC*/
		double_coset_rep(perm1, n, newbase, newbl, *newGS, newm,
							  dummies, dl, dummysetlabels, metricQ, perm2);
#ifdef VERBOSE_CANON						/*PPC*/
		printf("can: Finished double_coset algorithm.\n");	/*PPC*/
#endif								/*PPC*/
		
		copy_list(perm2, cperm, n);
		}

	/* Free allocated memory */
	free(newfreeps);
	free(iperm);
	free(perm1);
	free(hperm);
	free(perm2);
	free(newbase);
	free(tmpbase);
	free(dummies);
	free(*newGS); 
	/* Also freeing either pointer or newGS makes xperm crash */

#ifdef VERBOSE_CANON						/*PPC*/
	printf("\n*****************************************\n");/*PPC*/
	printf("************ END OF ALGORITHM ***********\n");	/*PPC*/
	printf("*****************************************\n\n");/*PPC*/
#endif								/*PPC*/
}

