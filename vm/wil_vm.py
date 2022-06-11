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

memory = Memory()
is_running = True

# Load quadruples

quadruples = parse_wol(filename)

# Operations

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

def ptr_save(quad):
    op1 = utils.get_data(quad[1], memory)
    op2 = memory.get_address(quad[2])
    
    data = op1 + op2
    
    memory.assign_to_address('&' + str(data), quad[3])

    memory.instruction_pointer += 1

def ptr_get(quad):
    address = utils.get_data(quad[1], memory)

    print('ptr_get got address', address)

    data = utils.get_data(address, memory)
    print('ptr_get got data', data)

    memory.assign_to_address(data, quad[3])

    memory.instruction_pointer += 1

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
    '&save': ptr_save,
    '&get': ptr_get,
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
    print('Running quadruple', memory.instruction_pointer, quad)
    
    try:
        operations[quad[0]](quad)
    except KeyError:
        print("Error: Invalid operation", quad[0])
        exit(1)
    