#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
wctuple* getMaxCounts(trie* tr, int topN);
wctuple* __getMaxCounts(trie* tr, wctuple* maxes, int size, char* prefix);

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
    // fprintf(stdout, "Trie is now:\n");
    // printTrie(wordCounts);

    // Now, find the most occuring words
    fprintf(stdout, "Looking for most frequent words\n");
    int max = 10;
    wctuple* fq = getMaxCounts(wordCounts, max);
    for (int i = 0; i < max; i++) {
        fprintf(stdout, "#%i: %s (%i)\n", i, fq[i].str, fq[i].val);
    }

    // Frees
    for (int i = 0; i < max; i++) {
        // free(fq[i].str);
    }
    free(fq);

    for (int i=0; i < iwsize; i++) {
        free(ignoreWords[i]);
    }
    free(ignoreWords);

    freeTrie(wordCounts);
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
            printTrie(rtr, indents + 1);
        }
    }
}

wctuple* toList(trie* tr, int* sz) {
    return __toList(tr, sz, NULL);
}

wctuple* __toList(trie* tr, int* cntL, char* prefix) {
    wctuple* mlist;
    int lsize = 0;

    for (int i=0; i < trieLen; i++) {
        trie* rtr = getTrie(tr, (char) i);
        if (rtr != NULL) {
            char* mprefix;
            if (prefix != NULL) {
                int slp = strlen(prefix);
                mprefix = calloc(slp+2, sizeof(char));
                strcpy(mprefix, prefix, slp);
                mprefix[slp] = (char) i;
            } else if {
                mprefix = calloc(2, sizeof(char));
                mprefix[0] = (char) i;
            }

            // mprefix now contains the cumulative word

            if (rtr->count > 0) {
                // Add to the list. This is a word

                if (lsize == 0) {
                    // Freshly allocate
                    mlist = malloc(sizeof(wctuple));
                    mlist[0].val = rtr->count;
                    mlist[0].str = strdup(mprefix);
                } else {
                    // Expand the list (inefficient with copying, I know)
                    wctuple* nl = malloc(sizeof(wctuple));

                    // Copy everything bigger into the array
                    int i = 0;
                    while (i < lsize && mlist[i].val > rtr->count) {
                        nl[i] = mlist[i];
                        ++i;
                    }
                    // Add self
                    nl[i].val = rtr->count;
                    nl[i].str = strdup(mprefix);
                    ++i;

                    // Copy everything smaller
                    while (i < lsize) {
                        nl[i] = mlist[i-1];
                        ++i;
                    }
                    free(mlist);
                    mlist = nl;
                }
            }

            // Descend on the remaining tries
            int* retSize = malloc(sizeof(int));
            wctuple* recList = __toList(rtr, retSize, mprefix);

            // Merge results
            // Allocate space for both
            wctuple* merged = malloc(sizeof(wctuple)*(*retSize + lsize));
            int aidx = 0;
            int bidx = 0;

            for (int j=0; j < *retSize + lsize; j++) {
                // Add the next largest
                if (aidx < lsize  && mlist[lsize]  ) {
                    // Add from mlist
                    merged[j] = mlist[aidx];
                    ++aidx;
                } else if (bidx < *retSize) {
                    merged[j] = recList[bidx];
                    ++bidx;
                } else {
                    // Ran out of vals before done merging
                    fprintf(stderr, "Ran out of vals before done merging. Need for %i. A: %i/%i B: %i/%i\n", j, aidx, lsize, bidx, *recSize);
                }
            }

            // Free the spares, and save it as the current
            free(mlist);
            free(recList);
            mlist = merged;
            lsize = *retSize + lsize;
            free(retSize);
        }
    }

    *cntL = lsize;
    return mList;
}


wctuple* getMaxCounts(trie* tr, int topN);
wctuple* __getMaxCounts(trie* tr, wctuple* maxes, int size, char* prefix);
