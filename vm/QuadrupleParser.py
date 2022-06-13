def parse_quadruples(quadruples: [], line: str):
    quadruple = ['', '', '', '']
    quad_index = 0
    is_in_string = False
    temp_str = ''
    for line_index in range(len(line)):
        if not is_in_string and line[line_index] == ',':
            quadruple[quad_index] = temp_str
            temp_str = ''
            quad_index += 1
        elif line[line_index] == '"' or line[line_index] == '\'':
            is_in_string = not is_in_string
            temp_str += line[line_index]
        else:
            temp_str += line[line_index]
    quadruple[quad_index] = temp_str.replace('\n', '')
    quadruples.append(quadruple)


def parse_wol(filename: str):

    obj_file = open(filename, 'r')

    classDir = {}
    funcDir = {}
    quadruples = []
    position = 0
    for line in obj_file:
        if line[:3] == '<%>':
            position += 1
        else:
            if position == 0:
                c = line.split(' ')
                classDir[c[0]] = c[1]
            elif position == 1:
                f = line.split(' ')
                funcDir[f[0]] = f[1]
            else:        
                parse_quadruples(quadruples, line)

    obj_file.close()

    return classDir, funcDir, quadruples