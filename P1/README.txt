CÓDIGO FUENTE
trapecio.c montecarlo.c

SBATCH JOB
Está definido en job.sh.sample, el cual sustituyendo el ejecutable y el número de tareas
permite hacer todo lo requerido para la práctica.

LANZADOR
jobs.py es un programa de Python que ejecuta todas las combinaciones de procesadores y
métodos.

PARSEADOR
Las salidas de las pruebas van a la carpeta out/, y el archivo parser.py las recolecta
todas y genera un CSV para importar en Excel.