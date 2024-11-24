#!/usr/bin/python3

import os
import shutil
import sys
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


def has_name(output, name):
    print("Contains name",name,"OUT",out)
    return True;

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


def launch_fs():
    return Popen(fs_binary+" "+mount_point, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)

def show_fs(fs_popen):
    (stdout, stderr) = fs_popen.communicate(timeout=timeout)
    print("------------FS STDOUT:", stdout)
    print("------------FS STDERR:", stderr)

def launch_step(step):
    print("-------> run:",step.command)
    p = Popen(step.command, stdin=PIPE, stdout=PIPE, stderr=STDOUT, shell=True)
    
    if step.stdin:
        (stdout, stderr) = p.communicate(input = step.stdin.encode(),timeout=100)
    else:
        (stdout, stderr) = p.communicate(timeout=100)        
    step.check(0, stdout, stderr)

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
            #for itm in given[1:]:
            #    self.givens.append(itm.replace(EXPR_MOUNT, mount_point))
        
        for itm in data[STEPS]:
            self.steps.append(resolve_step(itm))
        
    def run(self):
        print("Given:",self.givens, "\n------\n")
        launch_step(TestSuccesfulExec("mkdir "+mount_point))
        for step in self.steps:
            launch_step(step)
            
        launch_step(TestSuccesfulExec("rm -r "+mount_point))
            
def run_tests(tests):

    #tempdir = tempfile.mkdtemp(suffix='-fs-test')

    #print("=== Temporary files will be stored in: {} ===\n".format(tempdir))

    #subs_map = [
    #    ('{shell_binary}', shell_binary),
    #    ('{tempdir}', tempdir),
    #    ('{reflector}', reflector_binary),
    #]

    # Sort for consistent ordering
    tests.sort()

    count = 1
    failed = 0
    total = len(tests)

    for test_path in tests:
        test = FilesystemTest(test_path)#, subs_map)
        try:
            test.run()
            cprint("PASS {}/{}: {} ({})".format(count, total, test.description, test.name), "green")
        except Exception as e:
            cprint("FAIL {}/{}: {} ({}). Exception ocurred: {}".format(count, total, test.description, test.name, e), "red")
            failed += 1
        finally:
            count += 1

    cprint("{} out of {} tests passed".format(total - failed, total), "yellow" if failed else "green")

    #shutil.rmtree(tempdir)

def run_all_tests():
    test_files_path = "./testing/specs"

    tests = [join(test_files_path, f) for f in listdir(test_files_path) if isfile(join(test_files_path, f))]

    run_tests(tests)

def run_single_test(test_name: str):
    test_path = "./testing/specs/{}.yaml".format(test_name)

    run_tests([test_path])

class TestSuccesfulExec:
    def __init__(self, command : str):
        self.command = command#.replace(EXPR_MOUNT, mount_point)
        self.stdin = None
    def check(self, ret, stdout, stderr):       
       if ret != 0:
          raise Exception("Check command was succesful failed!")

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
        return "do: "+self.command
    
    def check(self, ret, stdout, stderr):       
       if self.ret != None and self.ret != ret:
          raise Exception("Check ret code failed expected "+self.ret+" was: "+ret)
          
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

class StatStep(TestStep):
    def __init__(self, item, command):
       super().__init__(item, command)
    def check(self, ret, stdout, stderr):
       super().check(ret, stdout, stderr)
       print("-->Now stat validation!")
          
    def __repr__(self):
        return "stat: "+self.command
       
def resolve_step(item):
    action = item[DO]
    
    command = action.split(" ", 2)[0]
    if command in special_commands:
       print(command, "WAS special")
       return special_commands[command](item,action)
    
    return TestStep(item, action)


special_commands = {
    "stat" : StatStep
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

