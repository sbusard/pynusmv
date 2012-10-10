class LockStack():
    """
    A lock stack is a stack with an additional lock.
    
    When popping, the lock is activated, but the top value is actually removed
    only when the lock is already activated. Whenever pushing a value, the lock
    is deactivated.
    This means that to remove the top, you have to pop twice in a raw
    (without pushing between the two pops).
    The stack can be unlocked.
    """
    
    def __init__(self):
        self.__stack = []
        self.__lock = False
        
    def pop(self):
        if len(self.__stack) <= 0:
            raise IndexError("Empty stack")
        if self.__lock:
            self.__stack.pop()
        else:
            self.__lock = True
            
    def push(self, value):
        self.__stack.append(value)
        self.__lock = False
        
    def top(self):
        if len(self.__stack) <= 0:
            raise IndexError("Empty stack")
        return self.__stack[-1]
        
    def isEmpty(self):
        return len(self.__stack) == 0
        
    def all(self):
        return tuple(self.__stack)
        
    def unlock(self):
        self.__lock = False
        
    def copy(self):
        copy = LockStack()
        for e in self.__stack:
            copy.push(e)
        return copy