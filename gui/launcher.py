import os
import subprocess
import errno
import atexit


DEFAULT_CONFIG = 'settings/D19c.xml'

EXECUTABLES = {
    'bin/exe1' : ['arg1', 'arg2'],
    # 'gui/test_pipe' : []
}

PIPE_NAME = '/tmp/Ph2_ACF_' + str(os.getpid())


# setup named pipe
try:
    os.mkfifo(PIPE_NAME)
except OSError as e:
    if e.errno != errno.EEXIST:
        raise

def _delete_pipe():
    os.remove(PIPE_NAME)
    # print('pipe removed')

atexit.register(_delete_pipe)



def open_pipe():
    return open(PIPE_NAME)


class Launcher:
    _dir = lambda path: os.path.dirname(path)

    def __init__(self, base_dir=_dir(_dir(os.path.realpath(__file__)))):
        self.base_dir = base_dir
        self.config_file = DEFAULT_CONFIG

    def set_config(self, config_file):
        self.config_file = config_file

    def launch(self, executable, args=[]):
        args += self._get_args(executable)
        return subprocess.Popen([os.path.join(self.base_dir, executable)] + args, cwd=self.base_dir)

    def _get_args(self, executable):
        return ['-c', self.config_file, '-p', PIPE_NAME] + list(EXECUTABLES.get(executable, []))