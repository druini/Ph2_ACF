from launcher import *

l = Launcher()

# os.chdir('..')

proc = l.launch('gui/test_pipe')

with open_pipe() as pipe:
    line = pipe.readline()
    while line:
        print(line)
        line = pipe.readline()

proc.wait()