import sys

from pynusmv.prop import PropDb
from pynusmv.prop import propTypes
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.fsm import BddFsm
from pynusmv.exception import PyNuSMVError

from .eval import eval_ctl

def check(modelPath, evalSpecs=True):
    init_nusmv()
    # Initialize the model
    fsm = BddFsm.from_filename(modelPath)
    propDb = glob.prop_database()

    # Check all CTL properties
    if evalSpecs:
        for prop in propDb:
            #  Check type
            if prop.type == propTypes['CTL']:
                spec = prop.exprcore

                # Get violating states
                violating = (fsm.init & ~eval_ctl(fsm, spec) &
                             fsm.state_constraints)
                print('Specification',str(spec), 'is',
                      str(violating.is_false()))
                # We could generate counter-examples here
    deinit_nusmv()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("[ERROR] Missing model.")
    else:
        try:
            check(sys.argv[-1], not "-dcx" in sys.argv)
        except PyNuSMVError as e:
            print("[Error]", str(e))