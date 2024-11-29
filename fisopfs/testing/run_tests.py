#!/usr/bin/python3

import os
import shutil
import sys
import re
import tempfile
import yaml

from os import listdir
from os.path import isfile, join
from yaml.loader import SafeLoader
from subprocess import Popen, PIPE, STDOUT
from termcolor import cprint

GIVEN = "given"

STEPS = "steps"
DO = "do"
RET = "ret"
OUT = "out"
IN = "in"

EXPR_MOUNT = "{fs}/"

fs_binary = "./fisopfs" # default
reflector_binary = "testing/reflector" #default
mount_point = "prueba/" # default

def mount_normal_fs():
    p= Popen("mkdir "+mount_point, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    p.communicate()
    if p.returncode != 0:
       raise Exception("Failed mount of filesystem!")

def unmount_normal_fs():
    p= Popen("rm -r "+mount_point, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    p.communicate()
    if p.returncode != 0:
       raise Exception("Failed umount of filesystem!")


def mount_fs():
    mount_normal_fs();

def umount_fs():
    unmount_normal_fs();








def compare_strings(current, expected):
    if current != expected:
        message="""
        Expected:

        {}

        But got:
        
        {}
        """.format(expected.encode() if expected else None, current.encode() if expected else None)
        return message
    return None


def launch_step(step):
    print("->run:",step)
    p = Popen(step.command, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    
    if step.stdin:
        (stdout, stderr) = p.communicate(input = step.stdin.encode(),timeout=100)
    else:
        (stdout, stderr) = p.communicate(timeout=100)        
    step.check(p.returncode, stdout, stderr)

class FilesystemTest():
    def __init__(self, filepath: str):
        data = ""
        with open(filepath) as f:
            data = yaml.load(f, Loader=SafeLoader)

        self.name = data['name']
        self.description = data['description']
        self.givens = ""
        self.steps = []
        if not STEPS in data:
            print("Invalid fs test!")
            return
            
        if GIVEN in data:
            self.givens = "\n".join(data[GIVEN])
        
        for itm in data[STEPS]:
            self.steps.append(resolve_step(itm))
        
    def run(self):
        if(self.givens != ""):
            print("Given:",self.givens, "\n------\n")
            
        for step in self.steps:
            launch_step(step)
            
            
def run_tests(tests):

    #tempdir = tempfile.mkdtemp(suffix='-fs-test')

    # Sort for consistent ordering
    tests.sort()

    count = 1
    failed = 0
    total = len(tests)
    
    fails = "";

    for test_path in tests:
        test = FilesystemTest(test_path)#, subs_map)
        
        mount_fs()
        
        try:
            test.run()
            umount_fs()
            cprint("PASS {}/{}: {} ({})\n".format(count, total, test.description, test.name), "green")
        except Exception as e:
            msg = "FAIL {}/{}: {} ({}). Exception ocurred: {}".format(count, total, test.description, test.name, e);
            cprint(msg, "red")
            fails+= "\n"+msg;
            failed += 1
            umount_fs()
            #raise e
        finally:
            count += 1

    cprint("{} out of {} tests passed".format(total - failed, total), "yellow" if failed else "green")
    if fails != "":
        cprint("Summary of errors:")
        cprint(fails, "red")
    #shutil.rmtree(tempdir)

def run_all_tests():
    test_files_path = "./testing/specs"

    tests = [join(test_files_path, f) for f in listdir(test_files_path) if isfile(join(test_files_path, f))]

    run_tests(tests)

def run_single_test(test_name: str):
    test_path = "./testing/specs/{}.yaml".format(test_name)

    run_tests([test_path])


##### Steps/tests de acciones/comandos en si.
#####
class TestSuccesfulExec:
    def __init__(self, command : str):
        self.command = command#.replace(EXPR_MOUNT, mount_point)
        self.stdin = None
    def __repr__(self):
       return self.command
    def check(self, ret, stdout, stderr):       
       if ret != 0:
          raise Exception("Check "+self.command+" was succesful failed!")

class TestStep:
    def __init__(self, command : str):
        self.command = command.replace(EXPR_MOUNT, mount_point)
        self.ret = 0
    def __init__(self, item,  command : str):
        self.command = command.replace(EXPR_MOUNT, mount_point)
        
        # Se podria usar lambdas/clases para construir pero se va a usar ifs y ya.
        self.ret = item.get(RET, None)
        self.out = item.get(OUT, None)
        self.stdin = item.get(IN, None)
        
    def __repr__(self):
        return self.command+"\nin: "+self.stdin if self.stdin else self.command
    
    def check(self, ret, stdout, stderr):       
       if self.ret != None and self.ret != ret:
          raise Exception("Check '"+self.command+"' expected ret "+str(self.ret)+" was: "+str(ret))
          
       if self.out != None:
          if len(self.out) == 0:
             msg = "Expected no input got "+stdout if stdout else None
          else:
             if stdout:
                 msg = compare_strings(stdout.decode(), self.out)
             else:
                 msg = "Expected input but got none! "
                
          
          if msg:
            raise Exception("Check out failed "+msg)



CHILDREN = "children"

class LsStep(TestStep):
    def __init__(self, item, command):
       super().__init__(item, command)
       self.exp_children = []
       for child in item.get(CHILDREN, []):
           self.exp_children.append(str(child))
       
    
    def check(self, ret, stdout, stderr):
       super().check(ret, stdout, stderr)
       res = stdout.decode()
       
       if(len(self.exp_children) == 0):
           if(res != None and res != ""):
               raise Exception("Expected no children! for ls but there was output from ls! '"+res+"'")
           return
       if(res == None):
           raise Exception("Expected children! none on ls out")
       
       itms = res.split("\n")[0:-1] # There is a final '' item.
       
       if len(itms) != len(self.exp_children):
           raise Exception("Expected count of childre different got: "+str(len(itms))+" wanted: "+str(len(self.exp_children)))
       
       for child in itms:
           if not child in self.exp_children:
                raise Exception("Unexpected child "+child)
    




K_PATH = "path"
K_TYPE = "type"
K_SIZE = "size"
K_BLOCKS = "blocks"
K_INODE = "inode"
K_REFS = "refs"
K_PERM = "perm"

ANY_PATH= "(?:[\w\d]+/)*[\w\d]+"
ANY_VL= "(?:\w+\s*)+"
ANY_NUM= "\d+"

line_sep = "\s*\n\s*"

base_reg = line_sep.join([
    "\s*File:\s*{path}",
    "\s*".join([
        "Size:\s*{size}",
        "Blocks:\s*{blocks}",
        "IO\sBlock: {io_block}",
        "{type_f}",
    ]),    
    "Device:\s*{dev}\s*Inode:\s*{inode}\s*Links:\s*{refs}\s*Access:\s*\({acc}"
    ]
) 

class StatStep(TestStep):
    def __init__(self, item, command):
       super().__init__(item, command)
       
       self.itm = item
       self.path = item.get(K_PATH, ANY_PATH)
       if self.path != ANY_PATH:
           self.path = self.path.replace(EXPR_MOUNT, mount_point)
                  
       size = item.get(K_SIZE, ANY_NUM)
       type_f = item.get(K_TYPE, ANY_VL)
       
       reg = base_reg.format(
            path = self.path,
            size = size,
            type_f = type_f,
            inode = item.get(K_INODE, ANY_NUM),
            acc = item.get(K_PERM, ANY_NUM),
            blocks = item.get(K_BLOCKS, ANY_NUM),
            refs = item.get(K_REFS, ANY_NUM),
            io_block = ANY_NUM,
            dev = ANY_PATH,
       )
       reg = reg.replace(" ", "\s")
       self.reg = re.compile(reg)
       
       
    def check(self, ret, stdout, stderr):
       super().check(ret, stdout, stderr)
       res = stdout.decode()
       
       if self.reg.match(res):
           return
       msg = "\nwith path: '"+self.path+"'"
       
       for vl in self.itm:
           if(vl != DO and vl != K_PATH):
                msg+= "\n with "+vl+": "+ str(self.itm[vl]) 
       raise Exception("Failed validation of stat expected: "+msg+"\n got: "+res)
       
       
          
    def __repr__(self):
        return "stat: "+self.command
       
def resolve_step(item):
    action = item[DO]
    
    command = action.split(" ", 2)[0]
    if command in special_commands:
       #print(command, "WAS special")
       return special_commands[command](item,action)
    
    return TestStep(item, action)


special_commands = {
    "stat" : StatStep,
    "ls" : LsStep,
}

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("{}: {} <filesystem-binary> <mount_point> <reflector>".format(sys.argv[0], sys.argv[0]))
        exit()

    fs_binary = sys.argv[1]
    mount_point = sys.argv[2]
    
    if mount_point[-1] != "/":
        mount_point+="/"
    
    reflector_binary = sys.argv[3]

    if len(sys.argv) == 5:
        run_single_test(sys.argv[4])
    else:
        run_all_tests()



