import os
from os import path
import subprocess
from argparse import ArgumentParser


def execute_command(args: list) -> tuple:
    process = subprocess.Popen(args, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    result, error = process.communicate()
    process.wait()
    return process.returncode, result, error


def compile_shader(src: str, t, ep, out: str) -> tuple:
    fxc_executable = path.abspath(path.join("C:\\", "Program Files (x86)", "Windows Kits", "10", "bin", "10.0.26100.0", "x64", "fxc.exe"))
    code, result, error = execute_command([fxc_executable, "/T", t, "/E", ep, "/Fo", out, src])
    if len(error):
        print(f"Error: {error}")
        exit(1)
    else:
        print(f"Compiled: {result}")
    return code, result, error


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('-o', '--outdir', type=str, default="./")
    args = parser.parse_args()

    shaders_dir = path.abspath(path.join(path.dirname(__file__), "hlsl"))
    outdir = path.abspath(args.outdir)

    print(f"Compiling Shader Modules into '{outdir}'")
    os.makedirs(outdir, exist_ok=True)

    compile_shader(path.join(shaders_dir, "vertex.hlsl"), t="vs_5_0", ep="VSMain", out=path.join(outdir, "vertex.vs.dxbc"))
    compile_shader(path.join(shaders_dir, "unity.hlsl"), t="ps_5_0", ep="PSMain", out=path.join(outdir, "unity.ps.dxbc"))

    print(f"Successfully compiled all Shader Modules to '{outdir}'")
    exit(0)
