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

operations = {
    "goto": goto,
    "gotof": gotof,
    "end": end,
    "=": assign,
    "*": multiply,
    "/": divide,
    "%": modulo,
    "+": add,
    "-": subtract,
    ">": greater,
    ">=": geq,
    "<": lesser,
    "<=": leq,
    "==": eq,
    "!=": neq
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
    