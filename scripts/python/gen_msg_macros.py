import os
import datetime
import zipfile
import urllib
import sys

num_max_args = 5

def gen_macros(hardly_macro, dynamix_macro):
    for i in range(0, num_max_args + 1):
        args = ''
        if i > 0:
            for k in range(0, i):
                args += ', a_' + str(k) + '_t, a_' + str(k) + '_n'
        print('#define ' + hardly_macro + '_' + str(i) + '(ns, r_t, n' + args + ') namespace ns \\\n    { ' + dynamix_macro + '_' + str(i) + '(HAPI, r_t, n' + args + ') }')

gen_macros('HA_MSG', 'DYNAMIX_EXPORTED_MESSAGE')
print('')
gen_macros('HA_CONST_MSG', 'DYNAMIX_EXPORTED_CONST_MESSAGE')
print('')
gen_macros('HA_MULTI_MSG', 'DYNAMIX_EXPORTED_MULTICAST_MESSAGE')
print('')
gen_macros('HA_CONST_MULTI_MSG', 'DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE')
print('')
