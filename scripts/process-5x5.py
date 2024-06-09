#%% 
import numpy as np
from utils import cycle_to_str
from converters import decimal_encoding_to_np
from prettifiy import find_pretty_offset

# 5x5 converter is a little bit different because it needs
# to account for the destination frames in the output file
N = 5
LIVE_CELL_CHAR = 'O'
DEAD_CELL_CHAR = '-'

def raw_destination_frames_to_numpy(lines):
  
  grouped_rows = [[t for t in line.split('   ') if t != '\n'] for line in lines]
  cycle_length = len(grouped_rows[0])
  frames = [[] for _ in range(cycle_length)]

  for i in range(N):
    for j in range(cycle_length):
      row = [ int(n) for n in grouped_rows[i][j].split(' ') ]
      frames[j].append(row)

  return np.array(frames)

with open('5x5-destination-frames.txt') as input, \
     open('5x5-destination-frames-pretty.txt', 'w') as output:
  
  # each loop reads and processes one cycle in the input file
  while True:
    
    period_and_id_line = input.readline()

    if period_and_id_line == '':
      break

    cycle_decimal_encoding = input.readline()
    cycle = [decimal_encoding_to_np(id, N) for id in cycle_decimal_encoding.strip().split(' ')]
    offsets = [find_pretty_offset(s, N) for s in cycle]

    # parse destination frames N frames
    destination_frames_raw = [ input.readline() for _ in range(N) ]
    destination_frames = raw_destination_frames_to_numpy(destination_frames_raw)
  
    # skip configration because we already have it from 2nd line
    [ input.readline() for _ in range(N + 2) ]

    # output to new file
    output.write(period_and_id_line)
    output.write(cycle_decimal_encoding)

    pretty_dest_frames = [ np.roll(f, shift=offset, axis=(0, 1)) for f, offset in zip(destination_frames, offsets) ]
    output.write(cycle_to_str(pretty_dest_frames, N, False))
    output.write('\n')
    pretty_cycle = [ np.roll(f, shift=offset, axis=(0, 1)) for f, offset in zip(cycle, offsets) ]
    output.write(cycle_to_str(pretty_cycle, N, True))
    output.write('\n')

# %%
import numpy as np
from utils import cycle_to_str
from converters import decimal_encoding_to_np
from prettifiy import find_pretty_offset
from converters import decimal_encoding_to_np
from converters import binary_encoding_to_numpy
from prettifiy import penalty
from prettifiy import find_pretty
from prettifiy import PENALTY_KERNEL
from prettifiy import masked_middle
from scipy.signal import convolve2d

#a = binary_encoding_to_numpy("0000100010001000100010000", 5)
b = decimal_encoding_to_np('1329', 5)

print(np.roll(b, shift=(1, 2), axis=(0, 1)))
print(penalty(np.roll(b, shift=(1, 2), axis=(0, 1))))

print(find_pretty(b, 5))

#print(b)
#print(masked_middle())
#print(convolve2d(masked_middle(b), np.flip(PENALTY_KERNEL), mode='same', boundary='wrap'))

#penalty(a), 
#print(penalty(b))
#print(find_pretty(b, 5))
# %%
