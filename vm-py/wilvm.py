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
    if quad[1] == False:
        goto(quad)
    else:
        memory.instruction_pointer += 1

def end(quad):
    global is_running
    is_running = False
    memory.instruction_pointer += 1

def assign(quad):
    data = utils.get_data(quad[1])

    # TODO: Assign data to memory address

    memory.instruction_pointer += 1

operations = {
    "goto": goto,
    "gotof": gotof,
    "end": end,
    "=": assign
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
    