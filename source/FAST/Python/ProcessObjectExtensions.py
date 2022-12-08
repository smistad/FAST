@classmethod
def create(cls, *args, **kwargs):
    instance = cls(*args, **kwargs)
    # This is hack for solving the problem with PythonProcessObjects being deleted too early
    # For instance in this case:
    # a = SomePythonPO.create()
    # b = SomeOtherPO.create().connect(a)
    # a = SomeOtherPO.create().connect(b) # Variable is overwritten, and gets deleted here, even though b has a reference to it..
    # This hack will however result in the PO never being deleted..
    # See section 32.5.3 here: https://swig.org/Doc4.0/Python.html
    return instance.__disown__() # Returns a weak reference


def getInputData(self, portID:int=0):
    val = self._getInputData(portID)
    className = val.getNameOfClass()
    val = eval(className + '.fromDataObject')(val)
    return val


def connect(self, *args):
    inputPortID = 0
    outputPortID = 0
    if len(args) == 1:
        parentProcessObject = args[0]
    elif len(args) == 2:
        if isinstance(args[0], int):
            inputPortID = int(args[0])
            parentProcessObject = args[1]
        else:
            parentProcessObject = args[0]
            outputPortID = int(args[1])
    elif len(args) == 3:
        inputPortID = int(args[0])
        parentProcessObject = args[1]
        outputPortID = int(args[2])
    else:
        raise TypeError("Incorrect arguemnts to connect()")

    super().connect(inputPortID, parentProcessObject, outputPortID)
    return self
