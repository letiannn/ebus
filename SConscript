from building import *
import os

cwd = GetCurrentDir()
group = []
src = Glob('*.c')
CPPPATH = [cwd]

if GetDepend('CCMP_USING_EXAMPLE_EBUS'):
    src += Glob('example/*.c')

list = os.listdir(cwd)
for d in list:
    path = os.path.join(cwd, d)
    if os.path.isfile(os.path.join(path, 'SConscript')):
        group = group + SConscript(os.path.join(d, 'SConscript'))

group = group + DefineGroup('ccmp_ebus', src, depend = ['CCMP_USING_EBUS'], CPPPATH = CPPPATH)

Return('group')
