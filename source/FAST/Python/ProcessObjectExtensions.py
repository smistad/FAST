@classmethod
def create(cls, *args, **kwargs):
    instance = cls(*args, **kwargs)
    return instance

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
