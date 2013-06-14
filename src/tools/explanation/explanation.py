class Explanation():
    """
    Explanations consistute a graph explaining why a particular state satisfies
    an ATL property.
    
    An Explanation is composed of state and a list of successors reachable
    through inputs.
    """
    def __init__(self, state, successors=None):
        """
        state -- the state of the explanation node;
        successors -- a set of (inputs, successor) pairs where successors
                      are also explanations.
        """
        self.state = state
        self.successors = successors if successors else set()
        
    
    def dot(self):
        """
        Return a dot representation (as a text) of this explanation.
        """
        # Get all states, keep them and mark them
        ids = {}
        curid = 0
        extract = {self}
        while len(extract) > 0:
            expl = extract.pop()
            ids[expl] = "s" + str(curid)
            curid += 1
            for (action, succ) in expl.successors:
                if succ not in ids:
                    extract.add(succ)
                
        dot = "digraph {"
        
        # Add states to the dot representation
        for expl in ids:
            dot += (ids[expl] + " " + "[label=\"" +
                    '\\n'.join(var + "=" + val for var, val in
                                            expl.state.get_str_values().items()) +
                    "\"]" + ";\n")
        
        # For each state, add each transition to the representation
        for expl in ids:
            for action, succ in expl.successors:
                dot += (ids[expl] + "->" + ids[succ] + " " +
                        "[label=\"" + "\\n".join(var + "=" + val for var, val in 
                                        action.get_str_values().items()) + "\"]"
                        + ";\n")
        
        dot += "}"
        
        return dot