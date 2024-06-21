
#%%

import re

def parse_ranks(file_contents):
    split_chars = ' ,\n'
    pattern = f"[{re.escape(split_chars)}]"
    return [int(rank) for rank in re.split(pattern, file_contents) if rank != '']

with open('rankings-8.txt') as f:
    ranks = parse_ranks(f.read())
    print(ranks)

# %%
