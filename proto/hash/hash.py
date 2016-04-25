#!/usr/bin/env python

import random

import matplotlib.pyplot as plt

print("Starting...")

def get_strings(method):
  letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
  strings = letters
  if method == "examples":
    with open("uniq.txt") as fin:
      strings = fin.readlines()
  elif method == "constants":
    with open("const.txt") as fin:
      strings = fin.readlines()
  elif method == "mixed":
    with open("muniq.txt") as fin:
      strings = fin.readlines()
  elif method == "sample":
    strings = [
      "~source",
      "~density",
      "~plasticity",
      "~hardness",
      "~seed",
    ]
  elif method == "common_single":
    strings = "xyzijkLchrgb"
  elif method == "letters":
    strings = letters
  elif method == "permute":
    strings = []
    for l_1 in letters:
      strings.append(l_1)
      for l_2 in letters:
        strings.append(l_1 + l_2)
        for l_3 in letters:
          strings.append(l_1 + l_2 + l_3)
          for l_4 in letters:
            strings.append(l_1 + l_2 + l_3 + l_4)
  elif method == "random":
    strings = []
    for i in range(20000):
      s = ""
      length = random.choice(range(1, 30))
      for i in range(length):
        s += random.choice(letters)
      strings.append(s)

  return strings

#strings = get_strings("constants")
strings = get_strings("mixed")
#strings = get_strings("sample")

print(" ...strings...")

def hash(data):
  size = len(data)
  result = 0
  for i, c in enumerate(data):
    #result += ord(c) << ((3*i) % 56)
    result ^= ord(c) << ((8*i) % 57)
  #result = (
  #  ord(data[0]) ^
  #  (ord(data[( 1 % size)]) << 3) ^
  #  (ord(data[( 2 % size)]) << 6) ^
  #  (ord(data[( 3 % size)]) << 9) ^
  #  (ord(data[( 4 % size)]) << 12) ^
  #  (ord(data[( 8 % size)]) << 16) ^
  #  (ord(data[( 9 % size)]) << 20) ^
  #  (ord(data[(14 % size)]) << 22) ^
  #  (ord(data[(15 % size)]) << 24)
  #)

  #result = 16103 * (result + 48611)

  #result = result ^ (result % 17)
  result = result ^ (result % 17) ^ (result % 6011)
  #result = result ^ (result % 17) ^ (result % 1024) ^ (result % 6011)
  return result;

hashes = []
for s in strings:
  hashes.append(hash(s))

print(" ...hashes...")

h8 = [h % 8 for h in hashes]
h16 = [h % 16 for h in hashes]
h128 = [h % 128 for h in hashes]
h1024 = [h % 1024 for h in hashes]
h8192 = [h % 8192 for h in hashes]

print(" ...gathered...")

fig, ax = plt.subplots(ncols=5)

ax[0].hist(h8, 8)
ax[1].hist(h16, 16)
ax[2].hist(h128, 128)
ax[3].hist(h1024, 1024)
ax[4].hist(h8192, 8192)

print(" ...figures...")

for x in ax:
  x.axis("tight")

fig.set_tight_layout(True)
fig.set_size_inches(36, 8)

plt.savefig("hist.pdf", format="pdf")

print(" ...done.")

