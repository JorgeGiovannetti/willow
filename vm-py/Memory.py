class Memory:

    def __init__(self):
        self.global_memory = {"int": [], "float": [], "bool": [], "char": [], "string": []}
        self.memory_stack = [{"int": [], "float": [], "bool": [], "char": [], "string": []}]
        self.instruction_pointer = 0
        self.call_stack = []
    
    def push_memory_stack(self):
        self.memory_stack.append({"int": [], "float": [], "bool": [], "char": [], "string": []})

    def segment_mask(self, segment: int):
        return segment << 28

    def type_mask(self, type_code: int):
        return type_code << 20

    def mask_address(self, internal_address: int, memory_segment: int, type_code: int):
        return internal_address | self.segment_mask(memory_segment) | self.type_mask(type_code)
        
    def type_from_address(self, address):
        return (address & 0xff << 20) >> 20

    def segment_from_address(self, address):
        return (address & 0xf << 28) >> 28

    def internal_address(address):
        return address & ~(0xfff << 20)
