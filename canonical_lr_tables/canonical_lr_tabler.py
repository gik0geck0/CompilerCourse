#!/usr/bin/env python3

import sys
import grammernator

def main():
    # Check args
    if len(sys.argv) < 2:
        print("Usage: %s <grammerfile>" % sys.argv[0])
        exit(-1)

    # Open the grammer file
    grammer_file = open(sys.argv[1], 'r')

    # Read the rules into the grammer tree
    # Grammer tree :: RuleChar -> [Production]
    grammer_tree = {}

    rule_char = ''
    for line in grammer_file:

        # Remove whitespace at the beginning and the end
        line = line.strip()

        # Remove whitespace inbetween characters
        line = line.replace(" ", "")
        line = line.replace("\t", "")

        # Are we looking at S -> A
        #   or...              | B
        if line[0] == '|':
            # Add this production to the last rule_char's list of productions
            #                          Doing the split | makes it so that each possible rule on the same line is added separately
            grammer_tree[rule_char] += line[1:].split('|')
        else:
            # Change the rule character
            rule_char = line[0]

            # Check if it's a new rule char
            if rule_char not in grammer_tree:
                grammer_tree[rule_char] = []

            # Add the production, skipping the ->
            grammer_tree[rule_char] += line[3:].split('|')

    grammer_file.close()

    # Print the read-in grammer
    for rule in grammer_tree:
        print("%s -> %s" % (rule, str(grammer_tree[rule])))

    # Now, spin off the grammer-inator
    # Assume that 'S' is the starting symbol
    item_states = Grammernator(grammer_tree, 'S').get_states()

    # Display the item states
    map(print, item_states)

if __name__ == '__main__':
    main()
