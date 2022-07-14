#!/usr/bin/env python3
import fileinput
import subprocess
import time

JOB = "job.sh"
# EJS = ["montecarlo", "trapecio"]
EJS = ["trapecio"]
CORES = [1, 2, 4, 8, 16, 32]

NUM_PRUEBAS = 5


for ej in EJS:
    print("Compilando %s..." % ej)
    subprocess.run(["mpicc", "-o", ej, "%s.c" % ej, "-lm"])
    for c in CORES:
        f = open("%s.sample" % JOB, mode='r')
        sample = f.read()
        f.close()

        sample = sample.replace("__C__", str(c)).replace("__EJ__", ej)

        with open(JOB, "w") as f:
            f.write(sample)
        for i in range(NUM_PRUEBAS):
            print("Enviando %s(c=%d, i=%d)" % (ej, c, i+1))
            subprocess.run(["sbatch", JOB], stdout=subprocess.DEVNULL)
            time.sleep(1)
        time.sleep(5)
    time.sleep(10)
