
# TODO: What's needed of the State class?
class State:
    def __init__(self, ilist):
        pass

class Item:
    def __init__(self, parent, rule):
        # ASCII US (Unit Separator) is our dot
        divided = rule.split(chr(31))
        if len(divided) is not 2:
            print("Cannot make an item without a 'dot'. String was: %s" % rule)

        self.past = divided[0]
        self.future = divided[1]
        self.parent = parent

    def __str__(self):
        return self.past + u'\u2022' + self.future

    def __repr__(self):
        return self.parent + "->" + str(self)

    def __attrs(self):
        return (self.parent, self.past, self.future)

    def __eq__(self, other):
        return isinstance(other, Item) and self.__attrs() == other.__attrs()

    # Signifies uniqueness in a set
    def __hash__(self):
        return hash(self.__attrs())

class Grammernator:
    @staticmethod
    def isSymbol(char):
        # It's a symbol if it's a capital
        if ord(char) >= 65 and ord(char) <= 90:
            return True
        return False

    @staticmethod
    def loadfromfile(fname, startsym):
        # Open the grammer file
        grammer_file = open(fname, 'r')

        # Read the rules into the grammer tree
        # Grammer tree :: RuleChar -> [Production]
        grammer_tree = {}

        rule_char = ''
        for line in grammer_file:

            # Remove whitespace at the beginning and the end
            line = line.strip()

            # Reduce lambda to a single character to make it easier to deal with
            # Note: ANY occurance of "lambda" is reduced,
            #   so the actual word "lambda" is disallowed
            line = line.replace("lambda", "\u03BB")

            # Remove whitespace inbetween characters
            line = line.replace(" ", "")
            line = line.replace("\t", "")

            if len(line) == 0:
                continue

            # Are we looking at S -> A
            #   or...              | B
            if line[0] == '|':
                # Add this production to the last rule_char's list of productions
                #                           Doing the split | makes it so that each
                #                           possible rule on the same line is added
                #                           separately
                grammer_tree[rule_char] += line[1:].split('|')
            elif line[0] == '#':
                # Use '#' as comment lines
                pass
            else:
                # Change the rule character
                rule_char = line[0]

                # Check if it's a new rule char
                if rule_char not in grammer_tree:
                    grammer_tree[rule_char] = []

                # Add the production, skipping the ->
                grammer_tree[rule_char] += line[3:].split('|')

        grammer_file.close()

        return Grammernator(grammer_tree, startsym)

    def __init__(self, gram_tree, startsym):
        self.gtree = gram_tree
        self.states = None
        self.start_symbol = startsym
        self.alphabet = set()
        for prod in self.gtree:
            self.alphabet.add(prod)
            for rule in self.gtree[prod]:
                for symbol in rule:
                    # Do not consider lambdas as alphabet characters
                    if symbol != '\u03BB':
                        self.alphabet.add(symbol)

    def get_parse_table(self):
        # State -> Dict( symbol -> State )
        gtable = {}
        # There's a double-calc of all the gotos... but im too lazy to fix it
        for state in self.get_states():
            gtable[state] = {}
            for symbol in self.alphabet:
                gtable[state][symbol] = self.goto(state, symbol)
        return gtable

    def get_states(self):
        if self.states is not None: return self.states

        # Parse the grammer tree to get all the possible states
        # This is a set of sets of states.
        # Each item-set is a state
        # self.states :: [[Item]]
        self.states = set()

        # Get the fresh of the start, and add it as a state
        self.states.add(self.closure(self.getfresh(self.start_symbol)))

        # Recursively run through go-tos until there is no change
        self.states = self.get_states_recurse(self.states)

        return self.states

    # returns states with additions added. If this is the end of the line,
    #   then states will be unchanged
    # It probably does pass by reference, so passed in set will likely be changed
    def get_states_recurse(self, states):
        # For each state, calculate all the gotos, and if it's a new state, add it
        following_unique_states = set()
        for state in states:
            for symbol in self.alphabet:
                gt = self.goto(state, symbol)
                if len(gt) != 0 and gt not in states:
                    following_unique_states.add(gt)

        # Recurse if there are new additions
        if len(following_unique_states) > 0:
            states |= self.get_states_recurse(following_unique_states)

        return states

    # getfresh :: char -> [Item]
    def getfresh(self, symbol):
        freshes = []
        # Create a list of items for the fresh of the provided symbol
        for prod in self.gtree[symbol]:
            freshes.append(Item(symbol, str(chr(31)) + prod))
        return freshes

    # closure :: [Item] -> [Item]
    def closure(self, items):
        # Get all the closures for the next item
        k = self.closure_recurse(items)

        # Add in the current items
        k |= set(items)
        return frozenset(k)

    def closure_recurse(self, items):
        k = set()
        for item in items:
            # Add freshes of the next symbol to the set of items only if it's a
            #   non-terminal
            if len(item.future) > 0 and self.isSymbol(item.future[0]):
                next_set = set(self.getfresh(item.future[0]))
                k |= next_set

                derives_to_lambda = False
                for ni in next_set:
                    # If -> dot lambda
                    # NOTE: It's expected that there will be NOTHING after lambda
                    if len(ni.future) == 1 and ni.future[0] == '\u03BB':
                        #print(
                        derives_to_lambda = True
                        break

                # The or checks if the next symbol is lambda,
                #   and there are symbols after, but this should NEVER happen
                if len(item.future) > 1 and (derives_to_lambda or item.future[0] == '\u03BB'):
                    k |= set(self.getfresh(item.future[1]))

        if len(k) > 0:
            k |= self.closure_recurse(k)
        return k

    # goto :: [Item] -> Symbol -> [Item]
    def goto(self, items, symbol):
        # Calculate T (items with the dot moving one right, past the input symbol)
        has_move = set()
        for item in items:
            if len(item.future) > 0 and item.future[0] == symbol:
                # Add this rule to T, but with the dot moved to the right one symbol
                #   (That symbol == goto(_, symbol) )
                has_move.add(Item(item.parent, item.past + item.future[0] + chr(31) + item.future[1:]))

        # Return the closure of that
        return self.closure(has_move)

