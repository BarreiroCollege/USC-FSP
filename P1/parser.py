#!/usr/bin/env python3
import os
import re

DIR = "out/"
EJS = ["montecarlo", "trapecio"]
CORES = [1, 2, 4, 8, 16, 32]
NUM_PRUEBAS = 5


class Ejecucion:
    def __init__(self, T, E, C):
        self.T = T
        self.E = E
        self.C = C


DATA = []
for i, ej in enumerate(EJS):
    DATA.append([])
    for j, c in enumerate(CORES):
        DATA[i].append([])

for FILE in os.listdir(DIR):
    with open("%s%s" % (DIR, FILE)) as f:
        ej, core = None, None
        t, e, c = None, None, None
        for i, l in enumerate(f):
            if i == 1:
                match = re.search("PI\('(\w*)', (\d*)\)", l)
                ej = EJS.index(match.group(1).lower())
                core = CORES.index(int(match.group(2)))
            elif i == 2:
                match = re.search("T=([\d.]*)", l)
                t = float(match.group(1))
            elif i == 3:
                match = re.search("E=([\d.]*)", l)
                e = float(match.group(1))
            elif i == 4:
                match = re.search("C=([\d.]*)", l)
                c = float(match.group(1))
        DATA[ej][core].append(Ejecucion(t, e, c))

with open("out.csv", "w") as f:
    for i, ej in enumerate(EJS):
        for j, core in enumerate(CORES):
            for k, data in enumerate(DATA[i][j]):
                f.write("%s,%d,%lf,%lf,%lf\n" % (ej, core, data.T, data.E, data.C))
