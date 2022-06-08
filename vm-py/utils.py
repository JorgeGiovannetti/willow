from Memory import Memory

def is_address(data: str):
    return len(data) > 0 and data[0] == '&'

def cast_literal(data: str):
    # string
    if data[0] == '"':
        return data[1:-1]

    # char

    if data[0] == '\'':
        return data[1:-1]
    
    # bool
    if data == "true" or data == "false":
        return data == "true"
    
    # int
    if float(data) == int(data):
        return int(data)
    
    # float
    return float(data)

def get_data(data: str, memory: Memory):
    if is_address(data):
        # TODO: Get value from address
        pass
    else:
        return cast_literal(data)
