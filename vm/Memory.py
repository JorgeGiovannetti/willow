class Memory:

    def __init__(self):
        self.global_memory = {'int': [], 'float': [], 'bool': [], 'char': [], 'string': []}
        self.type_names = ['int', 'float', 'bool', 'char', 'string']
        self.segment_names = ['global', 'local', 'temp']
        self.instruction_pointer = 0
        self.stack_pointer = 0
        self.call_stack = []
        self.memory_stack = [[self.init_nonglobal_memory(),self.init_nonglobal_memory()]]

    def init_nonglobal_memory(self):
        return {type_name : [] for type_name in self.type_names}

    def type_default_value(self, type_name: str):
        if type_name == 'int':
            return 0
        if type_name == 'float':
            return 0.0
        if type_name == 'bool':
            return False
        if type_name == 'char' or type_name == 'string':
            return ''

    def extend_memory(self, memory_segment, type_name, internal_address):
        while len(memory_segment[type_name]) <= internal_address:
            memory_segment[type_name].append(self.type_default_value(type_name))

    def assign_to_address(self, data, address: str):
        address = self.get_address(address)
        segment = self.segment_from_address(address)
        data_internal_address = self.internal_address(address)
        type_name = self.type_names[self.type_from_address(address)]

        if self.segment_names[segment] == 'global':
            self.extend_memory(self.global_memory, type_name, data_internal_address)
            self.global_memory[type_name][data_internal_address] = data
        else:
            self.extend_memory(self.memory_stack[self.stack_pointer][segment-1], type_name, data_internal_address)
            self.memory_stack[self.stack_pointer][segment-1][type_name][data_internal_address] = data
    
    def get_data_from_address(self, address: str):
        address = self.get_address(address)
        segment = self.segment_from_address(address)
        data_internal_address = self.internal_address(address)
        type_name = self.type_names[self.type_from_address(address)]

        if self.segment_names[segment] == 'global':
            self.extend_memory(self.global_memory, type_name, data_internal_address)
            return self.global_memory[type_name][data_internal_address]
        else:
            self.extend_memory(self.memory_stack[self.stack_pointer][segment-1], type_name, data_internal_address)
            return self.memory_stack[self.stack_pointer][segment-1][type_name][data_internal_address]

    def get_address(self, address: str):
        return int(address[1:])
    
    def push_memory_stack(self):
        self.memory_stack.append({'int': [], 'float': [], 'bool': [], 'char': [], 'string': []})

    def segment_mask(self, segment: int):
        return segment << 29

    def type_mask(self, type_code: int):
        return type_code << 21

    def mask_address(self, internal_address: int, memory_segment: int, type_code: int):
        return internal_address | self.segment_mask(memory_segment) | self.type_mask(type_code)
        
    def type_from_address(self, address: int):
        return (address & 0xff << 21) >> 21

    def typename_from_address(self, address: str):
        return self.type_names[self.type_from_address(address)]

    def segment_from_address(self, address: int):
        return (address & 0x3 << 29) >> 29

    def internal_address(self, address: int):
        return address & (0xfffff)
    
    def is_pointer(self, address: int):
        return address & (1 << 31) == 1
