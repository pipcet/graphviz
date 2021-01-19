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
 * Generator of gpr include file from table data
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/******** Parameters **********/

#define MINNAME 1

/*************************/

static char *header = "#ifndef %s\n\
#define %s\n\n\
/* generated by mkdefs; do not edit */\n\n";

static char *prefix = "#define Y(i)    (1<<(i))\n\
\n\
#define V       0x4      /* NODE         */\n\
#define E       0x5      /* EDGE         */\n\
#define G       0x6      /* GRAPH        */\n\
#define O       0x7      /* OBJECT       */\n\
#define TV      0x8      /* TV_          */\n\
#define YALL    (Y(V)|Y(E)|Y(G))\n\
\n";

/*************************/

#define BSIZE  512

#define K_val 1
#define K_member 2

typedef struct rec record;

struct rec {
    record *next;
    char *symbol;
    char *name;
    char *lex;
    char *type;
    char *domain;
    char *range;
    char *data;
    int kind;
};

static int lineno;
static int tokno;

static record *newRec(record * curr)
{
    record *newr;

    newr = malloc(sizeof(record));
    if (!newr) {
	fprintf(stderr, "mkdefs: out of memory, line %d\n", lineno);
	exit(1);
    }
    curr->next = newr;
    newr->next = 0;
    return newr;
}

static void genGuard(char *filename, char *guard)
{
    char c;

    while ((c = *filename++)) {
	if (c == '.')
	    *guard++ = '_';
	else
	    *guard++ = toupper(c);
    }
    *guard = '\0';
}

static char *strTok(char *str)
{
    char *tk;

    tokno++;
    tk = strtok(str, " \t\n");
    return tk;
}

int main(int argc, char *argv[])
{
    char *filename;
    char *buf;
    char *tk;
    record vals;
    record *recp = &vals;
    FILE *fp;
    char guard[100];
    int idx = MINNAME;
    int lastval = 0;
    int lastmem = 0;

    if (argc != 2) {
	fprintf(stderr, "mkdefs: 1 argument necessary\n");
	exit(1);
    }
    filename = argv[1];

    vals.next = 0;
    buf = malloc(BSIZE);
    while (fgets(buf, BSIZE, stdin)) {
	lineno++;
	tokno = 0;
	tk = strTok(buf);
	if ((!tk) || (*tk == '#'))
	    continue;		/* comment */
	recp = newRec(recp);
	if (*tk == 'V')
	    recp->kind = K_val;
	else if (*tk == 'M')
	    recp->kind = K_member;
	else
	    recp->kind = 0;
	recp->data = buf;
	recp->symbol = tk;
	recp->name = strTok(0);
	recp->lex = strTok(0);
	recp->type = strTok(0);
	if (recp->kind) {
	    recp->domain = strTok(0);
	    recp->range = strTok(0);
	}
	buf = malloc(BSIZE);
    }

    fp = fopen(filename, "w");
    if (!fp) {
	fprintf(stderr, "mkdefs: Could not open %s for writing\n",
		filename);
	exit(1);
    }

    genGuard(filename, guard);
    fprintf(fp, header, guard, guard);
    fputs(prefix, fp);

    for (recp = vals.next; recp; recp = recp->next) {
	if (recp->kind == K_val)
	    lastval = idx;
	else if (recp->kind == K_member)
	    lastmem = idx;
	fprintf(fp, "#define\t%s\t% 5d\n", recp->symbol, idx++);
    }
    idx--;
    fprintf(fp, "\n#define LAST_V %d\n", lastval);
    fprintf(fp, "#define LAST_M %d\n", lastmem);
    fprintf(fp, "#define MINNAME %d\n#define MAXNAME %d\n\n", MINNAME,
	    idx);

    fprintf(fp, "static Exid_t symbols[] = {\n");
    for (recp = vals.next; recp; recp = recp->next) {
	fprintf(fp, "\tEX_ID ( %s, %s, %s, %s, 0),\n",
		recp->name, recp->lex, recp->symbol, recp->type);
    }
    fprintf(fp, "\tEX_ID ( {0}, 0, 0, 0, 0)\n};\n");

    fprintf(fp, "\nstatic char* typenames[] = {\n");
    for (recp = vals.next; recp; recp = recp->next) {
	if (*(recp->symbol) == 'T')
	    fprintf(fp, "\t%s,\n", recp->name);
    }
    fprintf(fp, "};\n");

    fprintf(fp, "\n#ifdef DEBUG\nstatic char* gprnames[] = {\n\t\"\",\n");
    for (recp = vals.next; recp; recp = recp->next) {
	fprintf(fp, "\t\"%s\",\n", recp->symbol);
    }
    fprintf(fp, "};\n#endif\n");

    fprintf(fp, "\ntypedef unsigned short tctype;\n");
    fprintf(fp, "\nstatic tctype tchk[][2] = {\n\t{ 0, 0 },\n");
    for (recp = vals.next; recp; recp = recp->next) {
	if (recp->kind)
	    fprintf(fp, "\t{ %s, %s },\n", recp->domain, recp->range);
    }
    fprintf(fp, "};\n");

    fprintf(fp, "\n#endif\n");
    fclose(fp);

    /* clean up */
    for (recp = vals.next; recp; recp = vals.next) {
      free(recp->data);
      vals.next = recp->next;
      free(recp);
    }
    free(buf);

    exit(0);
}
