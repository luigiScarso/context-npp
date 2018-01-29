import os
#import os.path
import subprocess
import sys
import shutil
import xml.etree.ElementTree as ET
from xml.dom import minidom
#import string
#import copy
import argparse


'''
This python script lets you generate the list of ConTeXt keywords from 
the standard context-en.xml, saving the result into the  context.xml 
file for Notepad++ .

usage: python command_primitives.py [-h] [-m MTXCMD] [-c CMDFILE] [-p PRIMFILE]

optional arguments:
  -h, --help            show this help message and exit
  -m MTXCMD, --mtxcmd MTXCMD
                        ConTeXt mtxrun program
  -i IFCFILE, --ifcfile IFCFILE
                        ConTeXt XML interface file
  -c CMDFILE, --cmdfile CMDFILE
                        ConTeXt XML command file
  -p PRIMFILE, --primfile PRIMFILE
                        ConTeXt primitives.lua script
  -d CHARDEF, --chardef CHARDEF
                        ConTeXt char-def.lua file
  -r MACROCHARDEFFILE, --macrochardeffile MACROCHARDEFFILE
                        ConTeXt macro-chardef.lua script

Default ConTeXt XML interface file C:/context/tex/texmf-context/tex/context/interface/mkiv/context-en.xml
Default ConTeXt mtxrun program: C:/context/tex/texmf-context/tex/texmf-win32/bin/mtxrun.exe
Default ConTeXt XML command file: C:/context/tex/texmf-context/tex/context/base/mkiv/mult-prm.lua
Default ConTeXt primitives.lua script: primitives.lua
Default ConTeXt char-def.lua file: C:/context/tex/texmf-context/tex/context/base/mkiv/chard-def.lua
Default ConTeXt macro-chardef.lua script: macro-chardef.lua
'''


## pretty print adapted
## https://pymotw.com/2/xml/etree/ElementTree/create.html
def prettify(elem,encoding):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = ET.tostring(elem, encoding)
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="    ", encoding=encoding)


def check(name,encoding):
    """check if the name is valid for encoding
    """
    Name = name.encode(encoding,errors='ignore')
    try:
            name.encode(encoding)
    except:
            print("! warning {0} is not valid for encoding '{1}': replaced with '{2}'".format(repr(name),encoding,Name))
    return Name


def parseifcfile(ifcfilename):
    """...
    """
    kwords = set()
    kwordl = []
    tree = ET.parse(ifcfilename)
    root = tree.getroot()
    return root



encoding = 'UTF-8'
parser = argparse.ArgumentParser()

parser.add_argument("-m", "--mtxcmd",
                    default='C:/context/tex/texmf-context/tex/texmf-win32/bin/mtxrun.exe',
                    help="ConTeXt mtxrun program")
parser.add_argument("-i", "--ifcfile",
                    default='C:/context/tex/texmf-context/tex/context/interface/mkiv/context-en.xml',
                    help="ConTeXt XML interface file")
parser.add_argument("-c", "--cmdfile",
                    default='C:/context/tex/texmf-context/tex/context/base/mkiv/mult-prm.lua',
                    help="ConTeXt XML command file")
parser.add_argument("-p", "--primfile",
                    default='primitives.lua',
                    help="ConTeXt primitives.lua script")
parser.add_argument("-d", "--chardef",
                    default='C:/context/tex/texmf-context/tex/context/base/mkiv/char-def.lua',
                    help="ConTeXt char-def.lua file")
parser.add_argument("-r", "--macrochardeffile",
                    default='macro-chardef.lua',
                    help="ConTeXt macro-chardef.lua script")


args = parser.parse_args()

ifcfile          = args.ifcfile
contextcmd       = args.cmdfile
mtxrun           = args.mtxcmd
chardef          = args.chardef
primitives_lua   = 'primitives.lua'
macrochardef_lua = 'macro-chardef.lua'
cwd              = os.getcwd()


if not( os.path.isfile(ifcfile)):
    print("! error: %s is not a file" %(ifcfile))
    sys.exit(1)        
try:
    shutil.copy2(ifcfile,cwd)
except Exception as e:
    print("! error: %s" %(e) )
    sys.exit(1)        


if not( os.path.isfile(contextcmd)):
    print("! error: %s is not a file" %(contextcmd))
    sys.exit(1)        
try:
    shutil.copy2(contextcmd,cwd)
except Exception as e:
    print("! error: %s" %(e) )
    sys.exit(1)        

if not( os.path.isfile(chardef)):
    print("! error: %s is not a file" %(chardef))
    sys.exit(1)        
try:
    shutil.copy2(chardef,cwd)
except Exception as e:
    print("! error: %s" %(e) )
    sys.exit(1)        
    

try:
    ##if (subprocess.call(mtxrun, shell=True,stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=False))!=0 :
	subprocess.check_output(mtxrun)
except Exception as e:
    print("! error: %s" %(e))
    sys.exit(1)        
   
if not( os.path.isfile(primitives_lua)):
    print("! error: %s not found" %(primitives_lua))
    sys.exit(1)        
    
print("calling "+mtxrun + ' --script primitives.lua')            
#subprocess.call([mtxrun,'--script ', primitives_lua])
if (subprocess.call(mtxrun + ' --script primitives.lua',shell=True))!=0:
    print("! error: %s --script primitives.lua failed" %(mtxrun))
    sys.exit(1)        

if not( os.path.isfile(macrochardef_lua)):
    print("! error: %s not found" %(macrochardef_lua))
    sys.exit(1)        
    
print("calling "+mtxrun + ' --script macro-chardef.lua')            
#subprocess.call([mtxrun,'--script ', primitives_lua])
if (subprocess.call(mtxrun + ' --script macro-chardef.lua',shell=True))!=0:
    print("! error: %s --script macro-chardef.lua" %(mtxrun))
    sys.exit(1)        

## 
## Output from primitives.lua:
## prm-etex.txt  prm-luatex.txt  prm-tex.txt
##

if not( os.path.isfile('prm-tex.txt')):
    print("! error: prm-tex.txt not found" )
    sys.exit(1)        

if not( os.path.isfile('prm-tex.txt')):
    print("! error: prm-tex.txt not found" )
    sys.exit(1)        
    
if not( os.path.isfile('prm-luatex.txt')):
    print("! error: prm-luatex.txt not found" )
    sys.exit(1)        

tex = []
inf = open('prm-tex.txt')
for line in inf:
   tex.append(line.replace('"','').rstrip())
inf.close()

etex = []
inf = open('prm-etex.txt')
for line in inf:
   etex.append(line.replace('"','').rstrip())
inf.close()

luatex = []
inf = open('prm-luatex.txt')
for line in inf:
   luatex.append(line.replace('"','').rstrip())
inf.close()


## 
## Output from macro-chardef.lua:
## macro-chardef.txt
##

if not( os.path.isfile('macro-chardef.txt')):
    print("! error: macro-chardef.txt" )
    sys.exit(1)        

macrochardef = []
inf = open('macro-chardef.txt')
for line in inf:
   macrochardef.append(line.replace('"','').rstrip())
inf.close()


## Build the output tree for ConTeXt.xml
NotepadPlus  = ET.Element('NotepadPlus')
Languages    = ET.SubElement(NotepadPlus,'Languages')
#LexerStyles  = ET.SubElement(NotepadPlus,'LexerStyles') ## later


##
## Language
##
Language   = ET.SubElement(Languages,'Language',
               {'name':"ConTeXt", 'ext':"tex", 'commentLine':"%", 'commentStart':"",
                 'commentEnd':""})
Language.append(ET.Comment(' luatex '))
Keywords_0 = ET.SubElement(Language,'Keywords',{'name': "0"});

Language.append(ET.Comment(' system '));
Keywords_1 = ET.SubElement(Language,'Keywords',{'name': "1"})

Language.append(ET.Comment(' document '));
Keywords_2 = ET.SubElement(Language,'Keywords',{'name': "2"})

Language.append(ET.Comment(' style '));
Keywords_3 = ET.SubElement(Language,'Keywords',{'name': "3"})

Language.append(ET.Comment(' other '));
Keywords_4 = ET.SubElement(Language,'Keywords',{'name': "4"})

Language.append(ET.Comment(' construct '));
Keywords_5 = ET.SubElement(Language,'Keywords',{'name': "5"})

Language.append(ET.Comment(' private '));
Keywords_6 = ET.SubElement(Language,'Keywords',{'name': "6"})

Language.append(ET.Comment(' start open '))
Keywords_7 = ET.SubElement(Language,'Keywords',{'name': "7"})

Language.append(ET.Comment(' stop close '))
Keywords_8 = ET.SubElement(Language,'Keywords',{'name': "8"})

## Output from mult-prm.lua
# now these are hardcoded
# Keywords_0.text = str.join(" ",tex)
# Keywords_1.text = str.join(" ",etex)
#
# We keep luatex, stil under development
Keywords_0.text = str.join(" ",luatex)


## API/context.xml:
## we keep tex and etex
## as well as luatex
kwords = set()
kwordl = []
kwords.update([i.split('\\')[1] for i in tex],[i.split('\\')[1] for i in etex],[i.split('\\')[1] for i in luatex])

## Output from macro-chardef.lua
## API/context.xml
kwords.update([i.split('\\')[1] for i in macrochardef])


## Input from context-en.xml
cd  = '{http://www.pragma-ade.com/commands}'
root = parseifcfile(ifcfile)
## A sax/xslt approach is more elegant, but
## we want a fast generation of the final elements
kwords_Begin  = set()
kwords_End  = set()
kwords_System  = set()
kwords_Document  = set()
kwords_Style  = set()
kwords_Other = set()
kwords_7 = set()
kwords_8 = set()
#
kwordl_Begin = []
kwordl_End = []
kwordl_System = []
kwordl_Document = []
kwordl_Style = []
kwordl_Other = []
kwordl_7 = []
kwordl_8 = []


## Output from macro-chardef.lua
## ConTeXt.xml
kwords_Other.update([i.split('\\')[1] for i in macrochardef])


## We map these xml values
## to be consistent with the pdf
remap_ctx_method ={
    'factor' : '*',
    'range'  : ':',
    'apply'  : '->:',
    'none'   : '' 
}
remap_ctx_types ={
    'cd:buffer' :'BUFFER',            
    'cd:category':'CATEGORY',          
    'cd:character':'CHARACTER',         
    'cd:color':'COLOR',             
    'cd:command':'COMMAND',           
    'cd:content':'CONTENT',           
    'cd:csname':'CSNAME',            
    'cd:dimension':'DIMENSION',         
    'cd:false':'FALSE',             
    'cd:file':'FILE',              
    'cd:first':'FIRST',             
    'cd:font':'FONT',              
    'cd:key':'KEY',               
    'cd:language':'LANGUAGE',          
    'cd:last':'LAST',              
    'cd:list':'LIST',              
    'cd:lpath':'LPATH',             
    'cd:luafunction':'LUAFUNCTION',       
    'cd:mark':'MARK',              
    'cd:matrix':'MATRIX',            
    'cd:name':'NAME',              
    'cd:node':'NODE',              
    'cd:number':'NUMBER',            
    'cd:oneargument':'ONEARGUMENT',       
    'cd:path':'PATH',              
    'cd:plural':'PLURAL',            
    'cd:processor' :'PROCESSOR',
    'cd:reference':'REFERENCE',         
    'cd:section':'SECTION',           
    'cd:sectionblock':'SECTIONBLOCK',           
    'cd:setup':'SETUP',             
    'cd:sign' : '[-+]',
    'cd:singular':'SINGULAR',          
    'cd:string':'STRING',            
    'cd:style':'STYLE',             
    'cd:template':'TEMPLATE',          
    'cd:text':'TEXT',              
    'cd:threearguments':'THREEARGUMENTS',    
    'cd:true':'TRUE',              
    'cd:twoarguments':'TWOARGUMENTS',      
    'cd:url':'URL',               
    'cd:value':'VALUE',             
    'cd:xmlsetup':'XMLSETUP'
}
remap_ctx_methods ={
    'range' : ':',
    'apply' : '->'
}



argshash = {}
for command in root.iter(cd + 'command'):
        name    = command.get('name')
        begin   = command.get('begin')
        end     = command.get('end')
        tp      = command.get('type')
        level   = command.get('level')
        variant = command.get('variant')
        seq     = command.find(cd + 'sequence')
        insts   = command.find(cd + 'instances')
        cat     = command.get('category')
        typec   = command.get('type')
        arguments = command.find(cd +'arguments')
       

        ##
        ## For the moment we skip variant=='example'
        ##
        if (variant is not None) and (variant=='example'):
            continue 

        ##
        ## We can have several
        ## instances for each command
        ## 
        Names = []
        if insts is not None:
            ##
            ## An instance can also have 
            ## a prefix and/or a postfix
            ##
            pre=''
            post=''
            if seq is not None:
                if len(seq)==3:
                    pre=seq[0].get('value')
                    post=seq[2].get('value')
                if len(seq)==2 and (seq[0].tag==(cd+'instance')):
                    post=seq[1].get('value')
                if len(seq)==2 and (seq[1].tag==(cd+'instance')):
                    pre=seq[0].get('value')
            ##    
            ## iterate on over all the instances    
            for i in insts.iterfind(cd+'constant'):
                Names.append("".join([pre,i.get('value'),post]))
        else:
            Names.append(name)
        for name in Names :
            Inserted = False
            BeginName=None
            StartName=None
            SimpleName=None
            ##
            ## We can have
            ## \beginNAME..\endNAME
            ## and/or
            ## \startNAME..\stopNAME
            ## but in this case
            ## we don't have \NAME
            ##
            if ((begin) and (end)):
                Begin = check(begin,"US-ASCII")
                End   = check(end,"US-ASCII")
                Name  = check(name,"US-ASCII")
                kwords_Begin.add("".join([Begin,Name]))
                kwords_End.add("".join([End,Name]))
                kwords.add("".join([Begin,Name]))
                kwords.add("".join([End,Name]))
                Inserted  = True
                BeginName = "".join([Begin,Name])
            if (typec and typec=='environment'):
                ## en interface only
                    Begin = 'start'
                    End   = 'stop'
                    Name  = check(name,encoding)
                    kwords_Begin.add("".join([Begin,Name]))
                    kwords_End.add("".join([End,Name]))
                    kwords.add("".join([Begin,Name]))
                    kwords.add("".join([End,Name]))
                    Inserted  = True
                    StartName = "".join([Begin,Name])
            if not(Inserted):
                kwords.add(name)
                SimpleName = name 
                if level:
                    if level=='system'   : kwords_System.add(name)
                    if level=='document' : kwords_Document.add(name)
                    if level=='style'    : kwords_Style.add(name)
                else:
                    kwords.add(name)
                    kwords_Other.add(name)
            ##
            ## For context.xml
            ##
            if arguments is not None:
                ##
                ## There can be more than one keywords,
                ## and if some Begin|Start|SimpleName is None,
                ## later we will delete the argshash[None]
                ## 
                argshash[BeginName]  = [] 
                argshash[StartName]  = [] 
                argshash[SimpleName] = [] 
                #keywords = arguments.find(cd+'keywords')
                for node in arguments:
                    if node.tag ==(cd+'text'):
                        text = node 
                        argshash[BeginName].append({'text':{}})
                        argshash[StartName].append({'text':{}})
                        argshash[SimpleName].append({'text':{}})

                    if node.tag ==(cd+'template'):
                        template = node 
                        argshash[BeginName].append({'template':{}})
                        argshash[StartName].append({'template':{}})
                        argshash[SimpleName].append({'template':{}})
                        optattr = template.get('optional')
                        if optattr:
                            argshash[BeginName][-1]['template']['optional']  = optattr
                            argshash[StartName][-1]['template']['optional']  = optattr
                            argshash[SimpleName][-1]['template']['optional'] = optattr

                    if node.tag ==(cd+'position'):
                        position = node 
                        argshash[BeginName].append({'position':{}})
                        argshash[StartName].append({'position':{}})
                        argshash[SimpleName].append({'position':{}})
                        listattr = position.get('list')
                        if listattr:
                            argshash[BeginName][-1]['position']['list']  = listattr
                            argshash[StartName][-1]['position']['list']  = listattr
                            argshash[SimpleName][-1]['position']['list'] = listattr

                    if node.tag ==(cd+'delimiter'):
                        delimiter = node 
                        argshash[BeginName].append({'delimiter':{}})
                        argshash[StartName].append({'delimiter':{}})
                        argshash[SimpleName].append({'delimiter':{}})
                        delimitattr = delimiter.get('name')
                        if delimitattr:
                            argshash[BeginName][-1]['delimiter']['name']  = delimitattr
                            argshash[StartName][-1]['delimiter']['name']  = delimitattr
                            argshash[SimpleName][-1]['delimiter']['name'] = delimitattr
                        
                    if node.tag ==(cd+'csname'):
                        argshash[BeginName].append({'csname':{}})
                        argshash[StartName].append({'csname':{}})
                        argshash[SimpleName].append({'csname':{}})
                        
                    if node.tag ==(cd+'content'):
                        content = node
                        argshash[BeginName].append({'content':{}})
                        argshash[StartName].append({'content':{}})
                        argshash[SimpleName].append({'content':{}})
                        delimiters = content.get('delimiters')
                        lists      = content.get('list')
                        optional   = content.get('optional')
                        if delimiters:
                            argshash[BeginName][-1]['content']['delimiters']  = delimiters
                            argshash[StartName][-1]['content']['delimiters']  = delimiters
                            argshash[SimpleName][-1]['content']['delimiters'] = delimiters
                        if lists:
                            argshash[BeginName][-1]['content']['lists']  = lists
                            argshash[StartName][-1]['content']['lists']  = lists
                            argshash[SimpleName][-1]['content']['lists'] = lists
                        if optional:
                            argshash[BeginName][-1]['content']['optional']  = optional
                            argshash[StartName][-1]['content']['optional']  = optional
                            argshash[SimpleName][-1]['content']['optional'] = optional
                    if node.tag ==(cd+'assignments'):
                        assignments = node 
                        argshash[BeginName].append({'assignments':{}})
                        argshash[StartName].append({'assignments':{}})
                        argshash[SimpleName].append({'assignments':{}})
                        argshash[BeginName][-1]['assignments']['param'] = {} 
                        argshash[StartName][-1]['assignments']['param'] = {} 
                        argshash[SimpleName][-1]['assignments']['param'] = {}
                        argshash[BeginName][-1]['assignments']['param']['parameter_name'] = []
                        argshash[StartName][-1]['assignments']['param']['parameter_name'] = [] 
                        argshash[SimpleName][-1]['assignments']['param']['parameter_name'] = []
                            
                        delimiters = assignments.get('delimiters')
                        lists      = assignments.get('list')
                        optional   = assignments.get('optional')
                        i=0
                        for parameter in assignments.findall('*'):
                            argshash[BeginName][-1]['assignments']['param']['parameter_name'].append([])
                            argshash[StartName][-1]['assignments']['param']['parameter_name'].append([])
                            argshash[SimpleName][-1]['assignments']['param']['parameter_name'].append([])
                            name = parameter.get('name')
                            if parameter.tag == (cd+'inherit'):
                                name="".join(['INHERIT:\\',name])
                            argshash[BeginName][-1]['assignments']['param']['parameter_name'][i].append(name)
                            argshash[StartName][-1]['assignments']['param']['parameter_name'][i].append(name)
                            argshash[SimpleName][-1]['assignments']['param']['parameter_name'][i].append(name)
                            for child in parameter:
                                if child.tag == (cd+'inherit'):
                                    v = "".join(['INHERIT:\\',child.get('name')])
                                else:
                                    prefix=child.get('prefix','')
                                    method=child.get('method','')
                                    v = child.get('type')
                                    v = remap_ctx_types.get(v,v)
                                    if len(prefix):
                                        prefix= remap_ctx_types.get(prefix,prefix)
                                        method= remap_ctx_methods.get(method,method)
                                        v = prefix+method+v
                                argshash[BeginName][-1]['assignments']['param']['parameter_name'][i].append(v)
                                argshash[StartName][-1]['assignments']['param']['parameter_name'][i].append(v)
                                argshash[SimpleName][-1]['assignments']['param']['parameter_name'][i].append(v)
                            i=i+1
                        if delimiters:
                            argshash[BeginName][-1]['assignments']['delimiters']  = delimiters
                            argshash[StartName][-1]['assignments']['delimiters']  = delimiters
                            argshash[SimpleName][-1]['assignments']['delimiters'] = delimiters
                        if lists:
                            argshash[BeginName][-1]['assignments']['lists']  = lists
                            argshash[StartName][-1]['assignments']['lists']  = lists
                            argshash[SimpleName][-1]['assignments']['lists'] = lists
                        if optional:
                            argshash[BeginName][-1]['assignments']['optional']  = optional
                            argshash[StartName][-1]['assignments']['optional']  = optional
                            argshash[SimpleName][-1]['assignments']['optional'] = optional
                                    
                    if node.tag ==(cd+'keywords'):
                        keywords = node
                        argshash[BeginName].append({'keywords':{}})
                        argshash[StartName].append({'keywords':{}})
                        argshash[SimpleName].append({'keywords':{}})
                        argshash[BeginName][-1]['keywords']['values']  = []
                        argshash[StartName][-1]['keywords']['values']  = []
                        argshash[SimpleName][-1]['keywords']['values'] = []
                        delimiters = keywords.get('delimiters')
                        lists      = keywords.get('list')
                        optional   = keywords.get('optional')
                        for child in keywords.iterfind('*'):
                            if child.tag == (cd+'inherit'):
                                v = "".join(['INHERIT:\\',child.get('name')])
                            else:
                                type_prefix=child.get('prefix','')
                                type_prefix=remap_ctx_types.get(type_prefix,type_prefix)
                                type_method=child.get('method','')
                                type_method=remap_ctx_method.get(type_method,type_method)
                                v = child.get('type')
                                v = type_prefix+type_method+remap_ctx_types.get(v,v)
                            argshash[BeginName][-1]['keywords']['values'].append(v)
                            argshash[StartName][-1]['keywords']['values'].append(v)
                            argshash[SimpleName][-1]['keywords']['values'].append(v)
                        if delimiters:
                            argshash[BeginName][-1]['keywords']['delimiters']  = delimiters
                            argshash[StartName][-1]['keywords']['delimiters']  = delimiters
                            argshash[SimpleName][-1]['keywords']['delimiters'] = delimiters
                        if lists:
                            argshash[BeginName][-1]['keywords']['lists']  = lists
                            argshash[StartName][-1]['keywords']['lists']  = lists
                            argshash[SimpleName][-1]['keywords']['lists'] = lists
                        if optional:
                            argshash[BeginName][-1]['keywords']['optional']  = optional
                            argshash[StartName][-1]['keywords']['optional']  = optional
                            argshash[SimpleName][-1]['keywords']['optional'] = optional

            ##
            #if argshash.has_key(None): del argshash[None]
            if not None in argshash: del argshash[None]
#

kwordl_Begin  = sorted(list(kwords_Begin))
kwordl_End  = sorted(list(kwords_End))
kwordl_System  = sorted(list(kwords_System))
kwordl_Document  = sorted(list(kwords_Document))
kwordl_Style  = sorted(list(kwords_Style))
kwordl_Other  = sorted(list(kwords_Other))

#for name in kwords : kwordl.append(name)
#kwordl.sort()
kwordl = sorted(list(kwords))

Keywords_1.text  = "\\" + str.join(" \\",kwordl_System)
Keywords_2.text  = "\\" + str.join(" \\",kwordl_Document)
Keywords_3.text  = "\\" + str.join(" \\",kwordl_Style)
Keywords_4.text = "\\" + str.join(" \\",kwordl_Other)
Keywords_5.text = '^\place ^\set ^\load'
Keywords_6.text = r'\abbreviation \regular \emphasis \important \quran \booktitle \translationraw \transliterationraw \SURAHAYAH \AYAHSURAH \DateHC \Reign \CircaHC \WafatHC \WafatAH \WafatCE \SLMraw \SLWraw \AJ \SLM \SLW \CE \HS \AH \OR \Or \OR \Or \OR\ \Or\ \Wafat \Ol \OL \Sl \SL \IE \TRANSLATE \TRANSLITERATE \Oll \Sll \BC \CircaBC \FullDate \addresser \lettersubject \letteropening \IeS \OLl \transliteration \translation'
Keywords_7.text  = "\\" + str.join(" \\",kwordl_Begin)
Keywords_8.text  = "\\" + str.join(" \\",kwordl_End)


##
## Lexer
##
LexerStyles = ET.XML("".join([
     r'<LexerStyles>',
      r'<LexerType desc="ConTeXt" excluded="no" ext="" name="ConTeXt">',
            r'<WordsStyle bgColor="D3D5C5" fgColor="1E2D2E" fontName="ALM Fixed" fontSize="14" fontStyle="0" name="DEFAULT" nesting="0" styleID="0" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="718076" fontName="ALM Fixed" fontSize="14" fontStyle="2" name="LINE COMMENTS" nesting="0" styleID="1" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="D33682" fontName="ALM Fixed" fontSize="14" fontStyle="1" name="TEX" nesting="0" styleID="2" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="6C71C4" fontName="ALM Fixed" fontSize="14" fontStyle="1" name="ETEX" nesting="0" styleID="3" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="CB4B16" fontName="ALM Fixed" fontSize="14" fontStyle="1" keywordClass="0" name="LUATEX" nesting="0" styleID="4" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="FF0080" fontName="ALM Fixed" fontSize="14" fontStyle="0" keywordClass="1" name="SYSTEM" nesting="0" styleID="5" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="268BD2" fontName="ALM Fixed" fontSize="14" fontStyle="1" keywordClass="2" name="DOCUMENT" nesting="0" styleID="6" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="DC322F" fontName="ALM Fixed" fontSize="14" fontStyle="1" keywordClass="3" name="STYLE" nesting="0" styleID="7" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="FF0080" fontName="ALM Fixed" fontSize="14" fontStyle="0" keywordClass="4" name="CHARDEF" nesting="0" styleID="8" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="FF0080" fontName="ALM Fixed" fontSize="14" fontStyle="0" keywordClass="5" name="CONSTRUCT" nesting="0" styleID="9" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="FF0080" fontName="ALM Fixed" fontSize="14" fontStyle="0" keywordClass="6" name="PRIVATE" nesting="0" styleID="10"/>',
            r'<WordsStyle bgColor="D3D5C5" fgColor="2AA198" fontName="ALM Fixed" fontSize="14" fontStyle="1" keywordClass="7" name="START OPEN" nesting="0" styleID="11"/>',
            r'<WordsStyle bgColor="D3D5C5" fgColor="2AA198" fontName="ALM Fixed" fontSize="14" fontStyle="1" keywordClass="8" name="STOP CLOSE" nesting="0" styleID="12"/>',
            r'<WordsStyle bgColor="D3D5C5" fgColor="A12A3D" fontName="Courier New" fontSize="14" fontStyle="1" name="OPERATORS" nesting="0" styleID="13" />',
            r'<WordsStyle bgColor="D3D5C5" fgColor="A12A33" fontName="ALM Fixed" fontSize="14" fontStyle="1" name="NUMBERS" nesting="0" styleID="14" />',
            r'</LexerType>',
    r'</LexerStyles>'])
)

NotepadPlus.append(ET.Comment(' TODO: Fix the styles '));
NotepadPlus.append(LexerStyles)


## write  the xml file 
#  ConTeXt.xml
outtree = prettify(NotepadPlus,encoding)
outfilename = 'ConTeXt.xml'
outfile = open(outfilename,'w')
outfile.write(outtree)
outfile.close()
print('wrote ' + outfilename)

## Build the output tree
## context.xml
encoding = 'Windows-1252'

NotepadPlus  = ET.Element('NotepadPlus')
AutoComplete = ET.SubElement(NotepadPlus,'AutoComplete',{'language': "ConTeXt"})
#Environment  = ET.SubElement(AutoComplete,'Environment',
#                             {'ignoreCase':"yes", 'startFunc':'&nbsp;',
#                              'stopFunc':'0', 'additionalWordChar':'\\'})
Environment  = ET.fromstring('''<Environment additionalWordChar="\" ignoreCase="no" startFunc="&#x7f;"
                   startFunc1="[" startFunc2="{" startFunc3="(" stopFunc="&#x7f;" stopFunc1="]" stopFunc2="}" stopFunc3=")"
                   calltipFontName="consolas" calltipFontSize="9"
                   calltipForeColor="0x101010" calltipBackColor="0xF0F0F0"
                   sortMacroValues="no" maxLineLength="80"
                   toplines="1" macroValueOnSingleLine="yes"
                   columns="2" 
                   widthColumnSep="2" thresholdNrOfRows="20"
                   />''')

AutoComplete.append(Environment)

for name in kwordl:
    Keyword = ET.SubElement(AutoComplete,'KeyWord',{'ctxname':"".join(["\\",name])})
    if argshash.has_key(name):
        Keyword.set('func','yes')
        Overload = ET.SubElement(Keyword,'Overload',{'retVal':''})
        paramset_nr=0;
        for args in argshash[name]:
            if args.has_key('text'):
                ctxtext = args['text']
                Overload.append(ET.Comment(' TEXT '))
                val_enc = '_TEXT_'
                paramset_nr=paramset_nr+1
                Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),'ctxname':val_enc})

            if args.has_key('template'):
                ctxtemplate = args['template']
                Overload.append(ET.Comment(' TEMPLATE: optional=' + ctxtemplate.get('optional',' ')))
                val_enc = '|_TEMPLATE_|'
                paramset_nr=paramset_nr+1
                Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),
                                                         'optional':ctxtemplate.get('optional',''),
                                                         'ctxname':val_enc})

            if args.has_key('position'):
                ctxposition = args['position']['list']
                Overload.append(ET.Comment(' POSITION: list=' + ctxposition))
                if ctxposition=='yes' :
                    val_enc = '(_POSITION_,...)'
                else:
                    val_enc = '(_POSITION_)'
                paramset_nr=paramset_nr+1
                Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),
                                                         'list':ctxposition,
                                                         'ctxname':val_enc})

            if args.has_key('delimiter'):
                ctxdelimiter = args['delimiter']['name']
                Overload.append(ET.Comment(' DELIMITER: name=' + ctxdelimiter))
                val_enc = check(ctxdelimiter,encoding)
                paramset_nr=paramset_nr+1
                Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),
                                                         'delimiter':ctxdelimiter,
                                                         'ctxname':'\\'+val_enc})

            if args.has_key('csname'):
                ctxcsname = args['csname']
                Overload.append(ET.Comment(' CSNAME'))
                paramset_nr=paramset_nr+1
                Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),'ctxname':'\\_CSNAME_'})

            if args.has_key('content'):
                ctxcontent = args['content']
                Overload.append(ET.Comment(' CONTENT delimiters:' + ctxcontent.get('delimiters','')))
                Overload.append(ET.Comment(' CONTENT lists:' + ctxcontent.get('lists','')))
                Overload.append(ET.Comment(' CONTENT optional:' + ctxcontent.get('optional','')))
                #Overload = ET.SubElement(Keyword,'Overload',{'retVal':''})
                paramset_nr=paramset_nr+1
                Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),
                                                         'delimiters': ctxcontent.get('delimiters',''),
                                                         'lists': ctxcontent.get('lists',''),
                                                         'optional': ctxcontent.get('optional',''),
                                                         'ctxname': '_CONTENT_'})
                
            if args.has_key('assignments'):
                assigns        = args['assignments']
                ctxparamsname = args['assignments']['param']['parameter_name']
                if len(ctxparamsname) or len(ctxinherit): 
                        paramset_nr=paramset_nr+1
                for pvect in (ctxparamsname):
                    k = pvect[0]
                    val_enc = ''
                    if len(pvect[1:]):
                        val_enc = check("|".join([remap_ctx_types.get(i,i) for i in pvect[1:]]),encoding)
                        val_enc = "= "+val_enc
                    k_enc   = check(k,encoding) 
                    Param = ET.SubElement(Overload,'Param',
                                          {'paramset':str(paramset_nr),
                                           'delimiters':assigns.get('delimiters',''),
                                           'lists':assigns.get('lists',''),
                                           'optional':assigns.get('optional',''),
                                           'ctxname':remap_ctx_types.get(k_enc,k_enc)+val_enc})
            if args.has_key('keywords'):
                ctxkeywords = args['keywords']
                v = ctxkeywords['values']
                if len(v):
                    paramset_nr=paramset_nr+1
                for value in (list(v)):
                    #Overload = ET.SubElement(Keyword,'Overload',{'retVal':''})
                    if value is not None :
                        val_enc = check(value,encoding)
                        Param = ET.SubElement(Overload,'Param', {'paramset':str(paramset_nr),
                                                                 'delimiters':  ctxkeywords.get('delimiters',''),
                                                                 'lists': ctxkeywords.get('lists',''),
                                                                 'optional' : ctxkeywords.get('optional',''),
                                                                 'ctxname':remap_ctx_types.get(val_enc,val_enc)})
            
                #if len(argshash[name])>1:        
                #    Overload = ET.SubElement(Keyword,'Overload',{'retVal':''})
                #    Param = ET.SubElement(Overload,'Param', {'name':'%%%%%%%%%%%%%%%%%%%%%%%'+str(argshash[name])})
        #
        Overload.set("paramsets",str(paramset_nr))

outtree = prettify(NotepadPlus,encoding)
if not os.path.exists('APIs'):
    os.makedirs('APIs')

outfile = open('APIs/context.xml','w')
outfile.write(outtree)
outfile.close()
print('wrote context.xml')

## clean-up files from primitives.lua
if os.path.isfile('prm-etex.txt')   : os.remove('prm-etex.txt')
if os.path.isfile('prm-luatex.txt') : os.remove('prm-luatex.txt')
if os.path.isfile('prm-tex.txt')    : os.remove('prm-tex.txt')

## clean-up files from macro-chardef.tlua
if os.path.isfile('macro-chardef.txt')   : os.remove('macro-chardef.txt')
