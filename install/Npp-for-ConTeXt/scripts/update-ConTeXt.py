import os
#import os.path
import subprocess
import sys
import shutil
import xml.etree.ElementTree as ET
import xml.dom 
import xml.dom.minidom 
import argparse

import itertools
import os
import re

'''
This python script replaces the LexerStyles of the new ConTeXt.xml with the LexerStyles
of the current ConTeXt.xml. It saves the result as target-ConTeXt.xml, or optionally
overwrite the target.


usage: update-ConTeXt.py [-h] [-t TARGETCONTEXTFILE] [-n NEWCONTEXTFILE]
                         [--overwrite]

optional arguments:
  -h, --help            show this help message and exit
  -t TARGETCONTEXTFILE, --targetcontextfile TARGETCONTEXTFILE
                        current ConTeXt.xml for Notepad++
  -n NEWCONTEXTFILE, --newcontextfile NEWCONTEXTFILE
                        new ConTeXt.xml for Notepad++
  --overwrite           overwrite targetcontextfile


Default current ConTeXt.xml for Notepad++: C:/Program Files (x86)/Notepad++/plugins/config/ConTeXt.xml
Default new ConTeXt.xml for Notepad++:     ConTeXt.xml
Default overwrite: false
'''


 
def unique_file_name(file):
    ''' Append a counter to the end of file name if 
    such file allready exist.'''
    if not os.path.isfile(file):
        # do nothing if such file doesn exists
        return file 
    # test if file has extension:
    if re.match('.+\.[a-zA-Z0-9]+$', os.path.basename(file)):
        # yes: append counter before file extension.
        name_func = \
            lambda f, i: re.sub('(\.[a-zA-Z0-9]+)$', '_%i\\1' % i, f)
    else:
        # filename has no extension, append counter to the file end
        name_func = \
            lambda f, i: ''.join([f, '_%i' % i])
    for new_file_name in \
            (name_func(file, i) for i in itertools.count(1)):
        if not os.path.exists(new_file_name):
            return new_file_name


encoding = 'UTF-8'
parser = argparse.ArgumentParser()

parser.add_argument("-t", "--targetcontextfile",
                    default='C:/Program Files (x86)/Notepad++/plugins/config/ConTeXt.xml',
                    help="current ConTeXt.xml for Notepad++")
parser.add_argument("-n", "--newcontextfile",
                    default='ConTeXt.xml',
                    help="new ConTeXt.xml for Notepad++")
parser.add_argument("--overwrite", 
                    action='store_true',
                    help="overwrite targetcontextfile")


cwd = os.getcwd()

args = parser.parse_args()

targetcontextfile = args.targetcontextfile
newcontextfile    = args.newcontextfile
overwrite = args.overwrite

t_doc = xml.dom.minidom.parse(targetcontextfile)
n_doc = xml.dom.minidom.parse(newcontextfile)

t_root = t_doc.documentElement
n_root = n_doc.documentElement

t_LexerStyles_nodelist = t_root.getElementsByTagName('LexerStyles')
n_LexerStyles_nodelist = n_root.getElementsByTagName('LexerStyles')

if  t_LexerStyles_nodelist.length==1 :
    t_LexerStyles = t_LexerStyles_nodelist.item(0)
    if  n_LexerStyles_nodelist.length==1 :
        n_LexerStyles = n_LexerStyles_nodelist.item(0)
        n_root.replaceChild(t_LexerStyles,n_LexerStyles)
    else:
        print("new xml file has {0} LexerStyles, it should be 1".format(t_LexerStyles_nodelist.length))        
else:
    print("target xml file has {0} LexerStyles, it should be 1".format(t_LexerStyles_nodelist.length))


## write  the xml file 
#  target-ConTeXt.xml
if overwrite == False:
    outfilename = 'target-ConTeXt.xml'
else:
    backupfile = unique_file_name(targetcontextfile)
    shutil.copy2(targetcontextfile,backupfile)
    outfilename = targetcontextfile

outfile = open(outfilename,'wb')
n_root.writexml(outfile)
outfile.close()
print('wrote ' + outfilename)
    
