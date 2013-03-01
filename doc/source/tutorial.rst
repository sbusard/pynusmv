.. _tutorial:

Getting started
***************

This page presents a short overview of PyNuSMV capabilities with a small example.
Let's consider the following SMV model. This model is composed of two counters, incrementing from 0 to 3, and looping. They run asynchronously and the running one is defined at each step by the ``run`` action.

.. include:: models/counters.smv
    :literal:


Considering that the model is saved in the ``counters.smv`` file in the current directory, we can now run Python. 
The following Python session shows the basics of PyNuSMV. After importing pynusmv, the function :func:`init_nusmv <pynusmv.init.init_nusmv>` **must** be called before calling any other PyNuSMV functionality. The function :func:`deinit_nusmv <pynusmv.init.deinit_nusmv>` must also be called after using PyNuSMV to release all resources hold by NuSMV. After initializing PyNuSMV, the model is read with the function :func:`load_from_file <pynusmv.glob.load_from_file>` and the model is computed, that is, flattened and encoded into BDDs, with the function :func:`compute_model <pynusmv.glob.compute_model>`.


>>> import pynusmv
>>> pynusmv.init.init_nusmv()
>>> pynusmv.glob.load_from_file("counters.smv")
>>> pynusmv.glob.compute_model()
>>> pynusmv.init.deinit_nusmv()


The next Python session shows functionalities of FSMs, access to specifications of the model, calls to CTL model checking and manipulation of BDDs. First, NuSMV is initialized and the model is read. Then the model encoded with BDDs is retrieved from the main propositions database. The first (and only) proposition is then retrieved from the same database, and the specification of this proposition is isolated.

From the BDD-encoded FSM ``fsm`` and the specification ``spec``, we call the :func:`eval_ctl_spec <pynusmv.mc.eval_ctl_spec>` function to get all the states of ``fsm`` satisfying ``spec``. Conjuncted with the set of reachables states of the model, we get ``bdd``, a BDD representing all the reachable states of ``fsm`` satisfying ``spec``. Finally, from this BDD we extract all the single states and display them, that is, we display, for each of them, the value of each state variable of the model.


>>> import pynusmv
>>> pynusmv.init.init_nusmv()
>>> pynusmv.glob.load_from_file("counters.smv")
>>> pynusmv.glob.compute_model()
>>> fsm = pynusmv.glob.prop_database().master.bddFsm
>>> fsm
<pynusmv.fsm.BddFsm object at 0x1016d9e90>
>>> prop = pynusmv.glob.prop_database()[0]
>>> prop
<pynusmv.prop.Prop object at 0x101770250>
>>> spec = prop.expr
>>> print(spec)
AF c1.c = stop - 1
>>> bdd = pynusmv.mc.eval_ctl_spec(fsm, spec) & fsm.reachable_states
>>> bdd
<pynusmv.dd.BDD object at 0x101765a90>
>>> satstates = fsm.pick_all_states(bdd)
>>> for state in satstates:
...     print(state.get_str_values())
... 
{'c1.c': '2', 'c2.c': '2', 'stop': '3', 'start': '0'}
{'c1.c': '2', 'c2.c': '0', 'stop': '3', 'start': '0'}
{'c1.c': '2', 'c2.c': '1', 'stop': '3', 'start': '0'}
>>> pynusmv.init.deinit_nusmv()


This (very) short tutorial showed the main functionalities of PyNuSMV. More of them are available (to parse and evaluate a simple expression, to build new CTL specifications or to perform operations on BDDs); the :ref:`full reference <pynusmv-api>` of the library is given beside this tutorial.
