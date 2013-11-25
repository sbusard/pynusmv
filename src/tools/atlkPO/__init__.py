"""
This module contains a configuration variable accessible from everywhere
in the ATLK_irF model-checking tool used to tune the behaviour of different
algorithms.
config is the main object containing configuration variables, and these
variables are accessible through attributes of config.
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