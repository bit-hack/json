#!python

import sys


# ----------------------------------------------------------------------------

_seed = 12348

def _random():
    global _seed
    x = _seed
    x ^= x >> 12 & 0xffffffff
    x ^= x << 25 & 0xffffffff
    x ^= x >> 27 & 0xffffffff
    _seed = x    & 0xffffffff
    return (x * 0x4F6CDD1D) >> 8

def random(maximum):
    return _random() % maximum if maximum else 0

def choose(*args):
    return args[random(len(args))]

def rand_range(lo, hi):
    return lo + random(hi-lo)

def rand_name():
    alpha_num = '_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWXYZ0123456789'
    size = rand_range(1, 8)
    out = alpha_num[random(26 * 2 + 1)]
    for _ in range(0, size):
        out += alpha_num[random(len(alpha_num))]
    return out

# ----------------------------------------------------------------------------

def emit(s):
    sys.stdout.write(s)

# ----------------------------------------------------------------------------

complex = 0

def gen_ws():
    ws = [' ', '\t', '\n']
    num = random(4)
    for i in range(0, num):
        emit(choose(*ws))

def gen_member():
    gen_ws()
    gen_string()
    gen_ws()
    emit(':')
    gen_element()

def gen_members():
    num = random(4)
    for i in range(0, num):
        if i:
            emit(',')
        gen_member()

def gen_object():
    global complex
    complex += 1
    emit('{')
    gen_members()
    emit('}')

def gen_element():
    gen_ws()
    gen_value()
    gen_ws()

def gen_elements():
    num = random(10)
    for i in range(0, num):
        if i:
            emit(',')
        gen_element()

def gen_array():
    global complex
    complex += 1
    emit('[')
    gen_elements()
    emit(']')

def gen_string():
    name = rand_name()
    emit('"{}"'.format(name))

def gen_number():
    val = random(1000)
    emit(str(val))

def gen_true():
    emit('true')

def gen_false():
    emit('false')

def gen_null():
    emit('null')

def gen_value():
    global complex
    func = [ gen_string, gen_number, gen_true, gen_false, gen_null ]
    if complex < 15:
        func += [gen_object, gen_array]
        func += [gen_object, gen_array]
    choose(*func)()

def main():
    gen_element()

if __name__ == '__main__':
    _seed = int(sys.argv[1]) if len(sys.argv) > 1 else 12341
    main()
