import cmd
import os

GIT_DIR = '.git'

class GitShell(cmd.Cmd):
    intro = "Welcome to sandbox git shell. Type help or ? to list command."
    prompt = "(git) "

    def do_exit(self, arg):
        'Exit the git interactive sandbox shell.'
        if check_exists(GIT_DIR):
            print(f"Removing: {GIT_DIR}")
            os.rmdir(GIT_DIR)
        return True
    
    def do_init(self, arg):
        '"git init" equivalent'
        if not check_exists(GIT_DIR):
            os.mkdir(GIT_DIR)
        
def check_exists(dir):
    if os.path.exists(dir):
        return True
    return False