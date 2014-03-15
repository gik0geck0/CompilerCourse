#!/usr/bin/env python3

import sys
from grammernator import Grammernator

def main():
    # Check args
    if len(sys.argv) < 2:
        print("Usage: %s <grammerfile>" % sys.argv[0])
        exit(-1)

    grmntr = Grammernator.loadfromfile(sys.argv[1], 'S')

    if '-table' in sys.argv:
        gtable = grmntr.get_parse_table()
        statemap = {}

        i=0
        for state in gtable:
            print("%i: %s" % (i,str(set(state))))
            statemap[state] = i
            i += 1

        print("",end="\t")
        for inchr in grmntr.alphabet:
            print(inchr, end="\t")

        for state in gtable:
            print(statemap[state], end="\t")
            for inchr in grmntr.alphabet:
                if inchr in gtable[state] and len(gtable[state][inchr]) > 0:
                    print(statemap[gtable[state][inchr]], end="\t")
                else:
                    print("", end="\t")
            print("")

    else:
        # Now, spin off the grammer-inator
        # Assume that 'S' is the starting symbol
        item_states = grmntr.get_states()

        # Display the item states
        print("Item States:")
        for item in item_states:
            print(set(item))

if __name__ == '__main__':
    main()
