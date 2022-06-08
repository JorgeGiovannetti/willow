class Memory:

    def __init__(self):
        self.global_memory = {"int": [], "float": [], "bool": [], "char": [], "string": []}
        self.memory_stack = [{"int": [], "float": [], "bool": [], "char": [], "string": []}]
        self.instruction_pointer = 0
        self.call_stack = []

    def mask_address(internal_address, memory_segment, type_code):
        return internal_address | segmentMask(memory_segment) | typeMask(type_code)
        
    def type_from_address(address):
        return (address & 0xff << 20) >> 20

    def segment_from_address(address):
        return (address & 0xf << 28) >> 28

    def internal_address(address):
        return address & ~(0xfff << 20)

    def segment_mask(segment):
        return scopeKind << 28

    def type_mask(type_code):
        return type_code << 20