def cycle_to_str(cycle, size, ascii):
  strings = []
  ascii_lookup = { 0: '-', 1: 'O' }

  for row in range(size):
    for i in range(len(cycle)):
      for col in range(size):
        cell_value = cycle[i][row][col]
        strings.append(ascii_lookup[cell_value] if ascii else f'{cell_value}')
        strings.append(' ')
      strings.append('  ')
    strings.append('\n')
  return ''.join(strings)