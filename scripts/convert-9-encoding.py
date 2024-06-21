#%%
N = 9
LINES_PER_CYCLE = N + 3 

def encode_to_binary(number, bit_length):
    # Convert the number to binary and remove the '0b' prefix
    binary_str = bin(number)[2:]
    # Pad the binary string with leading zeros to the specified length
    padded_binary_str = binary_str.zfill(bit_length)
    
    return padded_binary_str

with open('9.txt') as input, open('9-altered.txt', 'w') as output:

    line_number = 0

    for line in input:
        if line_number % LINES_PER_CYCLE == 0:
            # period/id line
            output.write(line)

        if line_number % LINES_PER_CYCLE == 1:
            # cycle frames line
            frames = [ int(token) for token in line.strip().split(' ') if token != '']
            # some number of bits enough to encode given number without 0b
            encoded = [ bin(frame)[2:] for frame in frames ] 
            # pad with zeros 
            encoded = [ num.zfill(128) for num in encoded ] 
            encoded = [ (num[:64], num[64:]) for num in encoded ]
            [ output.write(f'{int(hi, 2)} {int(lo, 2)} ') for hi, lo in encoded ]
            output.write('\n')
            
        line_number = line_number + 1
# %%
