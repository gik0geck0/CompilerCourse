#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0

char *strdup(const char *s);

extern char* yytext;

// Trie: Able to store a value for words and partial words
#define trieLen 127
typedef struct _trie {
    int count;
    struct _trie* tries[trieLen];
} trie;
void freeTrie(trie* tr);
trie* newTrie();
trie* getTrie(trie* tr, char c);
trie* getTrieWithAlloc(trie* tr, char c);
trie* getTrieStr(trie* tr, char* str);
void addToTrie(int adder, char* str, trie* tr);
void printTrie(trie* tr);
void __printTrie(trie* tr, int indents);

// Basically just a tuple of String and Int
typedef struct wctuple {
    char* str;
    int val;
} wctuple;
wctuple* toList(trie* tr, int* sz);
wctuple* __toList(trie* tr, int* cntL, char* prefix);

void moveRight(wctuple* list, int rightOf, int size) {
    // move things to the right from the left until reaching the index
    int i = size-1; // size vs index
    if (list[size-1].str != NULL) {
        // Free up the rightmost word that's about to be forgotten about
        free(list[size-1].str);
    }
    while(i > rightOf) {
        list[i].val = list[i-1].val;
        list[i].str = list[i-1].str;
        --i;
    }
}

int isInStringArray(char** words, char* val) {
    // words and val are NULL terminated
    while(*words != NULL) {
        if (strcmp(*words, val) == 0)
            return 1;
        words += 1;
    }
    return 0;
}

void token_handler(void) {
    // Open the ignore file
    int iwmaxsize = 16;
    int iwsize = 0;
    char ** ignoreWords = calloc(16, sizeof(char*));
    FILE* fp = fopen("IGNORE", "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: IGNORE\n");
        exit(EXIT_FAILURE);
    }

    char* iWord = calloc(255, sizeof(char*));
    while(!feof(fp) && fscanf(fp, "%s", iWord)) {
        if (iwsize < iwmaxsize-1) {
            // Resize the array if we need more space
            char** newiw = calloc(iwmaxsize*2+1, sizeof(char*));
            memcpy(newiw, ignoreWords, iwsize*sizeof(char*));
            free(ignoreWords);
            ignoreWords = newiw;
        }
        // Note: iwsize is the index to the next available free space (size vs index)
        ignoreWords[iwsize] = (char*) strdup(iWord);
        iwsize++;
    }
    free(iWord);
    fclose(fp);
    // ignoreWords now holds all the words to ignore

    // Allocate the trie
    trie* wordCounts = newTrie();

	unsigned int t;
	while(( t = yylex()) ) {
        // Check against the IGNORE list
        if (!isInStringArray(ignoreWords, yytext)) {
            // Add 1 to this word's trie
            addToTrie(1, yytext, wordCounts);
        }
	}

    if (DEBUG) {
        fprintf(stdout, "Trie is now:\n");
        printTrie(wordCounts);
    }

    // Now, find the most occuring words
    int* sz = calloc(1, sizeof(int));
    wctuple* fq = toList(wordCounts, sz);

    // Print all the most occuring
    for (int i = 0; i < *sz; i++) {
        fprintf(stdout, "#%i: %s (%i)\n", i, fq[i].str, fq[i].val);
    }

    // Frees
    for (int i = 0; i < *sz; i++) {
        free(fq[i].str);
    }
    free(fq);
    free(sz);

    for (int i=0; i < iwsize; i++) {
        free(ignoreWords[i]);
    }
    free(ignoreWords);

    freeTrie(wordCounts);

    // Really lex? You really did this?
    free(yytext);
}

void freeTrie(trie* tr) {
    for (int i=0; i < trieLen; i++) {
        if (tr->tries[i] != NULL) {
            freeTrie(tr->tries[i]);
        }
    }
    free(tr);
}

trie* newTrie() {
    trie* nt = (trie*) calloc(1, sizeof(trie));
    nt->count = 0;
    return nt;
}

trie* getTrieWithAlloc(trie* tr, char c) {
    trie* rtc = tr->tries[(int)c];
    if (rtc == NULL) {
        tr->tries[(int)c] = newTrie();
        rtc = tr->tries[(int)c];
    }
    return rtc;
}

trie* getTrie(trie* tr, char c) {
    return tr->tries[(int)c];
}

trie* getTrieStr(trie* tr, char* str) {
    if (strlen(str) >= 1) {
        // Recurse on the rest of the string
        char first = str[0];
        getTrieStr(getTrieWithAlloc(tr, first), (str+1));
    } else {
        return tr;
    }
}

void addToTrie(int adder, char* str, trie* tr) {
    trie* rtr = getTrieStr(tr, str);
    if (rtr != NULL) {
        rtr->count += 1;
    }
}

void printTrie(trie* tr) {
    __printTrie(tr, 0);
}

void __printTrie(trie* tr, int indents) {
    for (int i=0; i < trieLen; i++) {
        trie* rtr = getTrie(tr, (char) i);
        if (rtr != NULL) {
            for (int j=0; j < indents; j++) {
                fprintf(stdout, "\t");
            }
            fprintf(stdout, "%c: %i\n", (char) i, rtr->count);
            __printTrie(rtr, indents + 1);
        }
    }
}

wctuple* toList(trie* tr, int* sz) {
    return __toList(tr, sz, NULL);
}

wctuple* __toList(trie* tr, int* cntL, char* prefix) {
    wctuple* mlist;
    *cntL = 0;

    for (int i=0; i < trieLen; i++) {
        trie* rtr = getTrie(tr, (char) i);
        if (rtr != NULL) {
            char* mprefix;
            if (prefix != NULL) {
                int slp = strlen(prefix);
                mprefix = calloc(slp+2, sizeof(char));
                memcpy(mprefix, prefix, slp*sizeof(char));
                mprefix[slp] = (char) i;
            } else {
                mprefix = calloc(2, sizeof(char));
                mprefix[0] = (char) i;
            }

            // mprefix now contains the cumulative word

            if (rtr->count > 0) {
                // Add to the list. This is a word

                if (*cntL == 0) {
                    // Freshly allocate
                    mlist = malloc(sizeof(wctuple));
                    mlist[0].val = rtr->count;
                    mlist[0].str = strdup(mprefix);
                    ++(*cntL);
                    if (DEBUG) {
                        fprintf(stdout, "Starter: %s => %i\n", mlist[0].str, mlist[0].val);
                    }
                } else {
                    // Expand the list (inefficient with copying, I know)
                    wctuple* nl = malloc(sizeof(wctuple)* (*cntL + 1));

                    // Copy everything bigger into the array
                    int j = 0;
                    while (j < *cntL && mlist[j].val > rtr->count) {
                        nl[j] = mlist[j];
                        ++j;
                    }
                    // Add self
                    nl[j].val = rtr->count;
                    nl[j].str = strdup(mprefix);
                    ++j;

                    // Copy everything smaller
                    while (j < *cntL + 1) {
                        nl[j] = mlist[j-1];
                        ++j;
                    }

                    if (*cntL > 0) {
                        free(mlist);
                    }
                    ++(*cntL);
                    mlist = nl;

                    if (DEBUG) {
                        fprintf(stdout, "List was added to:\n");
                        for (int k = 0; k < *cntL; k++) {
                            fprintf(stdout, "#%i: %s => %i\n", k, mlist[k].str, mlist[k].val);
                        }
                    }
                }
            }

            // Descend on the remaining tries
            int* retSize = malloc(sizeof(int));
            wctuple* recList = __toList(rtr, retSize, mprefix);
            // fprintf(stdout, "Got an array of size %i\n", *retSize);

            // Merge results
            // Allocate space for both
            wctuple* merged = malloc(sizeof(wctuple)*(*retSize + *cntL));
            int aidx = 0;
            int bidx = 0;

            for (int j=0; j < *retSize + *cntL; j++) {
                if (DEBUG) {
                    fprintf(stdout, "Choosing merge index %i. Choices:\n", j);
                    if (aidx < *cntL)
                        fprintf(stdout, "\tmlist[%i] = %s => %i\n", aidx, mlist[aidx].str, mlist[aidx].val);
                    if (bidx < *retSize)
                        fprintf(stdout, "\trecList[%i] = %s => %i\n", bidx, recList[bidx].str, recList[bidx].val);
                }

                // Add the next largest
                //  only if available
                //                    if we only have A, short circuit
                //                                       We also need to be greater than the choice in B
                if (aidx < *cntL  && (bidx >= *retSize || mlist[aidx].val > recList[bidx].val) ) {
                    // Add from mlist
                    merged[j] = mlist[aidx];
                    ++aidx;
                } else if (bidx < *retSize && (aidx >= *cntL || recList[bidx].val > mlist[aidx].val)) {
                    merged[j] = recList[bidx];
                    ++bidx;
                } else if (aidx < *cntL && bidx < *retSize) {
                    // Decide based on string comparison
                    if (strcmp(mlist[aidx].str, recList[bidx].str) <= 0) {
                        merged[j] = mlist[aidx];
                        ++aidx;
                    } else {
                        merged[j] = recList[bidx];
                        ++bidx;
                    }
                } else {
                    // Ran out of vals before done merging
                    fprintf(stderr, "Ran out of vals before done merging. Need for %i. A: %i/%i B: %i/%i\n", j, aidx, *cntL, bidx, *retSize);
                }
            }

            if (DEBUG) {
                fprintf(stdout, "Merged list for up to %s:\n", mprefix);
                for (int k = 0; k < *cntL + *retSize; k++) {
                    fprintf(stdout, "#%i: %s => %i\n", k, merged[k].str, merged[k].val);
                }
            }

            // Free the spares, and save it as the current
            if (*cntL > 0) {
                free(mlist);
            }

            if (*retSize > 0) {
                // fprintf(stdout, "Freeing an array that should be of size %i\n", *retSize);
                free(recList);
            }
            mlist = merged;
            *cntL = *retSize + *cntL;
            free(retSize);

            free(mprefix);
        }
    }

    // fprintf(stdout, "Returning an array of size %i\n", *cntL);
    return mlist;
}
