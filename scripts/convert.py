#%% 
import numpy as np
from scipy.signal import convolve2d

penalty_kernel = np.array([[10, 100, 10], [100, 1, 100], [10, 100, 10]])

def masked_middle(x):
  y = np.copy(x)
  y[1:-1, 1:-1] = 0
  return y

def empty_row_col_ids(x):
  row_indices = np.where(np.all(x == 0, axis=1))[0]
  col_indices = np.where(np.all(x == 0, axis=0))[0]
  return (row_indices, col_indices)

def penalty(x):
  y = masked_middle(x)
  return convolve2d(y, np.flip(penalty_kernel), mode='same', boundary='wrap').sum()

def find_pretty(s):

  min_penalty = penalty(s)
  prettiest_state = s

  for y_offset in range(N):
    for x_offset in range(N):
      s_prime = np.roll(s, shift=(y_offset, x_offset), axis=(0, 1))

      p = penalty(s_prime)

      if p <= min_penalty:
        min_penalty = p
        prettiest_state = s_prime
  
  if min_penalty != 0:
    return prettiest_state
  
  # 0 penalty means there exists a bounding box
  non_zero_indices = np.argwhere(prettiest_state != 0)

  if non_zero_indices.size > 0:  

    top_left = non_zero_indices[0]
    bottom_right = non_zero_indices[-1]
    bounding_box_size = bottom_right - top_left + [1, 1]

    board_midpoint = np.array([N, N]) // 2

    offset = board_midpoint - bounding_box_size // 2 - top_left
    return np.roll(prettiest_state, shift=(offset[0], offset[1]), axis=(0, 1))

N = 6
LIVE_CELL_CHAR = 'O'
DEAD_CELL_CHAR = '-'

def decimal_encoding_to_binary(x):
  num = int(x)
  board = ''.join(reversed(format(num, '08b')))
  board = board + '0' * (N * N - len(board)) # padding
  return board

def binary_encoding_to_numpy(x):
  array = np.array([int(bit) for bit in x])
  return array.reshape(-1, N)

def decimal_encoding_to_np(x):
  y = decimal_encoding_to_binary(x)
  return binary_encoding_to_numpy(y)

def handle_line(line):
  arrays = [decimal_encoding_to_np(id) for id in line.strip().split(' ')]
  arrays = [find_pretty(s) for s in arrays]
  print(arrays)

with open('s6.txt') as f:
  line_number = 0
  for line in f:

    # each configuration has N + 3 lines
    # second line of those N + 3 contains
    # cycle configuration numbers
    if line_number % (3 + N) == 1:
      handle_line(line)

    line_number = line_number + 1