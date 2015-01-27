from pynusmv.model import *

class player(Module):
    table = Identifier("table")
    turn = Identifier("turn")
    
    ARGS = [table, turn]
    
    act = IVar(Range(0, 9))

class main(Module):
    table = Var(Array(1, 9, Scalar(("__", "PO", "PX"))))
    turn = Var(Scalar(("PO", "PX")))
    
    pO = Var(player(table, turn))
    pX = Var(player(table, turn))
    
    POwin = Def(((table[1] == "PO") & (table[2] == "PO") & (table[3] == "PO"))|
                ((table[4] == "PO") & (table[5] == "PO") & (table[6] == "PO"))|
                ((table[7] == "PO") & (table[8] == "PO") & (table[9] == "PO"))|
                
                ((table[1] == "PO") & (table[4] == "PO") & (table[7] == "PO"))|
                ((table[2] == "PO") & (table[5] == "PO") & (table[8] == "PO"))|
                ((table[3] == "PO") & (table[6] == "PO") & (table[9] == "PO"))|
                
                ((table[1] == "PO") & (table[5] == "PO") & (table[9] == "PO"))|
                ((table[3] == "PO") & (table[5] == "PO") & (table[7] == "PO")))
    PXwin = Def(((table[1] == "PX") & (table[2] == "PX") & (table[3] == "PX"))|
                ((table[4] == "PX") & (table[5] == "PX") & (table[6] == "PX"))|
                ((table[7] == "PX") & (table[8] == "PX") & (table[9] == "PX"))|
                
                ((table[1] == "PX") & (table[4] == "PX") & (table[7] == "PX"))|
                ((table[2] == "PX") & (table[5] == "PX") & (table[8] == "PX"))|
                ((table[3] == "PX") & (table[6] == "PX") & (table[9] == "PX"))|
                
                ((table[1] == "PX") & (table[5] == "PX") & (table[9] == "PX"))|
                ((table[3] == "PX") & (table[5] == "PX") & (table[7] == "PX")))
    Draw = Def(~POwin & ~PXwin &
               (table[1] != "__") & (table[2] != "__") & (table[3] != "__") &
               (table[4] != "__") & (table[5] != "__") & (table[6] != "__") &
               (table[7] != "__") & (table[8] != "__") & (table[9] != "__"))
    
    INIT = [(table[1] == "__")]
    INIT += [(table[2] == "__")]
    INIT += [(table[3] == "__")]
    INIT += [(table[4] == "__")]
    INIT += [(table[5] == "__")]
    INIT += [(table[6] == "__")]
    INIT += [(table[7] == "__")]
    INIT += [(table[8] == "__")]
    INIT += [(table[9] == "__")]
    INIT += [turn == "PO"]
    
    # Protocol
    TRANS = [(POwin | PXwin | Draw | (turn == "PX")).implies(pO.act == 0)]
    TRANS += [(~(POwin | PXwin | Draw | (turn == "PX"))).implies(pO.act != 0)]
    TRANS += [(table[1] != "__").implies(pO.act != 1)]
    TRANS += [(table[2] != "__").implies(pO.act != 2)]
    TRANS += [(table[3] != "__").implies(pO.act != 3)]
    TRANS += [(table[4] != "__").implies(pO.act != 4)]
    TRANS += [(table[5] != "__").implies(pO.act != 5)]
    TRANS += [(table[6] != "__").implies(pO.act != 6)]
    TRANS += [(table[7] != "__").implies(pO.act != 7)]
    TRANS += [(table[8] != "__").implies(pO.act != 8)]
    TRANS += [(table[9] != "__").implies(pO.act != 9)]
    
    TRANS += [(POwin | PXwin | Draw | (turn == "PO")).implies(pX.act == 0)]
    TRANS += [(~(POwin | PXwin | Draw | (turn == "PO"))).implies(pX.act != 0)]
    TRANS += [(table[1] != "__").implies(pX.act != 1)]
    TRANS += [(table[2] != "__").implies(pX.act != 2)]
    TRANS += [(table[3] != "__").implies(pX.act != 3)]
    TRANS += [(table[4] != "__").implies(pX.act != 4)]
    TRANS += [(table[5] != "__").implies(pX.act != 5)]
    TRANS += [(table[6] != "__").implies(pX.act != 6)]
    TRANS += [(table[7] != "__").implies(pX.act != 7)]
    TRANS += [(table[8] != "__").implies(pX.act != 8)]
    TRANS += [(table[9] != "__").implies(pX.act != 9)]
    
    # Evolution
    TRANS += [(turn == "PO").implies(turn.next() == "PX")]
    TRANS += [(turn == "PX").implies(turn.next() == "PO")]
    
    TRANS += [table[1].next() == Case((((turn == "PO") & (pO.act == 1), "PO"),
                                       ((turn == "PX") & (pX.act == 1), "PX"),
                                       (Trueexp(), table[1])))]
    TRANS += [table[2].next() == Case((((turn == "PO") & (pO.act == 2), "PO"),
                                       ((turn == "PX") & (pX.act == 2), "PX"),
                                       (Trueexp(), table[2])))]
    TRANS += [table[3].next() == Case((((turn == "PO") & (pO.act == 3), "PO"),
                                       ((turn == "PX") & (pX.act == 3), "PX"),
                                       (Trueexp(), table[3])))]
    TRANS += [table[4].next() == Case((((turn == "PO") & (pO.act == 4), "PO"),
                                       ((turn == "PX") & (pX.act == 4), "PX"),
                                       (Trueexp(), table[4])))]
    TRANS += [table[5].next() == Case((((turn == "PO") & (pO.act == 5), "PO"),
                                       ((turn == "PX") & (pX.act == 5), "PX"),
                                       (Trueexp(), table[5])))]
    TRANS += [table[6].next() == Case((((turn == "PO") & (pO.act == 6), "PO"),
                                       ((turn == "PX") & (pX.act == 6), "PX"),
                                       (Trueexp(), table[6])))]
    TRANS += [table[7].next() == Case((((turn == "PO") & (pO.act == 7), "PO"),
                                       ((turn == "PX") & (pX.act == 7), "PX"),
                                       (Trueexp(), table[7])))]
    TRANS += [table[8].next() == Case((((turn == "PO") & (pO.act == 8), "PO"),
                                       ((turn == "PX") & (pX.act == 8), "PX"),
                                       (Trueexp(), table[8])))]
    TRANS += [table[9].next() == Case((((turn == "PO") & (pO.act == 9), "PO"),
                                       ((turn == "PX") & (pX.act == 9), "PX"),
                                       (Trueexp(), table[9])))]


print(player)
print(main)