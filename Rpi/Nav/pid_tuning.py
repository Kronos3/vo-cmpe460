import os
import traceback
import readline


def send_command(command, *args):
    print("%s %s" % (command, " ".join(args)))
    os.system("fprime-cli command-send "
              "-d dict/RpiRTLinuxTopologyAppDictionary.xml "
              "%s -a %s" % (command, " ".join(args)))


def set_parameter(component: str, prm_name: str, value):
    send_command("%s.%s_PRM_SET" % (component, prm_name), str(value))


class Commander:
    simple_to_actual_params = {
        "p": ("nav", "PID_P"),
        "i": ("nav", "PID_I"),
        "d": ("nav", "PID_D"),
        "t": ("nav", "PID_T"),
    }

    values = {
        "p": 0.5,
        "i": 0.5,
        "d": 0.5,
        "t": 0.5,
    }

    step: float

    def __init__(self):
        self.step = 0.1

    def set_step(self, value, *_):
        self.step = float(value)

    def increment(self, prm, *_):
        if prm not in self.simple_to_actual_params:
            print("prm not a valid param")
            return

        if prm not in self.values:
            print("prm has not been set yet")
            return

        new_val = self.values[prm] + self.step
        set_parameter(self.simple_to_actual_params[prm][0],
                      self.simple_to_actual_params[prm][1],
                      new_val)

        self.values[prm] = float(new_val)

    def decrement(self, prm, *_):
        if prm not in self.simple_to_actual_params:
            print("prm not a valid param")
            return

        if prm not in self.values:
            print("prm has not been set yet")
            return

        new_val = self.values[prm] - self.step
        set_parameter(self.simple_to_actual_params[prm][0],
                      self.simple_to_actual_params[prm][1],
                      new_val)

        self.values[prm] = float(new_val)

    def set(self, prm, value, *_):
        if prm not in self.simple_to_actual_params:
            print("prm not a valid param")
            return

        new_val = float(value)
        set_parameter(self.simple_to_actual_params[prm][0],
                      self.simple_to_actual_params[prm][1],
                      new_val)

        self.values[prm] = float(new_val)

    def save(self, *_):
        for _, (comp, prm) in self.simple_to_actual_params.items():
            send_command("%s.%s_PRM_SAVE" % (comp, prm))

        send_command("prmDb.PRM_SAVE_FILE")

    def print_help(self, *_):
        print("+ prm                increment parameter by step")
        print("- prm                decrement parameter by step")
        print("= prm val            set parameter to value")
        print(". val                set step size to value")
        print("c cmp.cmd args       send raw command to component")
        print("s                    save parameters to database and save database to file")
        print("h                    print help")

    def dispatch(self, command: str):
        command_m = command[0]
        command_handlers = {
            '+': self.increment,
            '-': self.decrement,
            '=': self.set,
            '.': self.set_step,
            'c': send_command,
            's': self.save,
            'h': self.print_help
        }

        if command_m[0] not in command_handlers:
            print("command %s not found" % command_m[0])
            return

        command_handlers[command_m[0]](*(command[1:].strip().split(' ')))


def main():
    cmd = Commander()
    while True:
        try:
            cmd.dispatch(input("> "))
        except KeyboardInterrupt:
            print("")
        except EOFError:
            return
        except Exception:
            print(traceback.format_exc())


if __name__ == "__main__":
    main()
