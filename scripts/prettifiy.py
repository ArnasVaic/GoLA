import numpy as np
from scipy.signal import convolve2d

PENALTY_KERNEL = np.array([[10, 100, 10], [100, 1, 100], [10, 100, 10]])

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
  return convolve2d(y, np.flip(PENALTY_KERNEL), mode='same', boundary='wrap').sum()

def find_pretty_offset(s, size):

  if np.all(s == 0):
    return (0, 0)

  min_penalty = penalty(s)
  #prettiest_state = s
  best_offset = (0, 0)

  for y_offset in range(size):
    for x_offset in range(size):
      s_prime = np.roll(s, shift=(y_offset, x_offset), axis=(0, 1))

      p = penalty(s_prime)

      if p <= min_penalty:
        min_penalty = p
        #prettiest_state = s_prime
        best_offset = (y_offset, x_offset)

        if min_penalty == 0:
          break

  if min_penalty != 0:
    return best_offset

  # 0 penalty means there exists a bounding box
  prettiest_state = np.roll(s, shift=best_offset, axis=(0, 1))
  non_zero_indices = np.argwhere(prettiest_state != 0)
  
  if non_zero_indices.size > 0:  

    top_left = np.min(non_zero_indices, axis=0)
    bottom_right = np.max(non_zero_indices, axis=0)
    bounding_box_size = bottom_right - top_left + [1, 1]

    board_midpoint = np.array([size, size]) // 2
    offset = board_midpoint - bounding_box_size // 2 - top_left
    return (best_offset[0] + offset[0], best_offset[1] + offset[1])

def find_pretty(s, size):
  offset = find_pretty_offset(s, size)
  return np.roll(s, shift=offset, axis=(0, 1))