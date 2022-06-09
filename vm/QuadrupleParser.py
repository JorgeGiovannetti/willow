def parse_wol(filename: str):

    obj_file = open(filename, 'r')

    quadruples = []
    for line in obj_file:
        quadruple = ['','','','']
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
        quadruple[quad_index] = temp_str.replace('\n','')
        quadruples.append(quadruple)
        
    obj_file.close()

    return quadruples