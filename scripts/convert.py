#%%
import numpy as np
from scipy.signal import convolve2d

#%% Configure size and visuals
N = 7
LIVE_CELL_CHAR = 'O'
DEAD_CELL_CHAR = '-'

penalty_kernel = np.array([[10, 100, 10], [100, 1, 100], [10, 100, 10]])

def parse_input(str):
  str = str.replace(DEAD_CELL_CHAR, '0')
  str = str.replace(LIVE_CELL_CHAR, '1')
  lines = str.split('\n')
  return np.array([[int(num) for num in line.split()] for line in lines if line != ''])

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

      if p < min_penalty:
        min_penalty = p
        prettiest_state = s_prime
  
  if min_penalty != 0:
    return prettiest_state
  # there exists a bounding box
  # try to find it
  non_zero_indices = np.argwhere(prettiest_state != 0)

  if non_zero_indices.size > 0:  

    top_left = non_zero_indices[0]
    bottom_right = non_zero_indices[-1]
    bounding_box_size = bottom_right - top_left + [1, 1]

    board_midpoint = np.array([N, N]) // 2

    offset = board_midpoint - bounding_box_size // 2 - top_left
    return np.roll(prettiest_state, shift=(offset[0], offset[1]), axis=(0, 1))
  
with open('input.txt') as f:
  str = f.read(2 * N ** 2) # input is spaced out horizontally
  s = parse_input(str)
  print(find_pretty(s))
