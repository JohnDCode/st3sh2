import threading
import queue
import subprocess
import pty
import os
import select
import queue
import termios
import time
import sys

# Send input to shell
shellInput = queue.Queue()

# Recieve from shell
shellOutput = queue.Queue()

# Handle pty as a seperate thread
class ShellThread(threading.Thread):

    # Take path to executable
    def __init__(self, executable):
        super().__init__(daemon=True)

        self.executable = executable
        self.running = True
        self.master = None
        self.proc = None



    # Setup and maintain pty connection
    def run(self):

        # Spawn the shell
        master, slave = pty.openpty()
        self.master = master
        self.proc = subprocess.Popen(
            [self.executable],
            stdin=slave,
            stdout=slave,
            stderr=slave,
            preexec_fn=os.setsid,
            close_fds=True
        )
        os.close(slave)

        
        # Main pty loop
        while self.running:

            # Send input to shell
            try:
                while True:
                    data = shellInput.get_nowait()
                    if data:
                        os.write(master, data)
            except queue.Empty:
                pass


            # Recieve output from shell
            ready, _, _ = select.select([master], [], [], 0)
            if master in ready:
                try:
                    output = os.read(master, 4096)
                    if output:
                        shellOutput.put(output)

                except OSError:
                    break


            # Check if shell exited
            if self.proc.poll() is not None:
                break

            time.sleep(0.005)


        # Cleanup proc on exit
        os.close(master)
        self.proc.terminate()
        self.proc.wait()
        os._exit(0)



    # Despawn
    def stop(self):
        self.running = False
        self.proc.terminate()
        self.proc.wait()
