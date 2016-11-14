/* $Id$ $Revision$ */
/* vim:set shiftwidth=4 ts=8: */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: See CVS logs. Details at http://www.graphviz.org/
 *************************************************************************/

/*
 * Written by Stephen North and Eleftherios Koutsofios.
 */

#include "config.h"

#include "gvc.h"
#include "gvio.h"

#ifdef WIN32_DLL
__declspec(dllimport) boolean MemTest;
__declspec(dllimport) int GvExitOnUsage;
/*gvc.lib cgraph.lib*/
    #pragma comment( lib, "cgraph.lib" )
    #pragma comment( lib, "gvc.lib" )
#else   /* not WIN32_DLL */
#include "globals.h"
#endif

#include <stdlib.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_FENV_H) && defined(HAVE_FEENABLEEXCEPT)
/* _GNU_SOURCE is needed for feenableexcept to be defined in fenv.h on GNU
 * systems.   Presumably it will do no harm on other systems. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
# include <fenv.h>
#elif HAVE_FPU_CONTROL_H
# include <fpu_control.h>
#elif HAVE_SYS_FPU_H
# include <sys/fpu.h>
#endif

static GVC_t *Gvc;
static graph_t * G;

#ifndef WIN32
static void intr(int s)
{
/* if interrupted we try to produce a partial rendering before exiting */
    if (G)
	gvRenderJobs(Gvc, G);
/* Note that we don't call gvFinalize() so that we don't start event-driven
 * devices like -Tgtk or -Txlib */
    exit (gvFreeContext(Gvc));
}

#ifndef NO_FPERR
static void fperr(int s)
{
    fprintf(stderr, "caught SIGFPE %d\n", s);
    /* signal (s, SIG_DFL); raise (s); */
    exit(1);
}

static void fpinit(void)
{
#if defined(HAVE_FENV_H) && defined(HAVE_FEENABLEEXCEPT)
    int exc = 0;
# ifdef FE_DIVBYZERO
    exc |= FE_DIVBYZERO;
# endif
# ifdef FE_OVERFLOW
    exc |= FE_OVERFLOW;
# endif
# ifdef FE_INVALID
    exc |= FE_INVALID;
# endif
    feenableexcept(exc);

#ifdef HAVE_FESETENV
#ifdef FE_NONIEEE_ENV
    fesetenv (FE_NONIEEE_ENV);
#endif
#endif

#elif  HAVE_FPU_CONTROL_H
    /* On s390-ibm-linux, the header exists, but the definitions
     * of the masks do not.  I assume this is temporary, but until
     * there's a real implementation, it's probably safest to not
     * adjust the FPU on this platform.
     */
# if defined(_FPU_MASK_IM) && defined(_FPU_MASK_DM) && defined(_FPU_MASK_ZM) && defined(_FPU_GETCW)
    fpu_control_t fpe_flags = 0;
    _FPU_GETCW(fpe_flags);
    fpe_flags &= ~_FPU_MASK_IM;	/* invalid operation */
    fpe_flags &= ~_FPU_MASK_DM;	/* denormalized operand */
    fpe_flags &= ~_FPU_MASK_ZM;	/* zero-divide */
    /*fpe_flags &= ~_FPU_MASK_OM;        overflow */
    /*fpe_flags &= ~_FPU_MASK_UM;        underflow */
    /*fpe_flags &= ~_FPU_MASK_PM;        precision (inexact result) */
    _FPU_SETCW(fpe_flags);
# endif
#endif
}
#endif
#endif

static graph_t *create_test_graph(void)
{
#define NUMNODES 5

    Agnode_t *node[NUMNODES];
    Agedge_t *e;
    Agraph_t *g;
    Agraph_t *sg;
    int j, k;
    char name[10];

    /* Create a new graph */
    g = agopen("new_graph", Agdirected,NIL(Agdisc_t *));

    /* Add nodes */
    for (j = 0; j < NUMNODES; j++) {
	sprintf(name, "%d", j);
	node[j] = agnode(g, name, 1);
	agbindrec(node[j], "Agnodeinfo_t", sizeof(Agnodeinfo_t), TRUE);	//node custom data
    }

    /* Connect nodes */
    for (j = 0; j < NUMNODES; j++) {
	for (k = j + 1; k < NUMNODES; k++) {
	    e = agedge(g, node[j], node[k], NULL, 1);
	    agbindrec(e, "Agedgeinfo_t", sizeof(Agedgeinfo_t), TRUE);	//edge custom data
	}
    }
    sg = agsubg (g, "cluster1", 1);
    agsubnode (sg, node[0], 1);

    return g;
}

const char *dograph(const char *dot)
{
  const char *data = NULL;
  unsigned int length = 0;
  for (int i = 0; i < 1; i++) {
    G = agmemread(dot);

    if (G) {
      gvLayoutJobs(Gvc, G);  /* take layout engine from command line */
      gvRenderData(Gvc, G, "svg", &data, &length);
      //gvFreeLayout(Gvc, G);
      //agclose(G);
    }

    if (i != 0)
      free(data);
  }

  return data;
}

const char *str = "hi there";
const char **pstr = &str;
const char ***ppstr = &pstr;

__attribute__((jsexport)) typeof(str) str;
__attribute__((jsexport)) typeof(pstr) pstr;
__attribute__((jsexport)) typeof(ppstr) ppstr;

struct __attribute__((jsexport)) st {
  const char *str;
  const char **pstr;
  const char ***ppstr;
};

struct st st1 = { "hey", &st1.str, &st1.pstr };
struct st *pst1 = &st1;
struct st **ppst1 = &pst1;

__attribute__((jsexport)) typeof(st1) st1;
__attribute__((jsexport)) typeof(pst1) pst1;
__attribute__((jsexport)) typeof(ppst1) ppst1;

__attribute__((jsexport)) typeof(dograph) dograph;
__attribute__((jsexport)) typeof(free) free;
__attribute__((jsexport)) typeof(malloc) malloc;

#define JSEXPORT_PASTE(a,b) a ## b
#define JSEXPORT_TYPE2(type, line) JSEXPORT_PASTE(typedef __attribute__((jsexport)) typeof(type) t_, line);
#define JSEXPORT_TYPE(type) JSEXPORT_TYPE2(type, __COUNTER__)

JSEXPORT_TYPE(G);
JSEXPORT_TYPE(G->e_id);

static __attribute__((jsexport)) typeof(G) G;

struct __attribute__((jsexport)) s2 {
  int x;
  int y;
  int z;
  int *ip;
  int **ipp;

  struct s2 *next;
  struct s2 **pprev;
};

__attribute__((jsexport)) struct s2 s;

int main(int argc, char **argv)
{
  Gvc = gvContextPlugins(lt_preloaded_symbols, DEMAND_LOADING);
  GvExitOnUsage = 1;
  gvParseArgs(Gvc, argc, argv);

  return 0;
}
