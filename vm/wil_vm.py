import sys
from Memory import Memory
from QuadrupleParser import parse_wol
import utils

# Obj file input

if len(sys.argv) < 2:
    print('Error: No input files. Execution terminated.')
    exit(1)
    
filename = sys.argv[1]

print('Executing from obj file', filename)
print('')

memory = Memory()
is_running = True
curr_params = []

# Load quadruples

classDir, funcDir, quadruples = parse_wol(filename)

for key in classDir:
    memory.add_type(key)

# General Operations

def goto(quad):
    memory.instruction_pointer = int(quad[3])

def gotof(quad):
    if not utils.get_data(quad[1], memory):
        goto(quad)
    else:
        memory.instruction_pointer += 1

def end(quad):
    global is_running
    is_running = False
    memory.instruction_pointer += 1

def ver(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)

    if op1 < 0 or op1 >= op2:
        print('Error: Array index out of bounds')
        exit(1)
    
    memory.instruction_pointer += 1

# Function Operations
def gosub(quad):
    global curr_params

    target = int(funcDir[quad[3]].location)

    func_params = funcDir[quad[3]].params

    memory.call_stack.append(memory.instruction_pointer)
    memory.memory_stack.append(memory.init_nonglobal_memory())

    while len(func_params) > 0:
        memory.assign_to_address(curr_params.pop(), func_params.pop())

    memory.instruction_pointer = target

def param(quad):
    global curr_params

    param = utils.get_data(quad[1], memory)
    curr_params.append(param)

    memory.instruction_pointer += 1

def endfunc(quad):
    memory.instruction_pointer = memory.call_stack.pop()
    memory.memory_stack.pop()
    
    memory.instruction_pointer += 1

# Pointer Operations

def ptr_displace(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = memory.get_address(quad[2])

    assert type(op1) == int
    
    data = op1 + op2
    ptr_address = '&' + str(data)
    
    memory.assign_to_address(ptr_address, quad[3])

    memory.instruction_pointer += 1

def ptr_save(quad):
    address = utils.get_data(quad[3], memory)
    data = utils.get_data(quad[1], memory)
    memory.assign_to_address(data, address)

    memory.instruction_pointer += 1

def ptr_get(quad):
    address = utils.get_data(quad[1], memory)
    data = utils.get_data(address, memory)
    memory.assign_to_address(data, quad[3])

    memory.instruction_pointer += 1

# I/O Operations

def writeln(quad):
    data = utils.get_data(quad[3], memory)
    print(data)

    memory.instruction_pointer += 1

def write(quad):
    data = utils.get_data(quad[3], memory)
    print(data, end='')

    memory.instruction_pointer += 1

def read(quad):
    data = input()
    casted_data = utils.cast_to_type(data, memory.typename_from_address(quad[3]))
    memory.assign_to_address(casted_data, quad[3])

    memory.instruction_pointer += 1

# Expressions Operations

def assign(quad):
    data = utils.get_data(quad[1], memory)

    memory.assign_to_address(data, quad[3])

    memory.instruction_pointer += 1

def multiply(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 * op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def divide(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 / op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def modulo(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 % op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def add(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    if type(op1) == int and type(op2) == str:
        op1 = str(op1)
    elif type(op1) == str and type(op2) == int:
        op2 = str(op2)
    data = op1 + op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def subtract(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 - op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def greater(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 > op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def geq(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 >= op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def lesser(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 < op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def leq(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 <= op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def eq(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 == op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def neq(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 != op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def _or(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 or op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def _and(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = utils.get_data(quad[2], memory)
    
    data = op1 and op2
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

def _not(quad):
    op1 = utils.get_data(quad[1], memory)
    
    data = not op1
    
    memory.assign_to_address(data, quad[3])
    memory.instruction_pointer += 1

operations = {
    'goto': goto,
    'gotof': gotof,
    'end': end,
    'ver': ver,
    'gosub': gosub,
    'param': param,
    'endfunc': endfunc,
    '&disp': ptr_displace,
    '&save': ptr_save,
    '&get': ptr_get,
    'writeln': writeln,
    'write': write,
    'read': read,
    '=': assign,
    '*': multiply,
    '/': divide,
    '%': modulo,
    '+': add,
    '-': subtract,
    '>': greater,
    '>=': geq,
    '<': lesser,
    '<=': leq,
    '==': eq,
    '!=': neq,
    'or': _or,
    'and': _and,
    '!': _not
}

# Runtime

while is_running and memory.instruction_pointer < len(quadruples):
    quad = quadruples[memory.instruction_pointer]
    
    try:
        operations[quad[0]](quad)
    except KeyError:
        print("Error: Invalid operation", quad[0])
        exit(1)
    