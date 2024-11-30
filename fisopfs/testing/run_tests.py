#!/usr/bin/python3

import os
import shutil
import sys
import re
import tempfile
import yaml
import time

from os import listdir
from os.path import isfile, join
from yaml.loader import SafeLoader
from subprocess import Popen, PIPE, STDOUT
from termcolor import cprint
import threading,shutil


TIME_BETWEEN_STEPS = 80 #MS 
TIME_BETWEEN_STEPS = TIME_BETWEEN_STEPS/1000 #Sec

TIME_BETWEEN_MOUNTS = 500 #MS 
TIME_BETWEEN_MOUNTS = TIME_BETWEEN_MOUNTS/1000 #Sec


GIVEN = "given"


RESET_MARK = "serialize"
STEPS = "steps"
DO = "do"
RET = "ret"
OUT = "out"
IN = "in"

EXPR_MOUNT = "{fs}/"

FAIL_FAST = False
SHOW_LOG_FS = False

fs_binary = "./fisopfs" # default
reflector_binary = "testing/reflector" #default
mount_point = "prueba/" # default
out_serial = "test_serial.fisopfs" # default

def write_out_to(name, content):
   name = name.replace(" " ,"_").replace("/","_")
   name = "./testing/out/out_"+name
   print("WRITING STDOUT OF FS TO ",name)
   
   hnd = open(name, "w+")
   hnd.write(content.decode() if content else "NONE")
   hnd.close()

def write_err_to(name, content):
   name = name.replace(" " ,"_").replace("/","_")
   name = "./testing/out/err_"+name
   print("WRITING STDERR OF FS TO ",name)
   
   hnd = open(name, "w+")
   hnd.write(content.decode() if content else "NONE")
   hnd.close()


def mount_normal_fs():
    if os.path.exists(mount_point):
       shutil.rmtree(mount_point) # por las dudas

    p= Popen("mkdir "+mount_point, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    (stdout, stderr) = p.communicate()    
    if p.returncode != 0:
       print("----createdir stdout:\n",str(stdout))
       print("----createdir stderr:\n",str(stderr))
       raise Exception("Failed create of dir!!")

def unmount_normal_fs():
    p= Popen("rm -r "+mount_point, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    p.communicate()
    return p.returncode
    #if p.returncode != 0:
       #raise Exception("Failed umount of filesystem! Directorie not removed?")


def get_comm():
    return fs_binary+" --filedisk "+out_serial+" -f "+mount_point

def fs_wait(process, test):
    (test.fs_out, test.fs_err) = process.communicate()
    test.fs_ret =process.returncode 
    
    if test.fs_ret == 0 and SHOW_LOG_FS:
       print("----mount out:")
       write_out_to(test.name,test.fs_out)
       

def mount_fs(command):
    mount_normal_fs()
    
    return Popen(command, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)

def umount_fs():
    comm = "umount "+mount_point
    p= Popen(comm, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    (stdout, stderr) = p.communicate()
    if p.returncode != 0:
       print("Unmounting command:",comm)
       
       print("----unmount out:")
       print(stdout.decode()if stdout else "NONE!")
       print("----unmount err:")
       print(stderr.decode() if stderr else "NONE!")
       #raise Exception("Failed umount of filesystem!")

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
    print(step)
    p = Popen(step.command, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    
    if step.stdin:
        (stdout, stderr) = p.communicate(input = step.stdin.encode(),timeout=100)
    else:
        (stdout, stderr) = p.communicate(timeout=100)        
    step.check(p.returncode, stdout, stderr)
    time.sleep(TIME_BETWEEN_STEPS)

class FilesystemTest():
    def __init__(self, filepath: str):
        data = ""
        with open(filepath) as f:
            data = yaml.load(f, Loader=SafeLoader)

        self.name = data['name']
        self.description = data['description']
        self.givens = ""
        self.steps = []
        self.breakpoint = -1
        
        self.fs_out = None
        self.fs_err = None
        self.fs_ret = 0
        
        self.thread_fs = None
        self.serial_file = data.get('serial_file', out_serial)
        
        if not STEPS in data:
            print("Invalid fs test!")
            return
            
        if GIVEN in data:
            self.givens = "\n".join(data[GIVEN])
        
        for itm in data[STEPS]:
            action = itm.get(DO, None)
            if(action == None):
               continue
            if action == RESET_MARK:
               self.breakpoint = len(self.steps)
               continue
            
            self.steps.append(resolve_step(action, itm))
    
    def mount_command(self):
        return fs_binary+" --filedisk "+self.serial_file+" -f "+mount_point
    
    def mount(self):               
        comm = self.mount_command()
        print(comm)
        proc = mount_fs(comm)
        proc.poll()
        self.thread_fs = threading.Thread(target = fs_wait, args = (proc,self)) 
        self.thread_fs.start()
    
    def unmount(self):
        umount_fs()
        self.thread_fs.join()
        return self.fs_ret != 0 # denotar que fallo
        
        

    def rerun(self, ind ):
        launch_step(self.steps[ind])
        return self.run(ind+1)
        
    def run(self, first = 0):        
        curr = first
        for step in self.steps[first:]:
            if(self.breakpoint == curr):
                print("----------SERIALIZE/DESERIALIZE!!!!")
                return curr
            launch_step(step)
            curr+=1
        
        return -1


def run_test(test):
    if os.path.exists(test.serial_file):
         print("----> CLEAN SERIAL FILE",test.serial_file)
         os.remove(test.serial_file) # por las dudas
    
    print("----> WAIT BEFORE MOUNT AGAIN!")
    time.sleep(TIME_BETWEEN_MOUNTS)
    test.mount()
    
    ind = test.run()
    
    if test.unmount():
       # Fallo el unmount hubo algun segmentation fault
       return ind>=0
    
    while(ind >=0):
        print("----> WAIT BEFORE MOUNT AGAIN!")
        time.sleep(TIME_BETWEEN_MOUNTS)
        
        test.mount()
        ind = test.rerun(ind)        
        if test.unmount():
           # Fallo el unmount hubo algun segmentation fault
           return True
    
    return False
            
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
        try:
            if run_test(test):
                msg = "FAIL {}/{}: {} ({}). Exception at filesystem!".format(count, total, test.description, test.name, );
                cprint(msg, "red")
                fails+= "\n"+msg;
                failed += 1
                write_out_to(test.name,test.fs_out)
                write_err_to(test.name,test.fs_err)
            
            cprint("PASS {}/{}: {} ({})\n".format(count, total, test.description, test.name), "green")
        except Exception as e:
            #print("----------------->FS STDOUT")            
            msg = "FAIL {}/{}: {} ({}). Exception ocurred: {}".format(count, total, test.description, test.name, e);
            cprint(msg, "red")
            
            fails+= "\n"+msg;
            failed += 1
            test.unmount()
            
            write_out_to(test.name,test.fs_out)
            write_err_to(test.name,test.fs_err)
            
            if FAIL_FAST:
               raise e
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
       
       if(len(self.exp_children) == 0):
           res = stdout.decode() if stdout else ""
           if(res != "" and res != None):
               raise Exception("Expected no children! for ls but there was output from ls! '"+res+"'")
           return
           
       if(stdout == None):
           raise Exception("Expected children! none on ls out")
       res = stdout.decode()
       
       itms = res.split("\n")[0:-1] # There is a final '' item.
       
       if len(itms) != len(self.exp_children):
           raise Exception("Expected count of children different got: "+str(len(itms))+" wanted: "+str(len(self.exp_children))+"::\n'"+res+"'")
       
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
        return self.command
        

def resolve_step(action, item):
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



