import numpy as np

def decimal_encoding_to_binary(x, size):
  num = int(x)
  board = ''.join(reversed(format(num, '08b')))
  board = board + '0' * (size * size - len(board)) # padding
  return board

def binary_encoding_to_numpy(x, size):
  array = np.array([int(bit) for bit in x])
  return array.reshape(-1, size)

def decimal_encoding_to_np(x, size):
  y = decimal_encoding_to_binary(x, size)
  return binary_encoding_to_numpy(y, size)