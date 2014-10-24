"""
This module contains a configuration variable accessible from everywhere
in the ATLK_irF model-checking tool used to tune the behaviour of different
algorithms.
config is the main object containing configuration variables, and these
variables are accessible through attributes of config.


The list of parameters follows. Every indentation is a sub-parameter;
for example, config.partial.early.type is type under early under partial.
Top-level parameters are sub-parameters of config.

 debug (boolean): printing or not debugging information
 partial (set of parameters): set of parameters for partial strategies
    early: set of parameters for early termination of partial strategies
        type (string): type of early termination
            - None: no early termination
            - full: stop when all Z states are in sat
            - partial: whenever sat grows, recompute strategies for the rest
            - threshold: when remaining states decrease of threshold,
                         recompute the strategies
        threshold (int): the threshold for early termination with threshold
    caching (boolean): whether or not caching states satisfying or not
                       sub-formulas
    filtering (boolean): activate filtering
    separation: separating Z states into sub-sets to check one at a time
        type (string): type of separation
            - None: no separation
            - random: pick a random state in Z and take its equivalence class
            - reach: pick first reachable state from initial ones and take
                     its equivalence class
    garbage: explicit call to garbage collection
        type (string): type to garbage collection calls
            - None: no explicit call to garbage collection
            - each: collect after every strategy
            - step: collect every step strategies
        step (int): number of strategies between each collection when type is
                    step
"""

class AttrDict(dict):
    def __init__(self, *args, **kwargs):
        super(AttrDict, self).__init__(*args, **kwargs)
        self.__dict__ = self

config = AttrDict()



# -------------
# General stuff
# -------------

# Whether or not printing debug information at console
config.debug = False

# Garbage collection
# ------------------

config.garbage = AttrDict()

# Which technique to use
#   None: garbage collection is never called explicitely
#   each: garbage collection is called after each strategy
#   step: garbage collection is called after the corresponding number of 
#         strategies
config.garbage.type = None

# Number of strategies to check before calling again garbage collection
config.garbage.step = 100



# ---------------------------------------------------
# Partial strategies implementation related variables
# ---------------------------------------------------
config.partial = AttrDict()


# Early termination in partial strategies
# ---------------------------------------
config.partial.early = AttrDict()
# Early termination type used
#   None: early termination not used
#   full: when all the states satisfy the property
#   partial: whenever sat grows, recompute the strategies with remaining states
#   threshold: when remaining states decreases below the given threshold, 
#              recompute the strategies for these remaining states
config.partial.early.type = None
# Early termination recomputation threshold: what is the factor between
# current remaining below which we have to recompute the strategies for 
# remaining states
config.partial.early.threshold = 0.75


# Caching subformulas in partial strategies
# -----------------------------------------

# Whether or not perform caching through accumulation of truth values
config.partial.caching = False


# Pre-filtering out losing moves
# ------------------------------

# Whether or not pre-filter losing moves
# before splitting into partial strategies
config.partial.filtering = False


# States set separation
# ---------------------

config.partial.separation = AttrDict()

# Which technique to use
#   None: states set separation not used
#   random: a random state is picked, its equivalence class is chosen
#   reach: the first reached state, from initial states,
#          and its equivalence class are chosen
config.partial.separation.type = None


# Alternating variant:
# --------------------

config.partial.alternate = AttrDict()

# Which technique to use for alternation
#   univ: perform universal filtering
#   strat: perform full-observability-based filtering
# This parameter should contain at least one of the two values
config.partial.alternate.type = {"univ", "strat"}