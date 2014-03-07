from sets import Set

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

class Grammernator:
    def __init__(self, startsym, gram_tree):
        self.gtree = gram_tree
        self.states = None
        self.start_symbol = startsym

    def get_states(self):
        if self.states is not None: return self.states

        # Parse the grammer tree to get all the possibe states
        self.states = Set()

        # Get the fresh of the start, and add it as a state
        closure(getfresh(startsym))

        return self.states

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
        k = closure_recurse(items)

        # Add in the current items
        k |= Set(items)
        return list(k)

    def closure_recurse(self, items):
        k = Set()
        for item in items:
            # Add freshes of the next symbol to the set of items only if it's a non-terminal
            if len(item.future) > 0 and isSymbol(item.future[0]):
                k |= Set(getfresh(item.future[0]))
        if len(k) > 0:
            k |= closure_recurse(list(k))
        return k


    def self.isSymbol(char):
        # It's a symbol if it's a capital
        if ord(char) > 65 and ord(char) < 90:
            return True
        return False
