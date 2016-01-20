"""
PyNuSMV is a Python framework for experimenting and prototyping BDD-based model
checking algorithms based on NuSMV. It gives access to main BDD-related NuSMV
functionalities, like model and BDD manipulation, while hiding NuSMV
implementation details by providing wrappers to NuSMV functions and data
structures. In particular, NuSMV models can be read, parsed and compiled,
giving full access to SMV's rich modeling language and vast collection of
existing models.

PyNuSMV is composed of several modules, each one proposing some
functionalities:

* :mod:`init <pynusmv.init>` contains all the functions needed to initialize
  and close NuSMV. These functions need to be used before any other access to
  PyNuSMV.
* :mod:`glob <pynusmv.glob>` provides functionalities to read and build a model
  from an SMV source file.
* :mod:`model <pynusmv.model>` provides functionalities to define NuSMV models
  in Python.
* :mod:`node <pynusmv.node>` provides a wrapper to NuSMV `node` structures.
* :mod:`fsm <pynusmv.fsm>` contains all the FSM-related structures like
  BDD-represented FSM, BDD-represented transition relation, BDD encoding and
  symbols table.
* :mod:`prop <pynusmv.prop>` defines structures related to propositions of a
  model; this includes CTL specifications.
* :mod:`dd <pynusmv.dd>` provides BDD-related structures like generic BDD,
  lists of BDDs and BDD-represented states, input values and cubes.
* :mod:`parser <pynusmv.parser>` gives access to NuSMV parser to parse simple
  expressions of the SMV language.
* :mod:`mc <pynusmv.mc>` contains model checking features.
* :mod:`exception <pynusmv.exception>` groups all the PyNuSMV-related
  exceptions.
* :mod:`utils <pynusmv.utils>` contains some side functionalities.

.. WARNING:: Before using any PyNuSMV functionality, make sure to call
   :func:`init_nusmv <pynusmv.init.init_nusmv>` function to initialize NuSMV;
   do not forget to also call :func:`deinit_nusmv <pynusmv.init.deinit_nusmv>`
   when you do not need PyNuSMV anymore to clean everything needed by NuSMV to
   run.

"""

__all__ = ['dd', 'exception', 'fsm', 'glob', 'init', 'mc', 'nusmv', 'parser',
           'prop', 'utils', 'model', 'node']

from . import dd
from . import fsm
from . import glob
from . import init
from . import mc
from . import parser
from . import prop
from . import utils
from . import model
from . import node
