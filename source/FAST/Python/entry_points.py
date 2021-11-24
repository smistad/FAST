import subprocess
import sys
import os

bin_path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'bin'))

def runUFFviewer():
    subprocess.run([os.path.join(bin_path, 'UFFviewer')] + sys.argv[1:])

def runSystemCheck():
    subprocess.run([os.path.join(bin_path, 'systemCheck')] + sys.argv[1:])

def runRunPipeline():
    subprocess.run([os.path.join(bin_path, 'runPipeline')] + sys.argv[1:])
