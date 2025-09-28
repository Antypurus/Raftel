#!env /usr/bin/python3

from pathlib import Path
import os
import enum
import platform
import sys
import shutil


PROJECT_BUILD_TARGET_MAP = {
    "": "",
    "rafter": "rafter",
}


PROJECT_RUN_TARGET_MAP = {
    "": "raftel",
    "rafter": "rafter"
}


PROJECT_TEST_TARGET_MAP = {
    "": "",
    "raftel": ""
}


class BuildType(enum.Enum):
    Debug = 0,
    Optimized = 1,
    Release = 2


class BuildSystem(enum.Enum):
    Make = 0,
    Ninja = 1,
    Xcode = 3,
    MSBuild = 4


# build configuration defaults
PROJECT = ""
RUN_PROJECT = False
BUILD_TESTS = False
RUN_TESTS = False

if platform.system() == "Windows":
    COMPILER = "Clang"
    CPP_COMPILER = "Clang"
else:
    COMPILER = "clang"
    CPP_COMPILER = "clang++"
BUILD_SYSTEM = BuildSystem.Ninja
BUILD_TYPE = BuildType.Debug
TARGET_OS = platform.system()
TARGET_ARCH = platform.machine()


def set_cwd_to_script_dir():
    script_path = os.path.abspath(__file__)
    script_dir = os.path.dirname(script_path)
    os.chdir(script_dir)


def parse_input_arguments(argc, argv):
    global PROJECT
    global RUN_PROJECT
    global BUILD_TESTS
    global RUN_TESTS
    global COMPILER
    global CPP_COMPILER
    global BUILD_TYPE
    global TARGET_OS
    global TARGET_ARCH
    global BUILD_SYSTEM

    index = 1
    while index < argc:
        argument = argv[index]

        if argument == "--project":
            index = index + 1
            PROJECT = argv[index]
        elif argument == "--run":
            RUN_PROJECT = True
        elif argument == "--build-tests":
            BUILD_TESTS = True
        elif argument == "--test":
            BUILD_TESTS = True
            RUN_TESTS = True
        elif argument == "--clang":
            if platform.system() == "Windows":
                COMPILER = "Clang"
                CPP_COMPILER = "Clang"
            else:
                COMPILER = "clang"
                CPP_COMPILER = "clang++"
        elif argument == "--gcc":
            COMPILER = "gcc"
            CPP_COMPILER = "g++"
        elif argument == "--msvc":
            COMPILER = "MSVC"
        elif argument == "--ninja":
            BUILD_SYSTEM = BuildSystem.Ninja
        elif argument == "--xcode":
            BUILD_SYSTEM = BuildSystem.Xcode
        elif argument == "--msbuild":
            BUILD_SYSTEM = BuildSystem.MSBuild
        elif argument == "--makefile":
            BUILD_SYSTEM = BuildSystem.Make
        elif argument == "--debug":
            BUILD_TYPE = BuildType.Debug
        elif argument == "--optimized":
            BUILD_TYPE = BuildType.Optimized
        elif argument == "--release":
            BUILD_TYPE = BuildType.Release
        else:
            print("unrecognized argument {}".format(argument))
            exit(1)

        index = index + 1


def validate_build_configuration():
    if platform.system() != "Windows" and COMPILER == "MSVC":
        print("MSVC Is Only Supported On Windows")
        exit(1)
    if PROJECT != "" and Path("./"+PROJECT+"/").exists() is False:
        print("Unrecognized/Missing Project {}".format(PROJECT))
        exit(1)


def print_build_details():
    print("Project to build: {}".format(PROJECT))
    print("Project should be ran: {}".format(RUN_PROJECT))
    print("Project tests should be built: {}".format(BUILD_TESTS))
    print("Project tests should be ran: {}".format(RUN_TESTS))
    print("Compiler: {}".format(COMPILER))
    print("Build Type: {}".format(BUILD_TYPE.name))
    print("Build Target OS: {}".format(TARGET_OS))
    print("Build Target Architecture: {}".format(TARGET_ARCH))


def create_build_dir():
    build_dir = "./build/"+TARGET_OS+"-"+TARGET_ARCH+"-" + \
        BUILD_SYSTEM.name+"-"+COMPILER+"-"+BUILD_TYPE.name
    if Path("build").exists() is False:
        os.mkdir("build")
    if Path(build_dir).exists() is False:
        os.mkdir(build_dir)
    os.chdir(build_dir)


def cmake_generate(generator, c_compiler, cpp_compiler, build_type):
    cmake_generate_command = "cmake"

    print(generator)
    if generator is not BuildSystem.MSBuild:
        cmake_generate_command = cmake_generate_command + " -G " + generator.name

    cmake_generate_command = cmake_generate_command +\
        " -D CMAKE_C_COMPILER=\"" + c_compiler +\
        "\" -D CMAKE_CXX_COMPILER=\"" + cpp_compiler + "\""

    if build_type is BuildType.Release:
        cmake_generate_command = cmake_generate_command + " -DCMAKE_BUILD_TYPE=Release"
    elif build_type is BuildType.Debug:
        cmake_generate_command = cmake_generate_command + " -DCMAKE_BUILD_TYPE=Debug"
    elif build_type is BuildType.Optimized:
        cmake_generate_command = cmake_generate_command + \
            " -DCMAKE_BUILD_TYPE=RelWithDebInfo"

    cmake_generate_command = cmake_generate_command + " ../.."

    print(cmake_generate_command)

    os.system(cmake_generate_command)


def cmake_build_target(target):
    cmake_build_command = "cmake --build ."
    if target != "":
        cmake_build_command = cmake_build_command + " -t " + target
    cmake_build_command = cmake_build_command + " -j"
    retvalue = os.system(cmake_build_command)
    return retvalue


def run_target(target):
    target_run_command = ""
    if platform.system() == "Windows":
        target_run_command = ".\\"+target
    else:
        target_run_command = "./"+target
    if TARGET_OS == "Windows":
        target_run_command = target_run_command + ".exe"
    print(target_run_command)
    os.system(target_run_command)


def run_project_build():
    cmake_generate(BUILD_SYSTEM, COMPILER, CPP_COMPILER, BUILD_TYPE)
    retvalue = cmake_build_target(PROJECT_BUILD_TARGET_MAP[PROJECT])

    if RUN_PROJECT is True:
        run_target(PROJECT_RUN_TARGET_MAP[PROJECT])

    if BUILD_TESTS is True:
        if PROJECT_TEST_TARGET_MAP[PROJECT] == "":
            print("Test target is not present for specified project, aborting")
            exit(1)

        cmake_build_target(PROJECT_TEST_TARGET_MAP[PROJECT])

        if RUN_TESTS is True:
            run_target(PROJECT_TEST_TARGET_MAP[PROJECT])

    return retvalue


def update_compile_commands():
    shutil.copyfile("compile_commands.json", "../../compile_commands.json")


if __name__ == "__main__":
    set_cwd_to_script_dir()
    parse_input_arguments(len(sys.argv), sys.argv)
    validate_build_configuration()
    print_build_details()
    create_build_dir()
    result = run_project_build()
    update_compile_commands()
    if result != 0:
        sys.exit(-1)
