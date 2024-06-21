# %%
import numpy as np

with open('6tr.txt') as file:
    array = []
    for line in file:
        array.append([ float(token) for token in line.split(' ') if token != ''])
    array = np.array(array)
    print(array.shape)
# %%
