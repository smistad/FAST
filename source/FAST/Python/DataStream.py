class DataStream:
    def __init__(self, *args):
        self._processObjects = args;
        self._ports = []
        self.finished = False
        self._executeToken = 0
        for po in self._processObjects:
            for i in range(po.getNrOfOutputPorts()):
                self._ports.append(po.getOutputPort(i))
        if len(self._ports) == 0:
            raise Exception('Can\'t create data stream (getDataStream) on a process object with 0 output ports')
    def __iter__(self):
        return self
    def __next__(self):
        if self.finished:
            raise StopIteration

        for port in self._ports:
            port.getProcessObject().run(self._executeToken)
        self._executeToken += 1
        if self._executeToken < 0:
            self._executeToken = 0
        result = []
        for port in self._ports:
            data = port.getNextFrame()
            if data.isLastFrame(): self.finished = True
            result.append(data)
        if len(result) == 1:
            return result[0]
        else:
            return result
